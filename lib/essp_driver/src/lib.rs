#![allow(non_snake_case, non_camel_case_types, dead_code)]
use std::ffi::{c_char, c_int, c_uchar, c_ulong, c_ulonglong, CString};

// ==================== 1. FFI CON LA LIBRERÍA C (eSSP/inc/SSPComs.h) ====================
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct SSP_FULL_KEY {
    pub FixedKey: c_ulonglong,
    pub EncryptKey: c_ulonglong,
}

#[repr(C)]
pub struct SSP_COMMAND {
    pub Key: SSP_FULL_KEY,
    pub BaudRate: c_ulong,
    pub Timeout: c_ulong,
    pub PortNumber: c_uchar,
    pub SSPAddress: c_uchar,
    pub RetryLevel: c_uchar,
    pub EncryptionStatus: c_uchar,
    pub CommandDataLength: c_uchar,
    pub CommandData: [c_uchar; 255],
    pub ResponseStatus: c_uchar,
    pub ResponseDataLength: c_uchar,
    pub ResponseData: [c_uchar; 255],
    pub IgnoreError: c_uchar,
}

impl Default for SSP_COMMAND {
    fn default() -> Self {
        unsafe { std::mem::zeroed() }
    }
}

// Cuando compilas para producción, enlaza con la librería C real
//#[cfg(not(test))]
extern "C" {
    fn OpenSSPPort(port: *const c_char) -> c_int;
    fn CloseSSPPort(port: c_int);
    fn SSPSendCommand(port: c_int, cmd: *mut SSP_COMMAND) -> c_int;
    fn NegotiateSSPEncryption(port: c_int, ssp_address: c_char, key: *mut SSP_FULL_KEY) -> c_int;
}

// ==================== 2. PUENTE CXX HACIA C++ (ValidadorUnit / CashHub) ====================

#[cxx::bridge(namespace = "ffi")]
pub mod ffi {
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    pub enum DeviceType {
        Bill, // SSP 0
        Coin, // SSP 16
    }

    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    pub enum EventKind {
        Credit,
        Dispensing,
        Dispensed,
        Timeout,
        IncompletePayout,
        Jammed,
        FraudAttempt,
        CashboxRemoved,
        CashboxReplaced,
        Error,
    }

    #[derive(Debug, Clone)]
    pub struct DenomLevel {
        pub value_cents: i32,       // Ej. 2000 ($20.00 MXN)
        pub value_pesos: i32,       // Ej. 20
        pub country_code: String,   // "MXN"
        pub stored_in_payout: i32,  // Cantidad actual en reciclador
        pub stored_in_cashbox: i32, // Cantidad enviada al apilador/cashbox
        pub route_to_payout: bool,  // true = Reciclador, false = Cashbox
    }

    #[derive(Debug, Clone, Copy)]
    pub struct RouteRule {
        pub value_cents: i32,
        pub to_payout: bool,
    }

    #[derive(Debug, Clone, Copy)]
    pub struct PayoutItem {
        pub value_cents: i32,
        pub count: i32,
    }

    #[derive(Debug, Clone)]
    pub struct CashEvent {
        pub device_id: String,
        pub device_type: DeviceType,
        pub kind: EventKind,
        pub value_cents: i32,
        pub value_pesos: i32,
        pub message: String,
    }

    extern "Rust" {
        type RustEsspUnit;

        fn detectar_ssp_en_puerto(puerto: &str) -> i32;
        fn new_essp_unit(puerto: &str, ssp_address: u8) -> Box<RustEsspUnit>;

        fn sincronizar(self: &mut RustEsspUnit) -> bool;
        fn iniciar_y_conectar(
            self: &mut RustEsspUnit,
            habilitar_recolector: bool,
            rutas: &[RouteRule],
        ) -> Result<Vec<DenomLevel>>;
        fn detener_y_desconectar(self: &mut RustEsspUnit);
        fn obtener_niveles(self: &mut RustEsspUnit) -> Result<Vec<DenomLevel>>;
        fn cambiar_ruta(self: &mut RustEsspUnit, valor_centavos: i32, a_reciclador: bool) -> Result<()>;
        fn dispensar_piezas(self: &mut RustEsspUnit, piezas: &[PayoutItem]) -> Result<()>;
        fn dispensar_por_canales(self: &mut RustEsspUnit, cantidades: &[i32]) -> Result<()>;
        fn vaciar_reciclador(self: &mut RustEsspUnit, smart_empty: bool) -> Result<()>;
        fn detener_pago(self: &mut RustEsspUnit) -> Result<()>;
        fn aceptar_escrow(self: &mut RustEsspUnit) -> Result<()>;
        fn rechazar_escrow(self: &mut RustEsspUnit) -> Result<()>;
        fn limpiar_niveles_cashbox(self: &mut RustEsspUnit);
        fn hacer_poll(self: &mut RustEsspUnit) -> Result<Vec<CashEvent>>;
        fn get_serial_id(self: &RustEsspUnit) -> &str;
        fn get_device_type(self: &RustEsspUnit) -> DeviceType;
        fn reiniciar(self: &mut RustEsspUnit) -> Result<()>;
    }
}
// ==================== 3. IMPLEMENTACIÓN DE HARDWARE (RustEsspUnit) ====================
pub struct RustEsspUnit {
    puerto: String,
    port_handle: c_int,
    ssp_address: u8,
    unit_type: u8,
    serial_id: String,
    key: SSP_FULL_KEY,
    encrypted: bool,
    canales_centavos: Vec<i32>,
}

pub fn new_essp_unit(puerto: &str, ssp_address: u8) -> Box<RustEsspUnit> {
    Box::new(RustEsspUnit {
        puerto: puerto.to_string(),
        port_handle: -1,
        ssp_address,
        unit_type: if ssp_address == 16 { 0x03 } else { 0x06 },
        serial_id: String::new(),
        key: SSP_FULL_KEY {
            FixedKey: 0x0123456701234567,
            EncryptKey: 0,
        },
        encrypted: false,
        canales_centavos: Vec::new(),
    })
}

/// Prueba primero dirección 16 (Monedero) y luego 0 (Billetero). Devuelve -1 si no responde.
pub fn detectar_ssp_en_puerto(puerto: &str) -> i32 {
    for &ssp in &[16u8, 0u8] {
        let mut unit = new_essp_unit(puerto, ssp);
        if unit.sincronizar() {
            return ssp as i32;
        }
    }
    -1
}

impl RustEsspUnit {
    fn send_raw(&mut self, data: &[u8], usar_encriptacion: bool) -> Result<Vec<u8>, String> {
        let mut cmd = SSP_COMMAND::default();
        cmd.Key = self.key;
        cmd.BaudRate = 9600;
        cmd.Timeout = 1000;
        cmd.PortNumber = self.port_handle as u8;
        cmd.SSPAddress = self.ssp_address;
        cmd.RetryLevel = 3;
        cmd.EncryptionStatus = u8::from(usar_encriptacion && self.encrypted);
        cmd.CommandDataLength = data.len() as u8;
        cmd.CommandData[..data.len()].copy_from_slice(data);

        let ok = unsafe { SSPSendCommand(self.port_handle, &mut cmd) };
        if ok == 0 {
            return Err(format!(
                "Timeout/Error serial enviando 0x{:02X} a SSP {}",
                data[0], self.ssp_address
            ));
        }
        self.key = cmd.Key;

        let len = cmd.ResponseDataLength as usize;
        let resp = cmd.ResponseData[..len].to_vec();
        if resp.is_empty() {
            return Err("Respuesta vacía del dispositivo".into());
        }
        if resp[0] != 0xF0 {
            return Err(format!(
                "Respuesta de error SSP en cmd 0x{:02X}: {:02X?}",
                data[0], resp
            ));
        }
        Ok(resp)
    }

    pub fn reiniciar(&mut self) -> Result<(), String> {
    // 0x01 = SSP_CMD_RESET (reinicia el validador físicamente)
    self.send_raw(&[0x01], self.encrypted)?;
    unsafe { CloseSSPPort(self.port_handle) };
    self.port_handle = -1;
    self.encrypted = false;
    Ok(())
}

    pub fn sincronizar(&mut self) -> bool {
        let Ok(c_port) = CString::new(self.puerto.as_str()) else {
            return false;
        };
        self.port_handle = unsafe { OpenSSPPort(c_port.as_ptr()) };
        if self.port_handle < 0 {
            return false;
        }
        let res = self.send_raw(&[0x11], false).is_ok(); // 0x11 = SSP_CMD_SYNC
        unsafe { CloseSSPPort(self.port_handle) };
        self.port_handle = -1;
        res
    }

    pub fn iniciar_y_conectar(
        &mut self,
        habilitar_recolector: bool,
        rutas: &[ffi::RouteRule],
    ) -> Result<Vec<ffi::DenomLevel>, String> {
        let c_port = CString::new(self.puerto.as_str()).map_err(|e| e.to_string())?;
        self.port_handle = unsafe { OpenSSPPort(c_port.as_ptr()) };
        if self.port_handle < 0 {
            return Err(format!("No se pudo abrir {}", self.puerto));
        }

        // 1. SYNC (0x11)
        self.send_raw(&[0x11], false)?;

        // 2. Negociar Encriptación eSSP (Diffie-Hellman + AES-128)
        self.key.FixedKey = 0x0123456701234567;
        let enc_ok = unsafe {
            NegotiateSSPEncryption(self.port_handle, self.ssp_address as c_char, &mut self.key)
        };
        if enc_ok == 0 {
            self.detener_y_desconectar();
            return Err("Fallo al negociar llaves AES eSSP".into());
        }
        self.encrypted = true;

        // 3. HOST_PROTOCOL_VERSION (0x06 -> v6 para soporte multimoneda MXN)
        self.send_raw(&[0x06, 0x06], true)?;

        // 4. Obtener Número de Serie (0x0C)
        let resp_ser = self.send_raw(&[0x0C], true)?;
        if resp_ser.len() >= 5 {
            let serial = u32::from_be_bytes([resp_ser[1], resp_ser[2], resp_ser[3], resp_ser[4]]);
            self.serial_id = serial.to_string();
        }

        // 5. SETUP_REQUEST (0x05)
        let resp_setup = self.send_raw(&[0x05], true)?;
        let (unit_type, canales) = parse_setup_request(self.ssp_address, &resp_setup)?;
        self.unit_type = unit_type;
        self.canales_centavos = canales;

        // 6. Habilitar módulo reciclador si es billetero
        if self.ssp_address == 0 {
            if self.send_raw(&[0x5C, 0x00], true).is_err() {
                let _ = self.send_raw(&[0x5C], true); // ENABLE_PAYOUT_DEVICE
            }
        }

        // 7. Configurar rutas iniciales
        for regla in rutas {
            let _ = self.cambiar_ruta(regla.value_cents, regla.to_payout);
        }

        // 8. Leer niveles actuales
        let niveles = self.obtener_niveles()?;

        // 9. Configurar Inhibits (si habilitar_recolector es false, bloqueamos la entrada de efectivo)
        if habilitar_recolector {
            let _ = self.send_raw(&[0x02, 0xFF, 0xFF], true);
            if self.ssp_address == 16 {
                let _ = self.send_raw(&[0x49, 0x01], true);
                for &val in &self.canales_centavos.clone() {
                    let mut cmd_v6 = vec![0x40, 0x01];
                    cmd_v6.extend_from_slice(&(val as u16).to_le_bytes());
                    cmd_v6.extend_from_slice(b"MXN");
                    if self.send_raw(&cmd_v6, true).is_err() {
                        let mut cmd_short = vec![0x40, 0x01];
                        cmd_short.extend_from_slice(&(val as u16).to_le_bytes());
                        let _ = self.send_raw(&cmd_short, true);
                    }
                }
            }
        } else {
            // Modo solo dispensado: inhibimos admisión de billetes/monedas
            let _ = self.send_raw(&[0x02, 0x00, 0x00], true);
            if self.ssp_address == 16 {
                let _ = self.send_raw(&[0x49, 0x00], true);
            }
        }

        // 10. ENABLE (0x0A) SIEMPRE al final: indispensable para que el hopper/reciclador pueda dispensar
        self.send_raw(&[0x0A], true)?;

        Ok(niveles)
    }

    pub fn dispensar_piezas(&mut self, piezas: &[ffi::PayoutItem]) -> Result<(), String> {
        if piezas.is_empty() {
            return Ok(());
        }
        // Aseguramos que el dispositivo esté en ENABLE (0x0A) antes de enviar 0x46
        let _ = self.send_raw(&[0x0A], true);
        eprintln!(
            "   [DISPENSAR SSP {}]: {:?}",
            self.ssp_address, piezas
        );
        let cmd = build_payout_by_denom_cmd(piezas, "MXN");
        self.send_raw(&cmd, true)?;
        Ok(())
    }
    
    pub fn obtener_niveles(&mut self) -> Result<Vec<ffi::DenomLevel>, String> {
        let mut niveles = Vec::new();
        for &val_cents in &self.canales_centavos.clone() {
            if val_cents <= 0 {
                continue;
            }
            // 0x35 = GET_DENOMINATION_LEVEL
            let mut cmd_lvl = vec![0x35];
            cmd_lvl.extend_from_slice(&val_cents.to_le_bytes());
            cmd_lvl.extend_from_slice(b"MXN");

            let stored = match self.send_raw(&cmd_lvl, true) {
                Ok(r) if r.len() >= 3 => u16::from_le_bytes([r[1], r[2]]) as i32,
                _ => 0,
            };

            // 0x3C = GET_DENOMINATION_ROUTE
            let mut cmd_rt = vec![0x3C];
            cmd_rt.extend_from_slice(&val_cents.to_le_bytes());
            cmd_rt.extend_from_slice(b"MXN");

            let route_to_payout = match self.send_raw(&cmd_rt, true) {
                Ok(r) if r.len() >= 2 => r[1] == 0x00,
                _ => false,
            };

            niveles.push(ffi::DenomLevel {
                stored_in_cashbox: 0,
                value_cents: val_cents,
                value_pesos: val_cents / 100,
                country_code: "MXN".to_string(),
                stored_in_payout: stored,
                route_to_payout,
            });
        }
        Ok(niveles)
    }

    pub fn cambiar_ruta(&mut self, valor_centavos: i32, a_reciclador: bool) -> Result<(), String> {
        let cmd = build_set_route_cmd(valor_centavos, "MXN", a_reciclador);
        self.send_raw(&cmd, true)?;
        Ok(())
    }


    pub fn detener_pago(&mut self) -> Result<(), String> {
        // 0x38 = SSP_CMD_HALT_PAYOUT (detiene el motor del hopper inmediatamente)
        self.send_raw(&[0x38], true)?;
        Ok(())
    }

    pub fn vaciar_reciclador(&mut self, smart_empty: bool) -> Result<(), String> {
        // 0x52 = SMART_EMPTY (cuenta las piezas al vaciar), 0x3F = EMPTY_ALL
        let opcode = if smart_empty { 0x52 } else { 0x3F };
        self.send_raw(&[opcode], true)?;
        Ok(())
    }

    pub fn hacer_poll(&mut self) -> Result<Vec<ffi::CashEvent>, String> {
        let resp = self.send_raw(&[0x07], true)?; // 0x07 = SSP_CMD_POLL

        parse_poll_response(
            &resp,
            &self.serial_id,
            self.get_device_type(),
            &self.canales_centavos,
        )
    }

    pub fn detener_y_desconectar(&mut self) {
        if self.port_handle >= 0 {
            let _ = self.send_raw(&[0x09], self.encrypted); // 0x09 = DISABLE
            unsafe { CloseSSPPort(self.port_handle) };
            self.port_handle = -1;
            self.encrypted = false;
        }
    }

    pub fn get_serial_id(&self) -> &str {
        &self.serial_id
    }

    pub fn get_device_type(&self) -> ffi::DeviceType {
        if self.ssp_address == 16 {
            ffi::DeviceType::Coin
        } else {
            ffi::DeviceType::Bill
        }
    }
    pub fn dispensar_por_canales(&mut self, cantidades: &[i32]) -> Result<(), String> {
        let mut piezas = Vec::new();
        for (idx, &cant) in cantidades.iter().enumerate() {
            if cant > 0 {
                if let Some(&val_cents) = self.canales_centavos.get(idx) {
                    piezas.push(ffi::PayoutItem {
                        value_cents: val_cents,
                        count: cant,
                    });
                }
            }
        }
        self.dispensar_piezas(&piezas)
    }

    pub fn aceptar_escrow(&mut self) -> Result<(), String> {
        // En eSSP estándar el billete en lectura se acepta automáticamente al continuar el poll
        Ok(())
    }

    pub fn rechazar_escrow(&mut self) -> Result<(), String> {
        // 0x08 = SSP_CMD_REJECT_NOTE (Devuelve el billete desde Escrow)
        self.send_raw(&[0x08], true)?;
        Ok(())
    }

    pub fn limpiar_niveles_cashbox(&mut self) {
        // Reinicia contadores locales de cashbox si se llevan en memoria
    }
}

// ==================== 4. LÓGICA PURA DE PROTOCOLO eSSP (TESTEABLE SIN HARDWARE) ====================

pub fn parse_setup_request(ssp_address: u8, resp: &[u8]) -> Result<(u8, Vec<i32>), String> {
    if resp.len() < 12 || resp[0] != 0xF0 {
        return Err("Trama SETUP_REQUEST inválida o incompleta".into());
    }

    let unit_type = resp[1];
    let mut canales_centavos = Vec::new();

    if ssp_address == 16 || unit_type == 0x03 || unit_type == 0x09 {
        let num_canales = resp[10] as usize;
        let mut offset = 11;
        for _ in 0..num_canales {
            if offset + 2 > resp.len() {
                return Err("Trama de monedero truncada en canales".into());
            }
            let val = u16::from_le_bytes([resp[offset], resp[offset + 1]]) as i32;
            canales_centavos.push(val);
            offset += 2;
        }
    } else {
        if resp.len() < 17 {
            return Err("Trama de billetero demasiado corta".into());
        }
        let num_canales = resp[12] as usize;
        let header_end = 17 + (2 * num_canales);
        if resp.len() < header_end {
            return Err("Trama de billetero sin multiplicador/versión".into());
        }

        let real_mult = ((resp[13 + 2 * num_canales] as i32) << 16)
            | ((resp[14 + 2 * num_canales] as i32) << 8)
            | (resp[15 + 2 * num_canales] as i32);
        let proto_ver = resp[16 + 2 * num_canales];

        if proto_ver >= 6 {
            let mut offset = 17 + (5 * num_canales);
            for _ in 0..num_canales {
                if offset + 4 > resp.len() {
                    return Err("Trama de billetero truncada en ExpandedChannelValues".into());
                }
                let raw_val = u32::from_le_bytes([
                    resp[offset],
                    resp[offset + 1],
                    resp[offset + 2],
                    resp[offset + 3],
                ]) as i32;
                // Si el firmware ya trae RealValueMultiplier = 100, raw_val * real_mult ya son centavos
                let mult = if real_mult >= 100 { real_mult } else { 100 };
                canales_centavos.push(raw_val * mult);
                offset += 4;
            }
        }
    }

    Ok((unit_type, canales_centavos))
}

pub fn build_set_route_cmd(valor_centavos: i32, country: &str, to_payout: bool) -> Vec<u8> {
    let route_byte = if to_payout { 0x00 } else { 0x01 };
    let mut cmd = vec![0x3B, route_byte];
    cmd.extend_from_slice(&valor_centavos.to_le_bytes());
    cmd.extend_from_slice(country.as_bytes());
    cmd
}

pub fn build_payout_by_denom_cmd(piezas: &[ffi::PayoutItem], country: &str) -> Vec<u8> {
    let mut num_denoms: u8 = 0;
    let mut payload = Vec::new();

    for item in piezas {
        if item.count > 0 && item.value_cents > 0 {
            num_denoms += 1;
            payload.extend_from_slice(&(item.count as u16).to_le_bytes());
            payload.extend_from_slice(&item.value_cents.to_le_bytes());
            payload.extend_from_slice(country.as_bytes());
        }
    }

    if num_denoms == 0 {
        return Vec::new();
    }

    let mut cmd = vec![0x46, num_denoms];
    cmd.extend_from_slice(&payload);
    cmd.push(0x58);
    cmd
}

pub fn parse_poll_response(
    resp: &[u8],
    device_id: &str,
    device_type: ffi::DeviceType,
    canales_centavos: &[i32],
) -> Result<Vec<ffi::CashEvent>, String> {
    if resp.is_empty() || resp[0] != 0xF0 {
        return Err("Respuesta POLL inválida".into());
    }

    let mut eventos = Vec::new();
    let mut i = 1;

    while i < resp.len() {
        match resp[i] {
            // 0xEF: SSP_POLL_READ_NOTE (2 bytes: 0xEF + canal). Canal 0 = leyendo, >0 = en Escrow
            0xEF => {
                i += 2;
            }
            // 0xEE: SSP_POLL_CREDIT_NOTE (Billetero: 0xEE + 1 byte de canal base 1)
            0xEE => {
                if i + 1 < resp.len() {
                    let canal = resp[i + 1] as usize;
                    if canal > 0 && canal <= canales_centavos.len() {
                        let cents = canales_centavos[canal - 1];
                        eventos.push(ffi::CashEvent {
                            device_id: device_id.into(),
                            device_type,
                            kind: ffi::EventKind::Credit,
                            value_cents: cents,
                            value_pesos: cents / 100,
                            message: "Billete acreditado".into(),
                        });
                    }
                    i += 2;
                } else {
                    break;
                }
            }
            // 0xDF: SSP_POLL_COIN_CREDIT | 0xBF: SSP_POLL_VALUE_ADDED (Monedero / SMART Coin System)
            0xDF | 0xBF => {
                // Formato 9 bytes con num_divisas: [cmd, num_div, v0, v1, v2, v3, 'M', 'X', 'N']
                if i + 8 < resp.len() && &resp[i + 6..i + 9] == b"MXN" {
                    let num_divisas = (resp[i + 1] as usize).max(1);
                    let cents =
                        i32::from_le_bytes([resp[i + 2], resp[i + 3], resp[i + 4], resp[i + 5]]);
                    if cents > 0 {
                        eventos.push(ffi::CashEvent {
                            device_id: device_id.into(),
                            device_type,
                            kind: ffi::EventKind::Credit,
                            value_cents: cents,
                            value_pesos: cents / 100,
                            message: "Moneda acreditada".into(),
                        });
                    }
                    i += 2 + (7 * num_divisas);
                }
                // Formato 8 bytes estándar v6: [cmd, v0, v1, v2, v3, 'M', 'X', 'N']
                else if i + 7 < resp.len() {
                    let cents =
                        i32::from_le_bytes([resp[i + 1], resp[i + 2], resp[i + 3], resp[i + 4]]);
                    if cents > 0 {
                        eventos.push(ffi::CashEvent {
                            device_id: device_id.into(),
                            device_type,
                            kind: ffi::EventKind::Credit,
                            value_cents: cents,
                            value_pesos: cents / 100,
                            message: "Moneda acreditada".into(),
                        });
                    }
                    i += 8;
                } else {
                    break;
                }
            }
            // 0xBE: SSP_POLL_COIN_FEEDING | 0xD6: SSP_POLL_HALTED (9 bytes)
            0xBE | 0xD6 => {
                if i + 8 < resp.len() {
                    let num_divisas = (resp[i + 1] as usize).max(1);
                    i += 2 + (7 * num_divisas);
                } else {
                    i += 1;
                }
            }
            // 0xD2: Dispensed | 0xDA: Dispensing | 0xD9: Timeout
            0xD2 | 0xDA | 0xD9 => {
                let kind = match resp[i] {
                    0xD2 => ffi::EventKind::Dispensed,
                    0xD9 => ffi::EventKind::Timeout,
                    _ => ffi::EventKind::Dispensing,
                };
                if i + 8 < resp.len() {
                    let num_divisas = (resp[i + 1] as usize).max(1);
                    let cents =
                        i32::from_le_bytes([resp[i + 2], resp[i + 3], resp[i + 4], resp[i + 5]]);
                    let msg = if kind == ffi::EventKind::Timeout {
                        format!(
                            "Timeout del hardware dispensando (entregado parcial: ${} MXN)",
                            cents / 100
                        )
                    } else {
                        String::new()
                    };
                    eventos.push(ffi::CashEvent {
                        device_id: device_id.into(),
                        device_type,
                        kind,
                        value_cents: cents,
                        value_pesos: cents / 100,
                        message: msg,
                    });
                    i += 2 + (7 * num_divisas);
                } else {
                    i += 5;
                }
            }
            // 0xDC: SSP_POLL_INCOMPLETE_PAYOUT (13 bytes)
            0xDC => {
                if i + 12 < resp.len() {
                    let num_divisas = (resp[i + 1] as usize).max(1);
                    let pagado_cents =
                        i32::from_le_bytes([resp[i + 2], resp[i + 3], resp[i + 4], resp[i + 5]]);
                    let pedido_cents =
                        i32::from_le_bytes([resp[i + 6], resp[i + 7], resp[i + 8], resp[i + 9]]);
                    eventos.push(ffi::CashEvent {
                        device_id: device_id.into(),
                        device_type,
                        kind: ffi::EventKind::IncompletePayout,
                        value_cents: pagado_cents,
                        value_pesos: pagado_cents / 100,
                        message: format!(
                            "Pago incompleto: se entregaron ${} de ${} MXN solicitados",
                            pagado_cents / 100,
                            pedido_cents / 100
                        ),
                    });
                    i += 2 + (11 * num_divisas);
                } else {
                    i += 9;
                }
            }
            // 0xE6: Fraud Attempt
            0xE6 => {
                eventos.push(ffi::CashEvent {
                    device_id: device_id.into(),
                    device_type,
                    kind: ffi::EventKind::FraudAttempt,
                    value_cents: 0,
                    value_pesos: 0,
                    message: "Intento de fraude detectado".into(),
                });
                if i + 8 < resp.len() && &resp[i + 6..i + 9] == b"MXN" {
                    let num_divisas = (resp[i + 1] as usize).max(1);
                    i += 2 + (7 * num_divisas);
                } else {
                    i += 2;
                }
            }
            // 0xE9: Unsafe Jam | 0xEA: Safe Jam | 0xD5: Jammed
            0xE9 | 0xEA | 0xD5 => {
                eventos.push(ffi::CashEvent {
                    device_id: device_id.into(),
                    device_type,
                    kind: ffi::EventKind::Jammed,
                    value_cents: 0,
                    value_pesos: 0,
                    message: "Atasco físico detectado".into(),
                });
                i += 1;
            }
            // 0xE3: Cashbox Removed
            0xE3 => {
                eventos.push(ffi::CashEvent {
                    device_id: device_id.into(),
                    device_type,
                    kind: ffi::EventKind::CashboxRemoved,
                    value_cents: 0,
                    value_pesos: 0,
                    message: "CASHBOX_REMOVED".into(),
                });
                i += 1;
            }
            // 0xE4: Cashbox Replaced
            0xE4 => {
                eventos.push(ffi::CashEvent {
                    device_id: device_id.into(),
                    device_type,
                    kind: ffi::EventKind::CashboxReplaced,
                    value_cents: 0,
                    value_pesos: 0,
                    message: "CASHBOX_REPLACED".into(),
                });
                i += 1;
            }
            // 0xB5: Channel Disable (1 byte) | Otros eventos de 1 byte (0xCC Stacking, 0xEB Stacked, 0xDB Stored, 0xE8 Disabled)
            _ => {
                i += 1;
            }
        }
    }

    Ok(eventos)
}

pub fn calcular_cambio_nativo(
    cambio_restante: &mut u32,
    niveles: &[ffi::DenomLevel],
    is_cambio: bool,
) -> Vec<ffi::PayoutItem> {
    let monto_original = *cambio_restante;
    let mut ordenados = niveles.to_vec();
    ordenados.sort_by(|a, b| b.value_pesos.cmp(&a.value_pesos));

    let mut piezas = Vec::new();

    for denom in ordenados {
        if denom.value_pesos <= 0 || denom.stored_in_payout <= 0 {
            continue;
        }
        let valor = denom.value_pesos as u32;
        let mut disponibles = denom.stored_in_payout;
        let mut usar = 0;

        while *cambio_restante >= valor
            && (!is_cambio || valor < monto_original)
            && disponibles > 0
        {
            *cambio_restante -= valor;
            disponibles -= 1;
            usar += 1;
        }

        if usar > 0 {
            piezas.push(ffi::PayoutItem {
                value_cents: denom.value_cents,
                count: usar,
            });
        }
    }
    piezas
}

// ==================== 5. SUITE DE PRUEBAS UNITARIAS ====================
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_setup_billetero_mxn_incluyendo_500_y_1000() {
        let mut trama = vec![
            0xF0, 0x06, b'0', b'1', b'0', b'0', b'M', b'X', b'N', 0x00, 0x00, 0x01, 0x06, 20, 50,
            100, 200, 255, 255, 0, 0, 0, 0, 0, 0, 0x00, 0x00, 0x01, 0x06,
        ];
        for _ in 0..6 {
            trama.extend_from_slice(b"MXN");
        }
        for &billete in &[20u32, 50, 100, 200, 500, 1000] {
            trama.extend_from_slice(&billete.to_le_bytes());
        }

        let (unit_type, canales) = parse_setup_request(0, &trama).expect("Debe parsear billetero");
        assert_eq!(unit_type, 0x06);
        assert_eq!(canales, vec![2000, 5000, 10000, 20000, 50000, 100000]);
    }

    #[test]
    fn test_parse_setup_monedero_ssp16_mxn() {
        let mut trama = vec![
            0xF0, 0x03, b'0', b'2', b'1', b'0', b'M', b'X', b'N', 0x06, 0x04,
        ];
        for &moneda_cents in &[100u16, 200, 500, 1000] {
            trama.extend_from_slice(&moneda_cents.to_le_bytes());
        }

        let (unit_type, canales) = parse_setup_request(16, &trama).expect("Debe parsear monedero");
        assert_eq!(unit_type, 0x03);
        assert_eq!(canales, vec![100, 200, 500, 1000]);
    }

    #[test]
    fn test_poll_credito_billete_y_moneda() {
        let canales_billetero = vec![2000, 5000, 10000, 20000, 50000, 100000];
        let trama_billete = vec![0xF0, 0xEF, 0x03, 0xEE, 0x05];
        let eventos = parse_poll_response(
            &trama_billete,
            "BILL-01",
            ffi::DeviceType::Bill,
            &canales_billetero,
        )
        .unwrap();

        assert_eq!(eventos.len(), 1);
        assert_eq!(eventos[0].kind, ffi::EventKind::Credit);
        assert_eq!(eventos[0].value_cents, 50000);
        assert_eq!(eventos[0].value_pesos, 500);

        let mut trama_moneda = vec![0xF0, 0xDF];
        trama_moneda.extend_from_slice(&1000i32.to_le_bytes());
        trama_moneda.extend_from_slice(b"MXN");

        let ev_moneda =
            parse_poll_response(&trama_moneda, "COIN-01", ffi::DeviceType::Coin, &[]).unwrap();
        assert_eq!(ev_moneda.len(), 1);
        assert_eq!(ev_moneda[0].kind, ffi::EventKind::Credit);
        assert_eq!(ev_moneda[0].value_pesos, 10);
    }

    #[test]
    fn test_construccion_comandos_ruta_y_payout() {
        let cmd_reciclador = build_set_route_cmd(2000, "MXN", true);
        assert_eq!(
            cmd_reciclador,
            vec![0x3B, 0x00, 0xD0, 0x07, 0x00, 0x00, b'M', b'X', b'N']
        );

        let cmd_cashbox = build_set_route_cmd(50000, "MXN", false);
        assert_eq!(cmd_cashbox[0], 0x3B);
        assert_eq!(cmd_cashbox[1], 0x01);

        let piezas = vec![
            ffi::PayoutItem {
                value_cents: 5000,
                count: 2,
            },
            ffi::PayoutItem {
                value_cents: 2000,
                count: 1,
            },
        ];
        let cmd_pago = build_payout_by_denom_cmd(&piezas, "MXN");

        assert_eq!(cmd_pago[0], 0x46);
        assert_eq!(cmd_pago[1], 2);
        assert_eq!(*cmd_pago.last().unwrap(), 0x58);
    }

    #[test]
    fn test_calculo_cambio_sin_underflow() {
        let niveles = vec![
            ffi::DenomLevel {
                stored_in_cashbox: 0
                value_cents: 2000,
                value_pesos: 20,
                country_code: "MXN".into(),
                stored_in_payout: 0,
                route_to_payout: true,
            },
            ffi::DenomLevel {
                stored_in_cashbox: 0
                value_cents: 5000,
                value_pesos: 50,
                country_code: "MXN".into(),
                stored_in_payout: 5,
                route_to_payout: true,
            },
        ];

        let mut remanente = 70;
        let piezas = calcular_cambio_nativo(&mut remanente, &niveles, true);

        assert_eq!(piezas.len(), 1);
        assert_eq!(piezas[0].value_cents, 5000);
        assert_eq!(piezas[0].count, 1);
        assert_eq!(remanente, 20);
    }

    
    #[test]
    #[ignore]
    fn test_hardware_real_usb() {
        use std::fs;
        use std::time::Duration;

        println!("\n=== ESCANEANDO PUERTOS /dev/ttyUSB* ===");
        let mut dispositivos_encontrados = 0;

        let entradas = fs::read_dir("/dev").expect("No se pudo leer /dev");
        for entrada in entradas.flatten() {
            let path = entrada.path().to_string_lossy().to_string();
            if !path.contains("ttyUSB") {
                continue;
            }

            println!("-> Probando puerto {}...", path);
            let ssp = detectar_ssp_en_puerto(&path);

            if ssp != -1 {
                dispositivos_encontrados += 1;
                let tipo_str = if ssp == 16 { "MONEDERO (SSP 16)" } else { "BILLETERO (SSP 0)" };
                println!("   [OK] Detectado {} en {}", tipo_str, path);

                let rutas_prueba = if ssp == 0 {
                    vec![
                        ffi::RouteRule { value_cents: 2000, to_payout: true },
                        ffi::RouteRule { value_cents: 5000, to_payout: true },
                        ffi::RouteRule { value_cents: 10000, to_payout: true },
                        ffi::RouteRule { value_cents: 20000, to_payout: true },
                        ffi::RouteRule { value_cents: 50000, to_payout: false },
                        ffi::RouteRule { value_cents: 100000, to_payout: false },
                    ]
                } else {
                    vec![]
                };

                let mut unidad = new_essp_unit(&path, ssp as u8);
                match unidad.iniciar_y_conectar(true, &rutas_prueba) {
                    Ok(niveles_iniciales) => {
                        println!(
                            "   [OK] Encriptación AES activa | UnitType: 0x{:02X} | Serie: {}",
                            unidad.unit_type,
                            unidad.get_serial_id()
                        );
                        for lvl in &niveles_iniciales {
                            println!(
                                "        - ${:>4} {} | En Reciclador: {:>3} | Ruta: {}",
                                lvl.value_pesos,
                                lvl.country_code,
                                lvl.stored_in_payout,
                                if lvl.route_to_payout { "Reciclador" } else { "Cashbox" }
                            );
                        }

                        // Solo hacemos la prueba interactiva en el Monedero (o cambia a true si tienes billetes)
                        if ssp == 16 {
                            println!("\n   [1/3] Escuchando por 10 segundos (inserta una o más monedas ahora)...");
                            for _ in 0..100 {
                                if let Ok(eventos) = unidad.hacer_poll() {
                                    for ev in eventos {
                                        println!(
                                            "        >>> [EVENTO] {:?} | ${} MXN ({})",
                                            ev.kind, ev.value_pesos, ev.message
                                        );
                                    }
                                }
                                std::thread::sleep(Duration::from_millis(50));
                            }

                            // Leemos los nuevos niveles para saber exactamente cuántas monedas entraron
                            let niveles_actuales = unidad.obtener_niveles().expect("Error leyendo niveles");
                            let mut piezas_a_devolver = Vec::new();

                            println!("   [OK] Niveles tras inserción:");
                            for (ini, act) in niveles_iniciales.iter().zip(niveles_actuales.iter()) {
                                let diferencia = act.stored_in_payout - ini.stored_in_payout;
                                println!(
                                    "        - ${:>4} {} | Antes: {:>3} -> Ahora: {:>3} (Entraron: +{})",
                                    act.value_pesos, act.country_code, ini.stored_in_payout, act.stored_in_payout, diferencia
                                );
                                if diferencia > 0 {
                                    piezas_a_devolver.push(ffi::PayoutItem {
                                        value_cents: act.value_cents,
                                        count: diferencia,
                                    });
                                }
                            }

                            // Si aún no quieres meter otra moneda, devolvemos 1 moneda de $1 MXN de la prueba anterior:
                            if piezas_a_devolver.is_empty() {
                                println!("   [INFO] No entraron monedas nuevas en esta ventana; devolviendo $1 MXN de la prueba anterior...");
                                piezas_a_devolver.push(ffi::PayoutItem {
                                    value_cents: 1000,
                                    count: 5,
                                });
                            }

                            println!("\n   [2/3] Dispensando monedas: {:?}...", piezas_a_devolver);
                            unidad
                                .dispensar_piezas(&piezas_a_devolver)
                                .expect("Fallo al enviar comando de dispensado (0x46)");

                            // Esperamos hasta que el hardware emita Dispensed (0xD2)
                            println!("   [3/3] Esperando entrega física (máx. 30s)...");
                            let mut entregado = false;
                            let mut error_pago: Option<String> = None;

                            // 300 iteraciones x 100ms = 30 segundos (tiempo suficiente para el timeout nativo de ITL)
                            for _ in 0..300 {
                                if let Ok(eventos) = unidad.hacer_poll() {
                                    for ev in eventos {
                                        println!(
                                            "        >>> [PAGO] {:?} | ${} MXN {}",
                                            ev.kind, ev.value_pesos, ev.message
                                        );
                                        match ev.kind {
                                            ffi::EventKind::Dispensed => {
                                                entregado = true;
                                            }
                                            ffi::EventKind::Timeout
                                            | ffi::EventKind::IncompletePayout
                                            | ffi::EventKind::Jammed
                                            | ffi::EventKind::Error => {
                                                error_pago = Some(format!(
                                                    "{:?}: {}",
                                                    ev.kind, ev.message
                                                ));
                                            }
                                            _ => {}
                                        }
                                    }
                                }
                                if entregado || error_pago.is_some() {
                                    break;
                                }
                                std::thread::sleep(Duration::from_millis(100));
                            }

                            // Verificación final del estado del pago
                            if let Some(motivo) = error_pago {
                                println!("   [ERROR DE PAGO] El hardware reportó fallo: {}", motivo);
                            } else if !entregado {
                                println!("   [ERROR TIMEOUT] Tiempo agotado (30s) esperando entrega física. Abortando giro de motor (0x38)...");
                                let _ = unidad.detener_pago();
                            } else {
                                println!("   [OK] Pago entregado exitosamente.");
                            }

                            if let Ok(niveles_finales) = unidad.obtener_niveles() {
                                println!("   [OK] Niveles finales después de devolver el efectivo:");
                                for lvl in &niveles_finales {
                                    println!(
                                        "        - ${:>4} {} | En Reciclador: {:>3}",
                                        lvl.value_pesos, lvl.country_code, lvl.stored_in_payout
                                    );
                                }
                            }
                        }

                        unidad.detener_y_desconectar();
                        println!("   [OK] Dispositivo en {} desconectado limpiamente.\n", path);
                    }
                    Err(e) => panic!("Fallo al inicializar eSSP en {}: {}", path, e),
                }
            }
        }

        assert!(
            dispositivos_encontrados > 0,
            "No se detectó ningún validador eSSP conectado en /dev/ttyUSB*"
        );
    }
}