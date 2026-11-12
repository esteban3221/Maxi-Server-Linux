

#![allow(non_snake_case, non_camel_case_types, dead_code)]
use std::ffi::{c_char, c_int, c_uchar, c_ulong, c_ulonglong, CString};

/// Comandos de salida del protocolo eSSP (Host -> Validador)
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SspCommand {
    Reset = 0x01,                    // SSP_CMD_RESET
    SetInhibits = 0x02,              // SSP_CMD_SET_INHIBITS
    SetupRequest = 0x05,             // SSP_CMD_SETUP_REQUEST
    HostProtocolVersion = 0x06,      // SSP_CMD_HOST_PROTOCOL_VERSION
    Poll = 0x07,                     // SSP_CMD_POLL
    RejectNote = 0x08,               // SSP_CMD_REJECT_NOTE
    Disable = 0x09,                  // SSP_CMD_DISABLE
    Enable = 0x0A,                   // SSP_CMD_ENABLE
    GetSerialNumber = 0x0C,          // SSP_CMD_GET_SERIAL_NUMBER
    Sync = 0x11,                     // SSP_CMD_SYNC
    GetAllLevels = 0x22,             // SSP_CMD_GET_ALL_LEVELS
    HaltPayout = 0x38,               // SSP_CMD_HALT_PAYOUT
    SetDenomRoute = 0x3B,            // SSP_CMD_SET_DENOMINATION_ROUTE
    GetDenomRoute = 0x3C,            // SSP_CMD_GET_DENOMINATION_ROUTE
    EmptyAll = 0x3F,                 // SSP_CMD_EMPTY_ALL
    SetCoinMechInhibits = 0x40,      // SSP_CMD_SET_COIN_MECH_INHIBITS
    PayoutByDenom = 0x46,            // SSP_CMD_PAYOUT_BY_DENOMINATION
    SetCoinGlobalInhibit = 0x49,     // SSP_CMD_SET_COIN_MECH_GLOBAL_INHIBIT
    SmartEmpty = 0x52,               // SSP_CMD_SMART_EMPTY
    DisablePayoutDevice = 0x5B,      // SSP_CMD_DISABLE_PAYOUT_DEVICE
    EnablePayoutDevice = 0x5C,       // SSP_CMD_ENABLE_PAYOUT_DEVICE
}
impl SspCommand {
    pub fn from_byte(b: u8) -> String {
        match b {
            0x01 => "Reset".into(),
            0x02 => "SetInhibits".into(),
            0x05 => "SetupRequest".into(),
            0x06 => "HostProtocolVersion".into(),
            0x07 => "Poll".into(),
            0x08 => "RejectNote".into(),
            0x09 => "Disable".into(),
            0x0A => "Enable".into(),
            0x0C => "GetSerialNumber".into(),
            0x11 => "Sync".into(),
            0x22 => "GetAllLevels".into(),
            0x38 => "HaltPayout".into(),
            0x3B => "SetDenomRoute".into(),
            0x3C => "GetDenomRoute".into(),
            0x3F => "EmptyAll".into(),
            0x40 => "SetCoinMechInhibits".into(),
            0x46 => "PayoutByDenom".into(),
            0x49 => "SetCoinGlobalInhibit".into(),
            0x52 => "SmartEmpty".into(),
            0x5B => "DisablePayoutDevice".into(),
            0x5C => "EnablePayoutDevice".into(),
            other => format!("Unknown(0x{:02X})", other),
        }
    }
}

/// Códigos de estatus de respuesta (primer byte: resp[0])
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SspStatus {
    Ok,                       // 0xF0: SSP_RESPONSE_OK
    SlaveReset,               // 0xF1: SSP_RESPONSE_SLAVE_RESET
    CommandNotKnown,          // 0xF2: SSP_RESPONSE_COMMAND_NOT_KNOWN
    WrongNoParameters,        // 0xF3: SSP_RESPONSE_WRONG_NO_PARAMETERS
    ParameterOutOfRange,      // 0xF4: SSP_RESPONSE_PARAMETER_OUT_OF_RANGE
    CommandCannotBeProcessed, // 0xF5: SSP_RESPONSE_COMMAND_CANNOT_BE_PROCESSED
    SoftwareError,            // 0xF6: SSP_RESPONSE_SOFTWARE_ERROR
    Fail,                     // 0xF8: SSP_RESPONSE_FAIL
    KeyNotSet,                // 0xFA: SSP_RESPONSE_KEY_NOT_SET
    Unknown(u8),
}

impl SspStatus {
    pub fn from_byte(b: u8) -> Self {
        match b {
            0xF0 => Self::Ok,
            0xF1 => Self::SlaveReset,
            0xF2 => Self::CommandNotKnown,
            0xF3 => Self::WrongNoParameters,
            0xF4 => Self::ParameterOutOfRange,
            0xF5 => Self::CommandCannotBeProcessed,
            0xF6 => Self::SoftwareError,
            0xF8 => Self::Fail,
            0xFA => Self::KeyNotSet,
            other => Self::Unknown(other),
        }
    }
}

/// Eventos devueltos dentro del payload de SSP_CMD_POLL (0x07)
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SspPollEvent {
    Disabled,           // 0xE8: Dispositivo deshabilitado
    ReadNote,           // 0xEF: Billete en lectura / Escrow (2 bytes)
    CreditNote,         // 0xEE: Billete acreditado (2 bytes)
    NoteRejecting,      // 0xED: Rechazando billete
    NoteRejected,       // 0xEC: Billete rechazado
    NoteStacking,       // 0xCC: Apilando billete
    NoteStacked,        // 0xEB: Billete apilado en Cashbox
    SafeNoteJam,        // 0xEA: Atasco seguro
    UnsafeNoteJam,      // 0xE9: Atasco inseguro
    FraudAttempt,       // 0xE6: Intento de fraude
    StackerFull,        // 0xE7: Cashbox lleno
    CashboxRemoved,     // 0xE3: Casette retirado
    CashboxReplaced,    // 0xE4: Casette colocado
    NoteStoredInPayout, // 0xDB: Billete guardado en reciclador
    Dispensing,         // 0xDA: Dispensando efectivo (9 bytes)
    Dispensed,          // 0xD2: Efectivo dispensado (9 bytes)
    Halted,             // 0xD6: Motor detenido por HALT_PAYOUT (9 bytes)
    Floating,           // 0xD7: Flotando efectivo
    Floated,            // 0xD8: Efectivo flotado
    Timeout,            // 0xD9: Timeout dispensando (9 bytes)
    IncompletePayout,   // 0xDC: Pago incompleto (13 bytes)
    Jammed,             // 0xD5: Atasco en dispensador
    CoinCredit,         // 0xDF: Moneda acreditada (8 o 9 bytes)
    CoinFeederRunning,  // 0xC1: Motor alimentador de monedas activo
    CoinFeeding,        // 0xBE: Leyendo moneda en tolva (9 bytes)
    ValueAdded,         // 0xBF: Moneda ingresada en SMART Coin System (9 bytes)
    Emptying,           // 0xC2: Vaciando a cashbox
    Emptied,            // 0xC3: Vaciado completado
    SmartEmptying,      // 0xB3: Vaciado inteligente en curso (9 bytes)
    SmartEmptied,       // 0xB4: Vaciado inteligente completado (9 bytes)
    ChannelDisable,     // 0xB5: Canales inhibidos
    Unknown(u8),
}

impl SspPollEvent {
    pub fn from_byte(b: u8) -> Self {
        match b {
            0xE8 => Self::Disabled,
            0xEF => Self::ReadNote,
            0xEE => Self::CreditNote,
            0xED => Self::NoteRejecting,
            0xEC => Self::NoteRejected,
            0xCC => Self::NoteStacking,
            0xEB => Self::NoteStacked,
            0xEA => Self::SafeNoteJam,
            0xE9 => Self::UnsafeNoteJam,
            0xE6 => Self::FraudAttempt,
            0xE7 => Self::StackerFull,
            0xE3 => Self::CashboxRemoved,
            0xE4 => Self::CashboxReplaced,
            0xDB => Self::NoteStoredInPayout,
            0xDA => Self::Dispensing,
            0xD2 => Self::Dispensed,
            0xD6 => Self::Halted,
            0xD7 => Self::Floating,
            0xD8 => Self::Floated,
            0xD9 => Self::Timeout,
            0xDC => Self::IncompletePayout,
            0xD5 => Self::Jammed,
            0xDF => Self::CoinCredit,
            0xC1 => Self::CoinFeederRunning,
            0xBE => Self::CoinFeeding,
            0xBF => Self::ValueAdded,
            0xC2 => Self::Emptying,
            0xC3 => Self::Emptied,
            0xB3 => Self::SmartEmptying,
            0xB4 => Self::SmartEmptied,
            0xB5 => Self::ChannelDisable,
            other => Self::Unknown(other),
        }
    }
}
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
    fn send_cmd(
        &mut self,
        command: SspCommand,
        args: &[u8],
        usar_encriptacion: bool,
    ) -> Result<Vec<u8>, String> {
        let mut payload = Vec::with_capacity(1 + args.len());
        payload.push(command as u8);
        payload.extend_from_slice(args);
        self.send_raw(&payload, usar_encriptacion)
    }

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
                "Timeout/Error serial enviando {} a SSP {}",
                SspCommand::from_byte(data[0]),
                self.ssp_address
            ));
        }
        self.key = cmd.Key;

        let len = cmd.ResponseDataLength as usize;
        let resp = cmd.ResponseData[..len].to_vec();
        if resp.is_empty() {
            return Err("Respuesta vacía del dispositivo".into());
        }

        let status = SspStatus::from_byte(resp[0]);
        if status != SspStatus::Ok {
            return Err(format!(
                "Error SSP {:?} en cmd {}: {:02X?}",
                status,
                SspCommand::from_byte(data[0]),
                resp
            ));
        }
        Ok(resp)
    }

    pub fn sincronizar(&mut self) -> bool {
        let c_port = match CString::new(self.puerto.as_str()) {
            Ok(s) => s,
            Err(_) => return false,
        };
        let handle = unsafe { OpenSSPPort(c_port.as_ptr()) };
        if handle < 0 {
            return false;
        }
        self.port_handle = handle;
        let ok = self.send_cmd(SspCommand::Sync, &[], false).is_ok();
        unsafe { CloseSSPPort(self.port_handle) };
        self.port_handle = -1;
        ok
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

        // 1. Sincronización inicial
        self.send_cmd(SspCommand::Sync, &[], false)?;

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

        // 3. Protocolo v6
        self.send_cmd(SspCommand::HostProtocolVersion, &[0x06], true)?;

        // 4. Obtener Número de Serie
        let resp_ser = self.send_cmd(SspCommand::GetSerialNumber, &[], true)?;
        if resp_ser.len() >= 5 {
            let serial = u32::from_be_bytes([resp_ser[1], resp_ser[2], resp_ser[3], resp_ser[4]]);
            self.serial_id = serial.to_string();
        }

        // 5. Setup Request
        let resp_setup = self.send_cmd(SspCommand::SetupRequest, &[], true)?;
        let (unit_type, canales) = parse_setup_request(self.ssp_address, &resp_setup)?;
        self.unit_type = unit_type;
        self.canales_centavos = canales;

        // 6. Habilitar módulo reciclador si es billetero
        if self.send_cmd(SspCommand::EnablePayoutDevice, &[0x00], true).is_err() {
            let _ = self.send_cmd(SspCommand::EnablePayoutDevice, &[], true);
        }

        // 7. Configurar rutas iniciales
        for regla in rutas {
            let _ = self.cambiar_ruta(regla.value_cents, regla.to_payout);
        }

        // 8. Leer niveles actuales
        let niveles = self.obtener_niveles()?;

        // 9. Configurar Inhibits según si debe aceptar efectivo o solo dispensar
        if habilitar_recolector {
            let _ = self.send_cmd(SspCommand::SetInhibits, &[0xFF, 0xFF], true);
            if self.ssp_address == 16 {
                let _ = self.send_cmd(SspCommand::SetCoinGlobalInhibit, &[0x01], true);
                for &val in &self.canales_centavos.clone() {
                    let mut args_v6 = vec![0x01];
                    args_v6.extend_from_slice(&(val as u16).to_le_bytes());
                    args_v6.extend_from_slice(b"MXN");
                    if self.send_cmd(SspCommand::SetCoinMechInhibits, &args_v6, true).is_err() {
                        let mut args_short = vec![0x01];
                        args_short.extend_from_slice(&(val as u16).to_le_bytes());
                        let _ = self.send_cmd(SspCommand::SetCoinMechInhibits, &args_short, true);
                    }
                }
            }
        } else {
            let _ = self.send_cmd(SspCommand::SetInhibits, &[0x00, 0x00], true);
            if self.ssp_address == 16 {
                let _ = self.send_cmd(SspCommand::SetCoinGlobalInhibit, &[0x00], true);
            }
        }

        // 10. Habilitar unidad
        self.send_cmd(SspCommand::Enable, &[], true)?;

        Ok(niveles)
    }

    pub fn detener_y_desconectar(&mut self) {
        if self.port_handle >= 0 {
            if self.encrypted {
                let _ = self.send_cmd(SspCommand::SetInhibits, &[0x00, 0x00], true);
                if self.ssp_address == 16 {
                    let _ = self.send_cmd(SspCommand::SetCoinGlobalInhibit, &[0x00], true);
                } else {
                    let _ = self.send_cmd(SspCommand::DisablePayoutDevice, &[], true);
                }
                let _ = self.send_cmd(SspCommand::Disable, &[], true);
            }
            unsafe { CloseSSPPort(self.port_handle) };
            self.port_handle = -1;
            self.encrypted = false;
        }
    }

    pub fn obtener_niveles(&mut self) -> Result<Vec<ffi::DenomLevel>, String> {
        let resp = self.send_cmd(SspCommand::GetAllLevels, &[], true)?;
        let mut niveles = parse_all_levels(&resp)?;

        for item in &mut niveles {
            let mut args = Vec::with_capacity(7);
            args.extend_from_slice(&item.value_cents.to_le_bytes());
            args.extend_from_slice(item.country_code.as_bytes());
            if let Ok(r_resp) = self.send_cmd(SspCommand::GetDenomRoute, &args, true) {
                if r_resp.len() >= 2 {
                    item.route_to_payout = r_resp[1] == 0x00;
                }
            }
        }
        Ok(niveles)
    }

    pub fn cambiar_ruta(&mut self, valor_centavos: i32, a_reciclador: bool) -> Result<(), String> {
        let mut args = Vec::with_capacity(8);
        args.push(if a_reciclador { 0x00 } else { 0x01 });
        args.extend_from_slice(&valor_centavos.to_le_bytes());
        args.extend_from_slice(b"MXN");
        self.send_cmd(SspCommand::SetDenomRoute, &args, true)?;
        Ok(())
    }
    

    pub fn dispensar_piezas(&mut self, piezas: &[ffi::PayoutItem]) -> Result<(), String> {
        if piezas.is_empty() {
            return Ok(());
        }

        // 1. Inhibir admisión de efectivo para que el alimentador/escrow termine su ciclo
        let _ = self.send_cmd(SspCommand::SetInhibits, &[0x00, 0x00], true);
        if self.ssp_address == 16 {
            let _ = self.send_cmd(SspCommand::SetCoinGlobalInhibit, &[0x00], true);
        }
        let _ = self.send_cmd(SspCommand::Enable, &[], true);

        // 2. Si es monedero (SSP 16), esperar a que termine de girar el alimentador (0xC1 / 0xBE)
        if self.ssp_address == 16 {
            for _ in 0..30 {
                match self.send_cmd(SspCommand::Poll, &[], true) {
                    Ok(resp) => {
                        let ocupado = resp[1..].iter().any(|&b| {
                            matches!(
                                SspPollEvent::from_byte(b),
                                SspPollEvent::CoinFeederRunning | SspPollEvent::CoinFeeding
                            )
                        });
                        if !ocupado {
                            break;
                        }
                    }
                    Err(_) => break,
                }
                std::thread::sleep(std::time::Duration::from_millis(300));
            }
        }

        // 3. Enviar PayoutByDenom (0x46) con reintentos mientras el hardware siga ocupado ([0xF5, 0x03])
        let cmd = build_payout_by_denom_cmd(piezas, "MXN");
        let mut ultimo_err = String::new();

        for intento in 0..20 {
            if intento > 0 {
                std::thread::sleep(std::time::Duration::from_millis(400));
                let _ = self.send_cmd(SspCommand::Poll, &[], true);
            }

            match self.send_raw(&cmd, true) {
                Ok(_) => return Ok(()),
                Err(e) => {
                    ultimo_err = e;
                    if !ultimo_err.contains("CommandCannotBeProcessed") {
                        return Err(ultimo_err);
                    }
                }
            }
        }

        Err(ultimo_err)
    }

    pub fn vaciar_reciclador(&mut self, smart_empty: bool) -> Result<(), String> {
        let cmd = if smart_empty {
            SspCommand::SmartEmpty
        } else {
            SspCommand::EmptyAll
        };
        self.send_cmd(cmd, &[], true)?;
        Ok(())
    }

    pub fn detener_pago(&mut self) -> Result<(), String> {
        self.send_cmd(SspCommand::HaltPayout, &[], true)?;
        Ok(())
    }

    pub fn rechazar_escrow(&mut self) -> Result<(), String> {
        self.send_cmd(SspCommand::RejectNote, &[], true)?;
        Ok(())
    }

    pub fn reiniciar(&mut self) -> Result<(), String> {
        self.send_cmd(SspCommand::Reset, &[], self.encrypted)?;
        unsafe { CloseSSPPort(self.port_handle) };
        self.port_handle = -1;
        self.encrypted = false;
        Ok(())
    }

    pub fn hacer_poll(&mut self) -> Result<Vec<ffi::CashEvent>, String> {
        let resp = self.send_cmd(SspCommand::Poll, &[], true)?;
        parse_poll_response(
            &resp,
            &self.serial_id,
            self.get_device_type(),
            &self.canales_centavos,
        )
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
        Ok(())
    }

    pub fn limpiar_niveles_cashbox(&mut self) {}

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

pub fn parse_all_levels(resp: &[u8]) -> Result<Vec<ffi::DenomLevel>, String> {
    if resp.len() < 2 || SspStatus::from_byte(resp[0]) != SspStatus::Ok {
        return Err("Respuesta inválida en GET_ALL_LEVELS".into());
    }

    let count = resp[1] as usize;
    let mut niveles = Vec::with_capacity(count);
    let mut offset = 2;

    for _ in 0..count {
        if offset + 9 > resp.len() {
            break;
        }
        let stored = u16::from_le_bytes([resp[offset], resp[offset + 1]]) as i32;
        let value_cents = i32::from_le_bytes([
            resp[offset + 2],
            resp[offset + 3],
            resp[offset + 4],
            resp[offset + 5],
        ]);
        let country_code = String::from_utf8_lossy(&resp[offset + 6..offset + 9]).to_string();

        niveles.push(ffi::DenomLevel {
            value_cents,
            value_pesos: value_cents / 100,
            country_code,
            stored_in_payout: stored,
            stored_in_cashbox: 0,
            route_to_payout: false,
        });

        offset += 9;
    }

    Ok(niveles)
}

pub fn build_payout_by_denom_cmd(piezas: &[ffi::PayoutItem], country: &str) -> Vec<u8> {
    let mut cmd = Vec::with_capacity(3 + piezas.len() * 9);
    cmd.push(SspCommand::PayoutByDenom as u8);
    cmd.push(piezas.len() as u8);

    for item in piezas {
        cmd.extend_from_slice(&(item.count as u16).to_le_bytes());
        cmd.extend_from_slice(&item.value_cents.to_le_bytes());
        cmd.extend_from_slice(country.as_bytes());
    }
    cmd.push(0x58); // 0x58 = Ejecutar pago real (0x19 = Test)
    cmd
}

pub fn parse_poll_response(
    resp: &[u8],
    device_id: &str,
    device_type: ffi::DeviceType,
    canales_centavos: &[i32],
) -> Result<Vec<ffi::CashEvent>, String> {
    if resp.is_empty() || SspStatus::from_byte(resp[0]) != SspStatus::Ok {
        return Err("Respuesta POLL inválida".into());
    }

    let mut eventos = Vec::new();
    let mut i = 1;

    while i < resp.len() {
        let ev_code = SspPollEvent::from_byte(resp[i]);
        match ev_code {
            SspPollEvent::ReadNote => {
                i += 2;
            }
            SspPollEvent::CreditNote => {
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
            SspPollEvent::CoinCredit | SspPollEvent::ValueAdded => {
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
                } else if i + 7 < resp.len() {
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
            SspPollEvent::CoinFeeding
            | SspPollEvent::Halted
            | SspPollEvent::SmartEmptying
            | SspPollEvent::SmartEmptied => {
                if i + 8 < resp.len() {
                    let num_divisas = (resp[i + 1] as usize).max(1);
                    i += 2 + (7 * num_divisas);
                } else {
                    i += 1;
                }
            }
            SspPollEvent::Dispensed | SspPollEvent::Dispensing | SspPollEvent::Timeout => {
                let kind = match ev_code {
                    SspPollEvent::Dispensed => ffi::EventKind::Dispensed,
                    SspPollEvent::Timeout => ffi::EventKind::Timeout,
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
            SspPollEvent::IncompletePayout => {
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
            SspPollEvent::FraudAttempt => {
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
            SspPollEvent::UnsafeNoteJam | SspPollEvent::SafeNoteJam | SspPollEvent::Jammed => {
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
            SspPollEvent::CashboxRemoved => {
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
            SspPollEvent::CashboxReplaced => {
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