use std::time::Duration;

pub trait SerialTransport {
    fn write_all(&mut self, data: &[u8]) -> Result<(), String>;
    fn read_some(&mut self, buf: &mut [u8], timeout: Duration) -> Result<usize, String>;
    fn clear_rx(&mut self);
}

#[cfg(target_os = "espidf")]
pub mod esp_io {
    use super::*;
    use core::ffi::c_void;
    use std::collections::VecDeque;
    use std::ptr;
    use std::sync::atomic::{AtomicBool, Ordering};
    use std::sync::{Arc, Condvar, Mutex};
    use std::thread::{self, JoinHandle};
    use std::time::Instant;

    use esp_idf_sys as sys;

    const FTDI_VID: u16 = 0x0403;
    const FTDI_PIDS: &[u16] = &[
        0x6001, // FT232R / IF17
        0x6014, // FT232H
        0x6015, // FT231X / FT230X
        0x6010, // FT2232
    ];

    const USB_DT_INTERFACE: u8 = 0x04;
    const USB_DT_ENDPOINT: u8 = 0x05;

    const FTDI_REQ_RESET: u8 = 0x00;
    const FTDI_REQ_SET_FLOW_CTRL: u8 = 0x02;
    const FTDI_REQ_SET_BAUD: u8 = 0x03;
    const FTDI_REQ_SET_LINE: u8 = 0x04;
    const FTDI_REQ_SET_LATENCY_TIMER: u8 = 0x09;

    // 3,000,000 / 9600 = 312.5 -> divisor 312 (0x0138) + fracción 0.5 (bit 14 = 0x4000) = 0x4138 exacto
    const FTDI_BAUD_9600_DIV: u16 = 0x4138;
    // 8 bits de datos (0x08), sin paridad (0x00), 2 bits de parada (0x1000)
    const FTDI_LINE_8N2: u16 = 0x1008;
    // Reduce la latencia por defecto de 16 ms a 2 ms en paquetes cortos eSSP
    const FTDI_LATENCY_MS: u16 = 2;

    const MAX_RX_BUF: usize = 4096;
    const CTRL_TX_TIMEOUT: Duration = Duration::from_millis(500);
    const BULK_OUT_TIMEOUT: Duration = Duration::from_millis(500);
    const CANCEL_WAIT_TIMEOUT: Duration = Duration::from_millis(500);

    #[inline]
    fn ms_to_ticks(ms: u32) -> sys::TickType_t {
        let hz = sys::configTICK_RATE_HZ as u64;
        (((ms as u64) * hz) / 1000).max(1) as sys::TickType_t
    }

    /// Extrae la dirección USB (1..=127) de `port_name` si viene especificada.
    /// Formatos aceptados: `"usb:2"`, `"ftdi:2"`, `"addr=2"`, `"/dev/usb2"`, `"2"`.
    /// Si no contiene número válido, devuelve `None` (abre el primer FTDI disponible).
    fn parse_target_usb_addr(port_name: &str) -> Option<u8> {
        let trimmed = port_name.trim();
        if trimmed.is_empty() {
            return None;
        }
        if let Ok(addr) = trimmed.parse::<u8>() {
            return (1..=127).contains(&addr).then_some(addr);
        }
        for sep in ["addr=", "usb:", "ftdi:", "/usb", "ttyUSB"] {
            if let Some(pos) = trimmed.rfind(sep) {
                let tail = &trimmed[pos + sep.len()..];
                let digits: String = tail.chars().take_while(|c| c.is_ascii_digit()).collect();
                if let Ok(addr) = digits.parse::<u8>() {
                    if (1..=127).contains(&addr) {
                        return Some(addr);
                    }
                }
            }
        }
        None
    }

    struct ClientHandle(sys::usb_host_client_handle_t);
    unsafe impl Send for ClientHandle {}

    struct RxState {
        buf: VecDeque<u8>,
        disconnected: bool,
        closing: bool,
        in_active: bool,
    }

    impl Default for RxState {
        fn default() -> Self {
            Self {
                buf: VecDeque::with_capacity(256),
                disconnected: false,
                closing: false,
                in_active: false,
            }
        }
    }

    struct RxSync {
        state: Mutex<RxState>,
        cvar: Condvar,
    }

    struct BulkInCtx {
        sync: Arc<RxSync>,
        mps_in: u16,
    }

    struct TxState {
        done: bool,
        status: sys::usb_transfer_status_t,
    }

    struct TxSync {
        state: Mutex<TxState>,
        cvar: Condvar,
    }

    struct TxCtx {
        sync: Arc<TxSync>,
    }

    extern "C" fn bulk_in_callback(transfer: *mut sys::usb_transfer_t) {
        if transfer.is_null() {
            return;
        }
        unsafe {
            let t = &mut *transfer;
            if t.context.is_null() {
                return;
            }
            let ctx = &*(t.context as *const BulkInCtx);

            let should_resubmit = if let Ok(mut guard) = ctx.sync.state.lock() {
                match t.status {
                    sys::usb_transfer_status_t_USB_TRANSFER_STATUS_COMPLETED => {
                        let actual = t.actual_num_bytes as usize;
                        // FTDI antepone 2 bytes de estado del módem en cada paquete Bulk IN
                        if actual > 2 && !t.data_buffer.is_null() {
                            let payload =
                                std::slice::from_raw_parts(t.data_buffer.add(2), actual - 2);
                            let overflow = (guard.buf.len() + payload.len()).saturating_sub(MAX_RX_BUF);
                            if overflow > 0 {
                                let drop_n = overflow.min(guard.buf.len());
                                guard.buf.drain(..drop_n);
                            }
                            let take = payload.len().min(MAX_RX_BUF);
                            guard.buf.extend(&payload[payload.len() - take..]);
                        }
                        let resubmit = !guard.closing;
                        if !resubmit {
                            guard.in_active = false;
                        }
                        ctx.sync.cvar.notify_all();
                        resubmit
                    }
                    sys::usb_transfer_status_t_USB_TRANSFER_STATUS_TIMED_OUT => {
                        let resubmit = !guard.closing;
                        if !resubmit {
                            guard.in_active = false;
                            ctx.sync.cvar.notify_all();
                        }
                        resubmit
                    }
                    sys::usb_transfer_status_t_USB_TRANSFER_STATUS_CANCELED => {
                        guard.in_active = false;
                        ctx.sync.cvar.notify_all();
                        false
                    }
                    // NO_DEVICE, STALL, ERROR, OVERFLOW, etc.: evita bucle infinito de reenvío
                    _ => {
                        guard.disconnected = true;
                        guard.in_active = false;
                        ctx.sync.cvar.notify_all();
                        false
                    }
                }
            } else {
                false
            };

            if should_resubmit {
                t.num_bytes = ctx.mps_in as i32;
                if sys::usb_host_transfer_submit(transfer) != sys::ESP_OK {
                    if let Ok(mut guard) = ctx.sync.state.lock() {
                        guard.disconnected = true;
                        guard.in_active = false;
                        ctx.sync.cvar.notify_all();
                    }
                }
            }
        }
    }

    extern "C" fn tx_complete_callback(transfer: *mut sys::usb_transfer_t) {
        if transfer.is_null() {
            return;
        }
        unsafe {
            let t = &*transfer;
            if t.context.is_null() {
                return;
            }
            let ctx = &*(t.context as *const TxCtx);
            if let Ok(mut guard) = ctx.sync.state.lock() {
                guard.status = t.status;
                guard.done = true;
                ctx.sync.cvar.notify_all();
            }
        }
    }

    pub struct EspFtdiPort {
        client: sys::usb_host_client_handle_t,
        dev: sys::usb_device_handle_t,
        if_num: u8,
        ep_in: u8,
        ep_out: u8,
        mps_in: u16,
        mps_out: u16,
        rx_sync: Arc<RxSync>,
        tx_sync: Arc<TxSync>,
        // Deben permanecer vivos hasta después de que termine `event_thread` en `Drop`
        _rx_ctx: Box<BulkInCtx>,
        _tx_ctx: Box<TxCtx>,
        in_transfer: *mut sys::usb_transfer_t,
        out_transfer: *mut sys::usb_transfer_t,
        stop_events: Arc<AtomicBool>,
        event_thread: Option<JoinHandle<()>>,
    }

    unsafe impl Send for EspFtdiPort {}

    impl EspFtdiPort {
        pub fn open(port_name: &str) -> Result<Self, String> {
            let target_addr = parse_target_usb_addr(port_name);

            unsafe {
                let mut client: sys::usb_host_client_handle_t = ptr::null_mut();
                let mut client_cfg = sys::usb_host_client_config_t {
                    is_synchronous: false,
                    client_context: ptr::null_mut(),
                    max_num_event_msg: 8,
                    async_: sys::usb_host_client_config_t__bindgen_ty_1 {
                        client_event_callback: None,
                        callback_arg: ptr::null_mut(),
                    },
                };

                let err = sys::usb_host_client_register(&mut client_cfg, &mut client);
                if err != sys::ESP_OK || client.is_null() {
                    return Err(format!("usb_host_client_register failed: {}", err));
                }

                let mut dev_addr_list = [0u8; 16];
                let mut num_devs: i32 = 0;
                let err = sys::usb_host_device_addr_list_fill(
                    dev_addr_list.len() as i32,
                    dev_addr_list.as_mut_ptr(),
                    &mut num_devs,
                );
                if err != sys::ESP_OK || num_devs <= 0 {
                    let _ = sys::usb_host_client_deregister(client);
                    return Err("No se encontraron dispositivos USB Host conectados".into());
                }

                let mut opened_dev: sys::usb_device_handle_t = ptr::null_mut();
                let mut found_if: Option<(u8, u8, u16, u8, u16)> = None;

                for &addr in &dev_addr_list[..(num_devs as usize)] {
                    if let Some(wanted) = target_addr {
                        if addr != wanted {
                            continue;
                        }
                    }

                    let mut dev: sys::usb_device_handle_t = ptr::null_mut();
                    if sys::usb_host_device_open(client, addr, &mut dev) != sys::ESP_OK
                        || dev.is_null()
                    {
                        continue;
                    }

                    let mut desc_ptr: *const sys::usb_device_desc_t = ptr::null();
                    if sys::usb_get_device_descriptor(dev, &mut desc_ptr) == sys::ESP_OK
                        && !desc_ptr.is_null()
                    {
                        let desc = &*desc_ptr;
                        if desc.idVendor == FTDI_VID && FTDI_PIDS.contains(&desc.idProduct) {
                            if let Some(ep_info) = Self::find_ftdi_bulk_endpoints(dev) {
                                opened_dev = dev;
                                found_if = Some(ep_info);
                                break;
                            }
                        }
                    }
                    let _ = sys::usb_host_device_close(client, dev);
                }

                let (if_num, ep_in, mps_in, ep_out, mps_out) = match found_if {
                    Some(info) if !opened_dev.is_null() => info,
                    _ => {
                        let _ = sys::usb_host_client_deregister(client);
                        return Err(format!(
                            "Dispositivo FTDI no encontrado para puerto '{}'",
                            port_name
                        ));
                    }
                };

                let err = sys::usb_host_interface_claim(client, opened_dev, if_num, 0);
                if err != sys::ESP_OK {
                    let _ = sys::usb_host_device_close(client, opened_dev);
                    let _ = sys::usb_host_client_deregister(client);
                    return Err(format!("usb_host_interface_claim failed: {}", err));
                }

                // Firma correcta ESP-IDF: usb_host_transfer_allocate(data_buffer_size, num_isoc_packets, &mut ptr)
                let mut out_transfer: *mut sys::usb_transfer_t = ptr::null_mut();
                let out_alloc_size = (mps_out as usize).max(64);
                let err = sys::usb_host_transfer_allocate(out_alloc_size, 0, &mut out_transfer);
                if err != sys::ESP_OK || out_transfer.is_null() {
                    let _ = sys::usb_host_interface_release(client, opened_dev, if_num);
                    let _ = sys::usb_host_device_close(client, opened_dev);
                    let _ = sys::usb_host_client_deregister(client);
                    return Err(format!("usb_host_transfer_allocate(OUT) failed: {}", err));
                }

                let mut in_transfer: *mut sys::usb_transfer_t = ptr::null_mut();
                let in_alloc_size = (mps_in as usize).max(64);
                let err = sys::usb_host_transfer_allocate(in_alloc_size, 0, &mut in_transfer);
                if err != sys::ESP_OK || in_transfer.is_null() {
                    let _ = sys::usb_host_transfer_free(out_transfer);
                    let _ = sys::usb_host_interface_release(client, opened_dev, if_num);
                    let _ = sys::usb_host_device_close(client, opened_dev);
                    let _ = sys::usb_host_client_deregister(client);
                    return Err(format!("usb_host_transfer_allocate(IN) failed: {}", err));
                }

                let rx_sync = Arc::new(RxSync {
                    state: Mutex::new(RxState::default()),
                    cvar: Condvar::new(),
                });
                let tx_sync = Arc::new(TxSync {
                    state: Mutex::new(TxState {
                        done: false,
                        status: sys::usb_transfer_status_t_USB_TRANSFER_STATUS_ERROR,
                    }),
                    cvar: Condvar::new(),
                });

                let rx_ctx = Box::new(BulkInCtx {
                    sync: Arc::clone(&rx_sync),
                    mps_in,
                });
                let tx_ctx = Box::new(TxCtx {
                    sync: Arc::clone(&tx_sync),
                });

                // Hilo dedicado a bombear `usb_host_client_handle_events` para que los callbacks se ejecuten
                let stop_events = Arc::new(AtomicBool::new(false));
                let stop_clone = Arc::clone(&stop_events);
                let client_wrap = ClientHandle(client);
                let event_thread = thread::Builder::new()
                    .name(format!("ftdi_usb_{}", if_num))
                    .stack_size(4096)
                    .spawn(move || {
                        let handle = client_wrap;
                        let poll_ticks = ms_to_ticks(50);
                        while !stop_clone.load(Ordering::Acquire) {
                            let _ = sys::usb_host_client_handle_events(handle.0, poll_ticks);
                        }
                    })
                    .map_err(|e| {
                        let _ = sys::usb_host_transfer_free(in_transfer);
                        let _ = sys::usb_host_transfer_free(out_transfer);
                        let _ = sys::usb_host_interface_release(client, opened_dev, if_num);
                        let _ = sys::usb_host_device_close(client, opened_dev);
                        let _ = sys::usb_host_client_deregister(client);
                        format!("No se pudo crear hilo USB Host events: {}", e)
                    })?;

                // A partir de aquí `Drop` garantiza limpieza ordenada si falla la configuración FTDI
                let mut port = Self {
                    client,
                    dev: opened_dev,
                    if_num,
                    ep_in,
                    ep_out,
                    mps_in,
                    mps_out,
                    rx_sync,
                    tx_sync,
                    _rx_ctx: rx_ctx,
                    _tx_ctx: tx_ctx,
                    in_transfer,
                    out_transfer,
                    stop_events,
                    event_thread: Some(event_thread),
                };

                port.configure_ftdi_9600_8n2()?;
                port.start_bulk_in()?;
                Ok(port)
            }
        }

        /// Recorre los descriptores en orden para emparejar cada interfaz con sus endpoints Bulk IN/OUT.
        unsafe fn find_ftdi_bulk_endpoints(
            dev: sys::usb_device_handle_t,
        ) -> Option<(u8, u8, u16, u8, u16)> {
            let mut cfg_ptr: *const sys::usb_config_desc_t = ptr::null();
            if sys::usb_get_active_config_descriptor(dev, &mut cfg_ptr) != sys::ESP_OK
                || cfg_ptr.is_null()
            {
                return None;
            }

            let cfg = &*cfg_ptr;
            let total_len = cfg.wTotalLength as i32;
            let mut offset: i32 = 0;
            let mut current_if: u8 = 0;
            let mut ep_in: Option<(u8, u16)> = None;
            let mut ep_out: Option<(u8, u16)> = None;

            let mut cur = cfg_ptr as *const sys::usb_standard_desc_t;
            while let Some(next_ptr) = {
                let p = sys::usb_parse_next_descriptor(cur, total_len, &mut offset);
                (!p.is_null()).then_some(p)
            } {
                cur = next_ptr;
                let desc = &*cur;

                if desc.bDescriptorType == USB_DT_INTERFACE {
                    let intf = &*(cur as *const sys::usb_intf_desc_t);
                    current_if = intf.bInterfaceNumber;
                    ep_in = None;
                    ep_out = None;
                } else if desc.bDescriptorType == USB_DT_ENDPOINT {
                    let ep = &*(cur as *const sys::usb_ep_desc_t);
                    // Transferencia Bulk (bmAttributes & 0x03 == 0x02)
                    if (ep.bmAttributes & 0x03) == 0x02 {
                        if (ep.bEndpointAddress & 0x80) != 0 && ep_in.is_none() {
                            ep_in = Some((ep.bEndpointAddress, ep.wMaxPacketSize));
                        } else if (ep.bEndpointAddress & 0x80) == 0 && ep_out.is_none() {
                            ep_out = Some((ep.bEndpointAddress, ep.wMaxPacketSize));
                        }

                        if let (Some((in_addr, in_mps)), Some((out_addr, out_mps))) =
                            (ep_in, ep_out)
                        {
                            return Some((current_if, in_addr, in_mps, out_addr, out_mps));
                        }
                    }
                }
            }

            None
        }

        fn send_control(&mut self, req: u8, value: u16, index: u16) -> Result<(), String> {
            unsafe {
                let mut ctrl: *mut sys::usb_transfer_t = ptr::null_mut();
                let ctrl_len = std::mem::size_of::<sys::usb_setup_packet_t>() + 8;
                let err = sys::usb_host_transfer_allocate(ctrl_len, 0, &mut ctrl);
                if err != sys::ESP_OK || ctrl.is_null() {
                    return Err(format!("usb_host_transfer_allocate(ctrl) failed: {}", err));
                }

                {
                    let mut guard = self
                        .tx_sync
                        .state
                        .lock()
                        .map_err(|_| "Mutex TX envenenado".to_string())?;
                    guard.done = false;
                    guard.status = sys::usb_transfer_status_t_USB_TRANSFER_STATUS_ERROR;
                }

                let t = &mut *ctrl;
                let setup = &mut *(t.data_buffer as *mut sys::usb_setup_packet_t);
                setup.bmRequestType = 0x40; // Vendor, host-to-device, device
                setup.bRequest = req;
                setup.wValue = value;
                setup.wIndex = index;
                setup.wLength = 0;

                t.num_bytes = std::mem::size_of::<sys::usb_setup_packet_t>() as i32;
                t.device_handle = self.dev;
                t.bEndpointAddress = 0;
                t.callback = Some(tx_complete_callback);
                t.context = (&*self._tx_ctx as *const TxCtx) as *mut c_void;

                let submit_err = sys::usb_host_transfer_submit_control(self.client, ctrl);
                if submit_err != sys::ESP_OK {
                    let _ = sys::usb_host_transfer_free(ctrl);
                    return Err(format!(
                        "FTDI control req {:#x} submit failed: {}",
                        req, submit_err
                    ));
                }

                let guard = self
                    .tx_sync
                    .state
                    .lock()
                    .map_err(|_| "Mutex TX envenenado".to_string())?;
                let (guard, _) = self
                    .tx_sync
                    .cvar
                    .wait_timeout_while(guard, CTRL_TX_TIMEOUT, |st| !st.done)
                    .map_err(|_| "Error esperando control transfer".to_string())?;

                if !guard.done {
                    // No liberar `ctrl` mientras siga en vuelo en el HCD para evitar Use-After-Free
                    return Err(format!("FTDI control req {:#x} timeout", req));
                }

                let status = guard.status;
                drop(guard);
                let _ = sys::usb_host_transfer_free(ctrl);

                if status != sys::usb_transfer_status_t_USB_TRANSFER_STATUS_COMPLETED {
                    return Err(format!(
                        "FTDI control req {:#x} falló con status {}",
                        req, status
                    ));
                }
                Ok(())
            }
        }

        fn configure_ftdi_9600_8n2(&mut self) -> Result<(), String> {
            // En protocolo FTDI el índice de canal es 1-based (1 = canal A / FT232R)
            let channel = (self.if_num as u16) + 1;
            self.send_control(FTDI_REQ_RESET, 0, channel)?;
            self.send_control(FTDI_REQ_SET_FLOW_CTRL, 0, channel)?;
            self.send_control(FTDI_REQ_SET_BAUD, FTDI_BAUD_9600_DIV, channel)?;
            self.send_control(FTDI_REQ_SET_LINE, FTDI_LINE_8N2, channel)?;
            self.send_control(FTDI_REQ_SET_LATENCY_TIMER, FTDI_LATENCY_MS, channel)?;
            Ok(())
        }

        fn start_bulk_in(&mut self) -> Result<(), String> {
            unsafe {
                if let Ok(mut guard) = self.rx_sync.state.lock() {
                    guard.closing = false;
                    guard.disconnected = false;
                    guard.in_active = true;
                }

                let t = &mut *self.in_transfer;
                t.device_handle = self.dev;
                t.bEndpointAddress = self.ep_in;
                t.num_bytes = self.mps_in as i32;
                t.callback = Some(bulk_in_callback);
                t.context = (&*self._rx_ctx as *const BulkInCtx) as *mut c_void;

                let err = sys::usb_host_transfer_submit(self.in_transfer);
                if err != sys::ESP_OK {
                    if let Ok(mut guard) = self.rx_sync.state.lock() {
                        guard.in_active = false;
                    }
                    return Err(format!("usb_host_transfer_submit(IN) failed: {}", err));
                }
                Ok(())
            }
        }
    }

    impl Drop for EspFtdiPort {
        fn drop(&mut self) {
            unsafe {
                // 1. Cancelar transfer Bulk IN de forma segura mientras el hilo de eventos sigue vivo
                let mut in_safe_to_free = true;
                if let Ok(mut guard) = self.rx_sync.state.lock() {
                    guard.closing = true;
                    if guard.in_active && !self.dev.is_null() {
                        drop(guard);
                        let _ = sys::usb_host_endpoint_halt(self.dev, self.ep_in);
                        let _ = sys::usb_host_endpoint_flush(self.dev, self.ep_in);

                        if let Ok(wait_guard) = self.rx_sync.state.lock() {
                            let (after, _) = self
                                .rx_sync
                                .cvar
                                .wait_timeout_while(wait_guard, CANCEL_WAIT_TIMEOUT, |st| {
                                    st.in_active
                                })
                                .unwrap_or_else(|e| e.into_inner());
                            in_safe_to_free = !after.in_active;
                        }
                        let _ = sys::usb_host_endpoint_clear(self.dev, self.ep_in);
                    }
                }

                // 2. Detener y unir el hilo de eventos USB Host antes de liberar recursos
                self.stop_events.store(true, Ordering::Release);
                if !self.client.is_null() {
                    let _ = sys::usb_host_client_unblock(self.client);
                }
                if let Some(handle) = self.event_thread.take() {
                    let _ = handle.join();
                }

                // 3. Liberar transfers, interfaz, dispositivo y cliente sin riesgo de Use-After-Free
                if in_safe_to_free && !self.in_transfer.is_null() {
                    let _ = sys::usb_host_transfer_free(self.in_transfer);
                }
                if !self.out_transfer.is_null() {
                    let _ = sys::usb_host_transfer_free(self.out_transfer);
                }
                if !self.dev.is_null() {
                    let _ = sys::usb_host_interface_release(self.client, self.dev, self.if_num);
                    let _ = sys::usb_host_device_close(self.client, self.dev);
                }
                if !self.client.is_null() {
                    let _ = sys::usb_host_client_deregister(self.client);
                }
            }
        }
    }

    impl SerialTransport for EspFtdiPort {
        fn write_all(&mut self, data: &[u8]) -> Result<(), String> {
            if data.is_empty() {
                return Ok(());
            }

            let chunk_size = (self.mps_out as usize).clamp(1, 64);
            for chunk in data.chunks(chunk_size) {
                unsafe {
                    {
                        let mut guard = self
                            .tx_sync
                            .state
                            .lock()
                            .map_err(|_| "Mutex TX envenenado".to_string())?;
                        guard.done = false;
                        guard.status = sys::usb_transfer_status_t_USB_TRANSFER_STATUS_ERROR;
                    }

                    let t = &mut *self.out_transfer;
                    t.device_handle = self.dev;
                    t.bEndpointAddress = self.ep_out;
                    t.num_bytes = chunk.len() as i32;
                    t.callback = Some(tx_complete_callback);
                    t.context = (&*self._tx_ctx as *const TxCtx) as *mut c_void;
                    ptr::copy_nonoverlapping(chunk.as_ptr(), t.data_buffer, chunk.len());

                    let err = sys::usb_host_transfer_submit(self.out_transfer);
                    if err != sys::ESP_OK {
                        return Err(format!("usb_host_transfer_submit(OUT) failed: {}", err));
                    }

                    let guard = self
                        .tx_sync
                        .state
                        .lock()
                        .map_err(|_| "Mutex TX envenenado".to_string())?;
                    let (guard, _) = self
                        .tx_sync
                        .cvar
                        .wait_timeout_while(guard, BULK_OUT_TIMEOUT, |st| !st.done)
                        .map_err(|_| "Error esperando Bulk OUT".to_string())?;

                    if !guard.done {
                        drop(guard);
                        // Cancelar transfer atascado antes de devolver error para que `out_transfer`
                        // no siga en vuelo en la siguiente escritura
                        let _ = sys::usb_host_endpoint_halt(self.dev, self.ep_out);
                        let _ = sys::usb_host_endpoint_flush(self.dev, self.ep_out);
                        if let Ok(w_guard) = self.tx_sync.state.lock() {
                            let _ = self.tx_sync.cvar.wait_timeout_while(
                                w_guard,
                                Duration::from_millis(200),
                                |st| !st.done,
                            );
                        }
                        let _ = sys::usb_host_endpoint_clear(self.dev, self.ep_out);
                        return Err("FTDI Bulk OUT write timeout".into());
                    }

                    let status = guard.status;
                    if status != sys::usb_transfer_status_t_USB_TRANSFER_STATUS_COMPLETED {
                        return Err(format!("FTDI Bulk OUT transfer falló con status {}", status));
                    }
                }
            }
            Ok(())
        }

        fn read_some(&mut self, buf: &mut [u8], timeout: Duration) -> Result<usize, String> {
            if buf.is_empty() {
                return Ok(0);
            }

            let deadline = Instant::now() + timeout;
            let mut guard = self
                .rx_sync
                .state
                .lock()
                .map_err(|_| "Mutex RX envenenado".to_string())?;

            loop {
                if !guard.buf.is_empty() {
                    let n = guard.buf.len().min(buf.len());
                    for slot in &mut buf[..n] {
                        *slot = guard.buf.pop_front().unwrap();
                    }
                    return Ok(n);
                }

                if guard.disconnected {
                    return Err("Dispositivo USB FTDI desconectado".into());
                }

                let now = Instant::now();
                if now >= deadline {
                    return Ok(0);
                }

                let (new_guard, _) = self
                    .rx_sync
                    .cvar
                    .wait_timeout(guard, deadline.saturating_duration_since(now))
                    .map_err(|_| "Error esperando datos en RX Condvar".to_string())?;
                guard = new_guard;
            }
        }

        fn clear_rx(&mut self) {
            if let Ok(mut guard) = self.rx_sync.state.lock() {
                guard.buf.clear();
            }
        }
    }
}

#[cfg(not(target_os = "espidf"))]
pub mod pc_io {
    use super::*;
    use std::io::{ErrorKind, Read, Write};

    pub struct PcSerialPort {
        inner: Box<dyn serialport::SerialPort>,
        last_timeout: Duration,
    }

    impl PcSerialPort {
        pub fn open(port_name: &str) -> Result<Self, String> {
            let init_timeout = Duration::from_millis(100);
            let p = serialport::new(port_name, 9600)
                .data_bits(serialport::DataBits::Eight)
                .stop_bits(serialport::StopBits::Two)
                .parity(serialport::Parity::None)
                .flow_control(serialport::FlowControl::None)
                .timeout(init_timeout)
                .open()
                .map_err(|e| format!("Error abriendo {}: {}", port_name, e))?;

            let _ = p.clear(serialport::ClearBuffer::All);
            Ok(Self {
                inner: p,
                last_timeout: init_timeout,
            })
        }
    }

    impl SerialTransport for PcSerialPort {
        fn write_all(&mut self, data: &[u8]) -> Result<(), String> {
            self.inner.write_all(data).map_err(|e| e.to_string())
        }

        fn read_some(&mut self, buf: &mut [u8], timeout: Duration) -> Result<usize, String> {
            if buf.is_empty() {
                return Ok(0);
            }

            let effective = timeout.max(Duration::from_millis(1));
            if effective != self.last_timeout {
                let _ = self.inner.set_timeout(effective);
                self.last_timeout = effective;
            }

            match self.inner.read(buf) {
                Ok(n) => Ok(n),
                Err(ref e)
                    if e.kind() == ErrorKind::TimedOut || e.kind() == ErrorKind::Interrupted =>
                {
                    Ok(0)
                }
                Err(e) => Err(e.to_string()),
            }
        }

        fn clear_rx(&mut self) {
            let _ = self.inner.clear(serialport::ClearBuffer::Input);
        }
    }
}