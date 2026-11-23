mod ssp_crypto;
mod ssp_dh;
mod ssp_wire;
mod transport;

use std::borrow::Cow;
use std::collections::{HashMap, VecDeque};
use std::time::Duration;

use crate::ssp_crypto::SspCipher;
use crate::ssp_dh::negotiate_ssp_encryption;
use crate::ssp_wire::{SspWire, SSP_STEX};
use crate::transport::SerialTransport;

const DEFAULT_TIMEOUT: Duration = Duration::from_millis(1000);
const DETECT_TIMEOUT: Duration = Duration::from_millis(300);
const DEFAULT_COUNTRY: [u8; 3] = *b"MXN";

/// Comandos de salida del protocolo eSSP (Host -> Validador)
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SspCommand {
    Reset = 0x01,                // SSP_CMD_RESET
    SetInhibits = 0x02,          // SSP_CMD_SET_INHIBITS
    SetupRequest = 0x05,         // SSP_CMD_SETUP_REQUEST
    HostProtocolVersion = 0x06,  // SSP_CMD_HOST_PROTOCOL_VERSION
    Poll = 0x07,                 // SSP_CMD_POLL
    RejectNote = 0x08,           // SSP_CMD_REJECT_NOTE
    Disable = 0x09,              // SSP_CMD_DISABLE
    Enable = 0x0A,               // SSP_CMD_ENABLE
    GetSerialNumber = 0x0C,      // SSP_CMD_GET_SERIAL_NUMBER
    Sync = 0x11,                 // SSP_CMD_SYNC
    GetAllLevels = 0x22,         // SSP_CMD_GET_ALL_LEVELS
    HaltPayout = 0x38,           // SSP_CMD_HALT_PAYOUT
    SetDenomRoute = 0x3B,        // SSP_CMD_SET_DENOMINATION_ROUTE
    GetDenomRoute = 0x3C,        // SSP_CMD_GET_DENOMINATION_ROUTE
    EmptyAll = 0x3F,             // SSP_CMD_EMPTY_ALL
    SetCoinMechInhibits = 0x40,  // SSP_CMD_SET_COIN_MECH_INHIBITS
    PayoutByDenom = 0x46,        // SSP_CMD_PAYOUT_BY_DENOMINATION
    SetCoinGlobalInhibit = 0x49, // SSP_CMD_SET_COIN_MECH_GLOBAL_INHIBIT
    SmartEmpty = 0x52,           // SSP_CMD_SMART_EMPTY
    DisablePayoutDevice = 0x5B,  // SSP_CMD_DISABLE_PAYOUT_DEVICE
    EnablePayoutDevice = 0x5C,   // SSP_CMD_ENABLE_PAYOUT_DEVICE
}

impl SspCommand {
    pub fn from_byte(b: u8) -> Cow<'static, str> {
        match b {
            0x01 => Cow::Borrowed("Reset"),
            0x02 => Cow::Borrowed("SetInhibits"),
            0x05 => Cow::Borrowed("SetupRequest"),
            0x06 => Cow::Borrowed("HostProtocolVersion"),
            0x07 => Cow::Borrowed("Poll"),
            0x08 => Cow::Borrowed("RejectNote"),
            0x09 => Cow::Borrowed("Disable"),
            0x0A => Cow::Borrowed("Enable"),
            0x0C => Cow::Borrowed("GetSerialNumber"),
            0x11 => Cow::Borrowed("Sync"),
            0x22 => Cow::Borrowed("GetAllLevels"),
            0x38 => Cow::Borrowed("HaltPayout"),
            0x3B => Cow::Borrowed("SetDenomRoute"),
            0x3C => Cow::Borrowed("GetDenomRoute"),
            0x3F => Cow::Borrowed("EmptyAll"),
            0x40 => Cow::Borrowed("SetCoinMechInhibits"),
            0x46 => Cow::Borrowed("PayoutByDenom"),
            0x49 => Cow::Borrowed("SetCoinGlobalInhibit"),
            0x52 => Cow::Borrowed("SmartEmpty"),
            0x5B => Cow::Borrowed("DisablePayoutDevice"),
            0x5C => Cow::Borrowed("EnablePayoutDevice"),
            other => Cow::Owned(format!("Unknown(0x{:02X})", other)),
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
    SlaveReset,           // 0xF1: Reinicio del dispositivo (1 byte)
    ReadNote,             // 0xEF: Billete en lectura / Escrow (2 bytes)
    CreditNote,           // 0xEE: Billete acreditado (2 bytes)
    NoteRejecting,        // 0xED: Rechazando billete (1 byte)
    NoteRejected,         // 0xEC: Billete rechazado (1 byte)
    NoteStacking,         // 0xCC: Apilando billete (1 byte)
    NoteStacked,          // 0xEB: Billete apilado en Cashbox (1 byte)
    SafeNoteJam,          // 0xEA: Atasco seguro (1 byte)
    UnsafeNoteJam,        // 0xE9: Atasco inseguro (1 byte)
    Disabled,             // 0xE8: Dispositivo deshabilitado (1 byte)
    StackerFull,          // 0xE7: Cashbox lleno (1 byte)
    FraudAttempt,         // 0xE6: Intento de fraude (2 bytes o 2 + 7*n en v6)
    CashboxReplaced,      // 0xE4: Casette colocado (1 byte)
    CashboxRemoved,       // 0xE3: Casette retirado (1 byte)
    NoteClearedToCashbox, // 0xE2: Billete despejado hacia cashbox tras reinicio (2 bytes)
    NoteClearedFromFront, // 0xE1: Billete despejado por el frente tras reinicio (2 bytes)
    NotePathOpen,         // 0xE0: Ruta de billete abierta (1 byte)
    CoinCredit,           // 0xDF: Moneda acreditada (8 bytes exactos en v6)
    CashboxPaid,          // 0xDE: Efectivo movido a cashbox durante operación (2 + 7*n en v6)
    IncompleteFloat,      // 0xDD: Flotado incompleto (2 + 11*n en v6)
    IncompletePayout,     // 0xDC: Pago incompleto (2 + 11*n en v6)
    NoteStoredInPayout,   // 0xDB: Billete guardado en reciclador (1 byte)
    Dispensing,           // 0xDA: Dispensando efectivo (2 + 7*n en v6)
    Timeout,              // 0xD9: Timeout dispensando (2 + 7*n en v6)
    Floated,              // 0xD8: Efectivo flotado (2 + 7*n en v6)
    Floating,             // 0xD7: Flotando efectivo (2 + 7*n en v6)
    Halted,               // 0xD6: Motor detenido por HALT_PAYOUT (2 + 7*n en v6)
    Jammed,               // 0xD5: Atasco en dispensador (1 byte o 2 + 7*n en v6)
    Dispensed,            // 0xD2: Efectivo dispensado (2 + 7*n en v6)
    Emptied,              // 0xC3: Vaciado completado (1 byte)
    Emptying,             // 0xC2: Vaciando a cashbox (1 byte)
    CoinFeederRunning,    // 0xC1: Motor alimentador de monedas activo (1 byte)
    ValueAdded,           // 0xBF: Moneda ingresada en SMART Coin System (2 + 7*n en v6)
    CoinFeeding,          // 0xBE: Leyendo moneda en tolva (2 + 7*n en v6)
    ChannelDisable,       // 0xB5: Canales inhibidos (1 byte)
    SmartEmptied,         // 0xB4: Vaciado inteligente completado (2 + 7*n en v6)
    SmartEmptying,        // 0xB3: Vaciado inteligente en curso (2 + 7*n en v6)
    Unknown(u8),
}

impl SspPollEvent {
    pub fn from_byte(b: u8) -> Self {
        match b {
            0xF1 => Self::SlaveReset,
            0xEF => Self::ReadNote,
            0xEE => Self::CreditNote,
            0xED => Self::NoteRejecting,
            0xEC => Self::NoteRejected,
            0xCC => Self::NoteStacking,
            0xEB => Self::NoteStacked,
            0xEA => Self::SafeNoteJam,
            0xE9 => Self::UnsafeNoteJam,
            0xE8 => Self::Disabled,
            0xE7 => Self::StackerFull,
            0xE6 => Self::FraudAttempt,
            0xE4 => Self::CashboxReplaced,
            0xE3 => Self::CashboxRemoved,
            0xE2 => Self::NoteClearedToCashbox,
            0xE1 => Self::NoteClearedFromFront,
            0xE0 => Self::NotePathOpen,
            0xDF => Self::CoinCredit,
            0xDE => Self::CashboxPaid,
            0xDD => Self::IncompleteFloat,
            0xDC => Self::IncompletePayout,
            0xDB => Self::NoteStoredInPayout,
            0xDA => Self::Dispensing,
            0xD9 => Self::Timeout,
            0xD8 => Self::Floated,
            0xD7 => Self::Floating,
            0xD6 => Self::Halted,
            0xD5 => Self::Jammed,
            0xD2 => Self::Dispensed,
            0xC3 => Self::Emptied,
            0xC2 => Self::Emptying,
            0xC1 => Self::CoinFeederRunning,
            0xBF => Self::ValueAdded,
            0xBE => Self::CoinFeeding,
            0xB5 => Self::ChannelDisable,
            0xB4 => Self::SmartEmptied,
            0xB3 => Self::SmartEmptying,
            other => Self::Unknown(other),
        }
    }
}

// ==================== 1. PUENTE CXX HACIA C++ (ValidadorUnit / CashHub) ====================

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
        fn esta_conectado(self: &RustEsspUnit) -> bool;
        fn configurar_admision(
            self: &mut RustEsspUnit,
            habilitar_recolector: bool,
            rutas: &[RouteRule],
        ) -> Result<Vec<DenomLevel>>;
        fn pausar_sesion(self: &mut RustEsspUnit) -> Result<()>;

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
        fn cambiar_ruta(
            self: &mut RustEsspUnit,
            valor_centavos: i32,
            a_reciclador: bool,
        ) -> Result<()>;
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

fn open_transport(puerto: &str) -> Result<Box<dyn SerialTransport + Send>, String> {
    #[cfg(target_os = "espidf")]
    {
        Ok(Box::new(crate::transport::esp_io::EspFtdiPort::open(
            puerto,
        )?))
    }
    #[cfg(not(target_os = "espidf"))]
    {
        Ok(Box::new(crate::transport::pc_io::PcSerialPort::open(
            puerto,
        )?))
    }
}

// ==================== 2. IMPLEMENTACIÓN DE HARDWARE (RustEsspUnit) ====================

pub struct RustEsspUnit {
    puerto: String,
    transport: Option<Box<dyn SerialTransport + Send>>,
    wire: SspWire,
    unit_type: u8,
    serial_id: String,
    country_code: [u8; 3],
    fixed_key: u64,
    encrypt_key: u64,
    cipher: Option<SspCipher>,
    pkt_count: u32,
    encrypted: bool,
    canales_centavos: Vec<i32>,
    route_cache: HashMap<i32, bool>,
    pending_events: VecDeque<ffi::CashEvent>,
}

impl Drop for RustEsspUnit {
    fn drop(&mut self) {
        self.detener_y_desconectar();
    }
}

pub fn new_essp_unit(puerto: &str, ssp_address: u8) -> Box<RustEsspUnit> {
    Box::new(RustEsspUnit {
        puerto: puerto.to_string(),
        transport: None,
        wire: SspWire::new(ssp_address),
        unit_type: if ssp_address == 16 { 0x03 } else { 0x06 },
        serial_id: String::new(),
        country_code: DEFAULT_COUNTRY,
        fixed_key: 0x0123456701234567,
        encrypt_key: 0,
        cipher: None,
        pkt_count: 0,
        encrypted: false,
        canales_centavos: Vec::new(),
        route_cache: HashMap::new(),
        pending_events: VecDeque::new(),
    })
}

/// Abre el puerto una sola vez y prueba dirección 16 (Monedero) y luego 0 (Billetero).
/// Devuelve -1 si ningún validador responde con SSP_RESPONSE_OK (0xF0).
pub fn detectar_ssp_en_puerto(puerto: &str) -> i32 {
    let mut port = match open_transport(puerto) {
        Ok(p) => p,
        Err(_) => return -1,
    };

    for &ssp in &[16u8, 0u8] {
        let mut wire = SspWire::new(ssp);
        if let Ok(resp) = wire.transact(port.as_mut(), &[SspCommand::Sync as u8], DETECT_TIMEOUT, 2)
        {
            if resp.first().copied().map(SspStatus::from_byte) == Some(SspStatus::Ok) {
                return ssp as i32;
            }
        }
    }
    -1
}

impl RustEsspUnit {
    pub fn esta_conectado(&self) -> bool {
        self.transport.is_some() && self.encrypted && self.cipher.is_some()
    }

    fn aplicar_inhibiciones(&mut self, habilitar_recolector: bool) -> Result<(), String> {
        if habilitar_recolector {
            if self.is_coin_device() {
                self.send_cmd(SspCommand::SetCoinGlobalInhibit, &[0x01], true)?;
                let cc = self.country_code;
                for idx in 0..self.canales_centavos.len() {
                    let val = self.canales_centavos[idx];
                    let Ok(val_u16) = u16::try_from(val) else {
                        continue;
                    };
                    let mut args_v6 = [0u8; 6];
                    args_v6[0] = 0x01;
                    args_v6[1..3].copy_from_slice(&val_u16.to_le_bytes());
                    args_v6[3..6].copy_from_slice(&cc);

                    if self
                        .send_cmd(SspCommand::SetCoinMechInhibits, &args_v6, true)
                        .is_err()
                    {
                        let mut args_short = [0u8; 3];
                        args_short[0] = 0x01;
                        args_short[1..3].copy_from_slice(&val_u16.to_le_bytes());
                        let _ = self.send_cmd(SspCommand::SetCoinMechInhibits, &args_short, true);
                    }
                }
            } else {
                self.send_cmd(SspCommand::SetInhibits, &[0xFF, 0xFF], true)?;
            }
        } else if self.is_coin_device() {
            self.send_cmd(SspCommand::SetCoinGlobalInhibit, &[0x00], true)?;
        } else {
            self.send_cmd(SspCommand::SetInhibits, &[0x00, 0x00], true)?;
        }
        Ok(())
    }

    /// Camino rápido (~80-150 ms) sobre una sesión AES ya abierta:
    /// 1) Verifica sesión viva con POLL y limpia eventos residuales.
    /// 2) Envía solo las rutas que cambiaron respecto a `route_cache`.
    /// 3) Lee snapshot fresco con una sola trama `GetAllLevels`.
    /// 4) Aplica inhibiciones y envía `Enable (0x0A)`.
    pub fn configurar_admision(
        &mut self,
        habilitar_recolector: bool,
        rutas: &[ffi::RouteRule],
    ) -> Result<Vec<ffi::DenomLevel>, String> {
        if !self.esta_conectado() {
            return Err("Sesión eSSP no conectada".into());
        }

        self.pending_events.clear();

        // Comprueba criptográficamente que la llave AES y el eCount siguen sincronizados
        let _ = self.send_cmd(SspCommand::Poll, &[], true)?;

        // Solo actualiza en hardware las denominaciones cuya ruta realmente cambió
        for regla in rutas {
            if self.route_cache.get(&regla.value_cents) != Some(&regla.to_payout) {
                self.cambiar_ruta(regla.value_cents, regla.to_payout)?;
            }
        }

        // Snapshot previo a abrir admisión (1 sola consulta serial gracias a route_cache)
        let niveles = self.obtener_niveles()?;

        self.aplicar_inhibiciones(habilitar_recolector)?;
        self.send_cmd(SspCommand::Enable, &[], true)?;

        Ok(niveles)
    }

    /// Pone el validador en reposo seguro entre ventas (inhibido + Disable 0x09)
    /// manteniendo abiertos el puerto serial y la sesión AES.
    /// Si falla la comunicación, cierra la sesión para forzar reconexión limpia en la siguiente venta.
    pub fn pausar_sesion(&mut self) -> Result<(), String> {
        if !self.esta_conectado() {
            return Ok(());
        }

        self.pending_events.clear();

        let res = (|| -> Result<(), String> {
            self.aplicar_inhibiciones(false)?;
            self.send_cmd(SspCommand::Disable, &[], true)?;
            Ok(())
        })();

        if res.is_err() {
            self.detener_y_desconectar();
        }
        res
    }

    fn is_coin_device(&self) -> bool {
        matches!(self.unit_type, 0x03 | 0x09) || self.wire.address == 16
    }

    fn country_str(&self) -> &str {
        std::str::from_utf8(&self.country_code).unwrap_or("MXN")
    }

    fn invalidate_session(&mut self) {
        self.encrypted = false;
        self.encrypt_key = 0;
        self.cipher = None;
        self.pkt_count = 0;
    }

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
        if data.is_empty() {
            return Err("Intento de enviar trama eSSP vacía".into());
        }

        let port = self
            .transport
            .as_mut()
            .ok_or_else(|| "Puerto eSSP no abierto".to_string())?;

        let do_enc = usar_encriptacion && self.encrypted;
        let tx_payload: Cow<'_, [u8]> = if do_enc {
            let cipher = self
                .cipher
                .as_ref()
                .ok_or_else(|| "Sesión marcada como encriptada sin SspCipher".to_string())?;
            Cow::Owned(cipher.encrypt_payload(self.pkt_count, data)?)
        } else {
            Cow::Borrowed(data)
        };

        let rx_raw = self
            .wire
            .transact(port.as_mut(), &tx_payload, DEFAULT_TIMEOUT, 3)?;

        let resp = if rx_raw.first().copied() == Some(SSP_STEX) {
            let cipher = self.cipher.as_ref().ok_or_else(|| {
                "Se recibió trama encriptada (STEX) sin sesión AES activa".to_string()
            })?;
            if do_enc {
                self.pkt_count = self.pkt_count.wrapping_add(1);
            }
            cipher.decrypt_payload(self.pkt_count, &rx_raw)?
        } else {
            if do_enc && rx_raw.first().copied().map(SspStatus::from_byte) == Some(SspStatus::Ok) {
                self.pkt_count = self.pkt_count.wrapping_add(1);
            }
            rx_raw
        };

        if resp.is_empty() {
            return Err("Respuesta vacía del dispositivo".into());
        }

        let status = SspStatus::from_byte(resp[0]);
        if status != SspStatus::Ok {
            if matches!(status, SspStatus::KeyNotSet | SspStatus::SlaveReset) {
                self.invalidate_session();
            }
            return Err(format!(
                "Error SSP {:?} en cmd {}: {:02X?}",
                status,
                SspCommand::from_byte(data[0]),
                resp
            ));
        }

        Ok(resp)
    }

    /// Ejecuta un POLL interno y encola cualquier evento relevante para que nunca se pierda
    /// dinero ni alarmas entre llamadas de C++ a `hacer_poll()`.
    fn poll_internal(&mut self) -> Result<PollParseOutput, String> {
        let resp = self.send_cmd(SspCommand::Poll, &[], true)?;
        let out = parse_poll_response_detailed(
            &resp,
            &self.serial_id,
            self.get_device_type(),
            &self.canales_centavos,
        )?;
        if !out.events.is_empty() {
            self.pending_events.extend(out.events.iter().cloned());
        }
        Ok(out)
    }

    pub fn sincronizar(&mut self) -> bool {
        let mut wire = SspWire::new(self.wire.address);
        let resp_res = if let Some(port) = self.transport.as_mut() {
            wire.transact(port.as_mut(), &[SspCommand::Sync as u8], DEFAULT_TIMEOUT, 3)
        } else {
            let mut port = match open_transport(&self.puerto) {
                Ok(p) => p,
                Err(_) => return false,
            };
            wire.transact(port.as_mut(), &[SspCommand::Sync as u8], DEFAULT_TIMEOUT, 3)
        };

        match resp_res {
            Ok(resp) => resp.first().copied().map(SspStatus::from_byte) == Some(SspStatus::Ok),
            Err(_) => false,
        }
    }

    pub fn iniciar_y_conectar(
        &mut self,
        habilitar_recolector: bool,
        rutas: &[ffi::RouteRule],
    ) -> Result<Vec<ffi::DenomLevel>, String> {
        // Liberar cualquier handle previo para evitar EBUSY / contención de UART en reconexiones
        self.transport = None;
        self.invalidate_session();
        self.route_cache.clear();
        self.pending_events.clear();

        let mut port = open_transport(&self.puerto)?;
        self.wire.reset_seq();

        // 1. SYNC inicial verificando 0xF0
        let sync_resp =
            self.wire
                .transact(port.as_mut(), &[SspCommand::Sync as u8], DEFAULT_TIMEOUT, 2)?;
        if sync_resp.first().copied().map(SspStatus::from_byte) != Some(SspStatus::Ok) {
            return Err(format!("SYNC rechazado por hardware: {:02X?}", sync_resp));
        }

        // 2. Negociación Diffie-Hellman (0x4A, 0x4B, 0x4C) y caché de llave AES-128
        let (enc_key, init_cnt) =
            negotiate_ssp_encryption(&mut self.wire, port.as_mut(), DEFAULT_TIMEOUT)?;
        self.encrypt_key = enc_key;
        self.cipher = Some(SspCipher::new(self.fixed_key, enc_key));
        self.pkt_count = init_cnt;
        self.encrypted = true;
        self.transport = Some(port);

        // 3. Protocolo v6, número de serie y setup de canales
        self.send_cmd(SspCommand::HostProtocolVersion, &[0x06], true)?;

        let resp_ser = self.send_cmd(SspCommand::GetSerialNumber, &[], true)?;
        if resp_ser.len() >= 5 {
            let serial = u32::from_be_bytes([resp_ser[1], resp_ser[2], resp_ser[3], resp_ser[4]]);
            self.serial_id = serial.to_string();
        }

        let resp_setup = self.send_cmd(SspCommand::SetupRequest, &[], true)?;
        let (unit_type, canales, detected_country) =
            parse_setup_request_with_country(self.wire.address, &resp_setup)?;
        self.unit_type = unit_type;
        self.canales_centavos = canales;
        if let Some(cc) = detected_country {
            self.country_code = cc;
        }

        // 4. Habilitar módulo de pago solo en billeteros con reciclador (en SSP 16 devuelve 0xF2)
        if !self.is_coin_device() {
            if self
                .send_cmd(SspCommand::EnablePayoutDevice, &[0x00], true)
                .is_err()
            {
                self.send_cmd(SspCommand::EnablePayoutDevice, &[], true)?;
            }
        }

        // 5. Aplicar reglas de ruteo (propaga fallo para evitar rutear billetes altos al reciclador)
        for regla in rutas {
            self.cambiar_ruta(regla.value_cents, regla.to_payout)?;
        }

        // 6. Leer niveles y poblar caché inicial de rutas
        let niveles = self.obtener_niveles_desde_hardware(true)?;

        // 7. Configurar inhibiciones según tipo de dispositivo
        self.aplicar_inhibiciones(habilitar_recolector)?;
        self.send_cmd(SspCommand::Enable, &[], true)?;
        Ok(niveles)
    }

    pub fn detener_y_desconectar(&mut self) {
        if self.transport.is_some() {
            if self.encrypted && self.cipher.is_some() {
                let _ = self.send_cmd(SspCommand::SetInhibits, &[0x00, 0x00], true);
                if self.is_coin_device() {
                    let _ = self.send_cmd(SspCommand::SetCoinGlobalInhibit, &[0x00], true);
                } else {
                    let _ = self.send_cmd(SspCommand::SetInhibits, &[0x00, 0x00], true);
                    let _ = self.send_cmd(SspCommand::DisablePayoutDevice, &[], true);
                }
                let _ = self.send_cmd(SspCommand::Disable, &[], true);
            }
            self.transport = None; // Cierra el puerto al hacer Drop
            self.invalidate_session();
            self.route_cache.clear();
        }
    }

    fn obtener_niveles_desde_hardware(
        &mut self,
        forzar_consulta_rutas: bool,
    ) -> Result<Vec<ffi::DenomLevel>, String> {
        let resp = self.send_cmd(SspCommand::GetAllLevels, &[], true)?;
        let mut niveles = parse_all_levels(&resp)?;

        for item in &mut niveles {
            if !forzar_consulta_rutas {
                if let Some(&cached_route) = self.route_cache.get(&item.value_cents) {
                    item.route_to_payout = cached_route;
                    continue;
                }
            }

            if item.country_code.len() == 3 && item.country_code.is_ascii() {
                let mut args = [0u8; 7];
                args[0..4].copy_from_slice(&item.value_cents.to_le_bytes());
                args[4..7].copy_from_slice(item.country_code.as_bytes());
                if let Ok(r_resp) = self.send_cmd(SspCommand::GetDenomRoute, &args, true) {
                    if r_resp.len() >= 2 {
                        let to_payout = r_resp[1] == 0x00;
                        item.route_to_payout = to_payout;
                        self.route_cache.insert(item.value_cents, to_payout);
                    }
                }
            }
        }
        Ok(niveles)
    }

    pub fn obtener_niveles(&mut self) -> Result<Vec<ffi::DenomLevel>, String> {
        self.obtener_niveles_desde_hardware(false)
    }

    pub fn cambiar_ruta(&mut self, valor_centavos: i32, a_reciclador: bool) -> Result<(), String> {
        let cmd = build_set_route_cmd(valor_centavos, self.country_str(), a_reciclador)?;
        self.send_raw(&cmd, true)?;
        self.route_cache.insert(valor_centavos, a_reciclador);
        Ok(())
    }

    /// Inhibe la admisión de efectivo y envía `PayoutByDenom (0x46)`.
    /// Nota de contrato FFI: tras terminar el pago (al recibir `Dispensed` / error en `hacer_poll`),
    /// el llamador debe re-habilitar los canales si desea aceptar efectivo nuevamente.
    pub fn dispensar_piezas(&mut self, piezas: &[ffi::PayoutItem]) -> Result<(), String> {
        if piezas.is_empty() {
            return Ok(());
        }

        // 1. Validar y construir comando ANTES de tocar el estado del hardware
        let cmd = build_payout_by_denom_cmd(piezas, self.country_str())?;

        // 2. Inhibir admisión de efectivo para que el alimentador/escrow cierre su ciclo
        if self.is_coin_device() {
            self.send_cmd(SspCommand::SetCoinGlobalInhibit, &[0x00], true)?;
        } else {
            self.send_cmd(SspCommand::SetInhibits, &[0x00, 0x00], true)?;
        }
        self.send_cmd(SspCommand::Enable, &[], true)?;

        // 3. Si es monedero, esperar a que termine el alimentador usando el parser formal
        //    (nunca escanear bytes crudos ni descartar créditos recibidos).
        if self.is_coin_device() {
            for _ in 0..30 {
                match self.poll_internal() {
                    Ok(out) => {
                        if !out.feeder_busy {
                            break;
                        }
                    }
                    Err(_) => break,
                }
                std::thread::sleep(Duration::from_millis(250));
            }
        }

        // 4. Enviar PayoutByDenom (0x46) con reintentos si responde CommandCannotBeProcessed (0xF5)
        let mut ultimo_err = String::new();
        for intento in 0..20 {
            if intento > 0 {
                std::thread::sleep(Duration::from_millis(350));
                let _ = self.poll_internal();
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
        let enc = self.encrypted && self.cipher.is_some();
        let _ = self.send_cmd(SspCommand::Reset, &[], enc);
        self.transport = None;
        self.invalidate_session();
        self.route_cache.clear();
        self.pending_events.clear();
        Ok(())
    }

    pub fn hacer_poll(&mut self) -> Result<Vec<ffi::CashEvent>, String> {
        let mut eventos: Vec<ffi::CashEvent> = self.pending_events.drain(..).collect();

        let resp = self.send_cmd(SspCommand::Poll, &[], true)?;
        let parsed = parse_poll_response_detailed(
            &resp,
            &self.serial_id,
            self.get_device_type(),
            &self.canales_centavos,
        )?;
        eventos.extend(parsed.events);
        Ok(eventos)
    }

    pub fn dispensar_por_canales(&mut self, cantidades: &[i32]) -> Result<(), String> {
        let mut piezas = Vec::new();
        for (idx, &cant) in cantidades.iter().enumerate() {
            if cant < 0 {
                return Err(format!(
                    "Cantidad negativa ({}) en índice de canal {}",
                    cant, idx
                ));
            }
            if cant > 0 {
                let &val_cents = self.canales_centavos.get(idx).ok_or_else(|| {
                    format!(
                        "Índice de canal {} fuera de rango (total canales: {})",
                        idx,
                        self.canales_centavos.len()
                    )
                })?;
                piezas.push(ffi::PayoutItem {
                    value_cents: val_cents,
                    count: cant,
                });
            }
        }
        self.dispensar_piezas(&piezas)
    }

    /// En eSSP sin modo Hold explícito (0x18), el billete se acepta al continuar el ciclo de POLL.
    /// Si se activa Escrow/Hold en el futuro, aquí debe enviarse el siguiente POLL o comando de confirmación.
    pub fn aceptar_escrow(&mut self) -> Result<(), String> {
        let _ = self.poll_internal()?;
        Ok(())
    }

    pub fn limpiar_niveles_cashbox(&mut self) {
        // Los contadores persistentes de cashbox los mantiene la capa superior o el comando ResetCounters (0x59).
    }

    pub fn get_serial_id(&self) -> &str {
        &self.serial_id
    }

    pub fn get_device_type(&self) -> ffi::DeviceType {
        if self.is_coin_device() {
            ffi::DeviceType::Coin
        } else {
            ffi::DeviceType::Bill
        }
    }
}

// ==================== 3. LÓGICA PURA DE PROTOCOLO eSSP (TESTEABLE SIN HARDWARE) ====================

fn is_ascii_country_code(slice: &[u8]) -> bool {
    slice.len() == 3 && slice.iter().all(|b| b.is_ascii_uppercase())
}

fn format_currency(cents: i32, country: &str) -> String {
    let sign = if cents < 0 { "-" } else { "" };
    let abs = cents.unsigned_abs();
    format!("{}${}.{:02} {}", sign, abs / 100, abs % 100, country)
}

fn validate_country_code(country: &str) -> Result<[u8; 3], String> {
    let bytes = country.as_bytes();
    if bytes.len() != 3 || !bytes.iter().all(|b| b.is_ascii_uppercase()) {
        return Err(format!(
            "Código de divisa inválido '{}': debe ser exactamente 3 letras ASCII mayúsculas (ej. MXN)",
            country
        ));
    }
    Ok([bytes[0], bytes[1], bytes[2]])
}

pub fn parse_setup_request(ssp_address: u8, resp: &[u8]) -> Result<(u8, Vec<i32>), String> {
    let (unit_type, canales, _) = parse_setup_request_with_country(ssp_address, resp)?;
    Ok((unit_type, canales))
}

fn parse_setup_request_with_country(
    ssp_address: u8,
    resp: &[u8],
) -> Result<(u8, Vec<i32>, Option<[u8; 3]>), String> {
    if resp.len() < 11 || resp[0] != 0xF0 {
        return Err("Trama SETUP_REQUEST inválida o incompleta".into());
    }

    let unit_type = resp[1];
    let detected_country = if is_ascii_country_code(&resp[6..9]) {
        Some([resp[6], resp[7], resp[8]])
    } else {
        None
    };

    let mut canales_centavos = Vec::new();

    if ssp_address == 16 || unit_type == 0x03 || unit_type == 0x09 {
        let num_canales = resp[10] as usize;
        let expected_end = 11 + (2 * num_canales);
        if resp.len() < expected_end {
            return Err("Trama de monedero truncada en canales".into());
        }
        let mut offset = 11;
        for _ in 0..num_canales {
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
            if offset + (4 * num_canales) > resp.len() {
                return Err("Trama de billetero truncada en ExpandedChannelValues".into());
            }
            // En datasets MXN con real_mult = 1, raw_val viene en pesos enteros (x100 -> centavos).
            // Si el firmware ya reporta real_mult >= 100, raw_val * real_mult ya está en centavos.
            let mult = if real_mult >= 100 {
                real_mult
            } else {
                real_mult
                    .max(1)
                    .checked_mul(100)
                    .ok_or_else(|| "Overflow calculando multiplicador de billete".to_string())?
            };

            for _ in 0..num_canales {
                let raw_val = u32::from_le_bytes([
                    resp[offset],
                    resp[offset + 1],
                    resp[offset + 2],
                    resp[offset + 3],
                ]);
                let raw_i32 = i32::try_from(raw_val)
                    .map_err(|_| "Valor de canal excede rango i32".to_string())?;
                let cents = raw_i32
                    .checked_mul(mult)
                    .ok_or_else(|| "Overflow calculando centavos de canal".to_string())?;
                canales_centavos.push(cents);
                offset += 4;
            }
        } else {
            // Protocolo legacy (< v6): ValueMultiplier en resp[9..12], canales en resp[13..13+n]
            let legacy_mult =
                ((resp[9] as i32) << 16) | ((resp[10] as i32) << 8) | (resp[11] as i32);
            let mult = legacy_mult
                .max(1)
                .checked_mul(100)
                .ok_or_else(|| "Overflow en multiplicador legacy".to_string())?;
            for c in 0..num_canales {
                let raw_val = resp[13 + c] as i32;
                let cents = raw_val
                    .checked_mul(mult)
                    .ok_or_else(|| "Overflow en canal legacy".to_string())?;
                canales_centavos.push(cents);
            }
        }
    }

    Ok((unit_type, canales_centavos, detected_country))
}

pub fn build_set_route_cmd(
    valor_centavos: i32,
    country: &str,
    to_payout: bool,
) -> Result<Vec<u8>, String> {
    if valor_centavos <= 0 {
        return Err(format!(
            "Denominación inválida para ruta: {} centavos",
            valor_centavos
        ));
    }
    let cc = validate_country_code(country)?;
    let route_byte = if to_payout { 0x00 } else { 0x01 };
    let mut cmd = Vec::with_capacity(9);
    cmd.push(SspCommand::SetDenomRoute as u8);
    cmd.push(route_byte);
    cmd.extend_from_slice(&valor_centavos.to_le_bytes());
    cmd.extend_from_slice(&cc);
    Ok(cmd)
}

pub fn parse_all_levels(resp: &[u8]) -> Result<Vec<ffi::DenomLevel>, String> {
    if resp.len() < 2 || SspStatus::from_byte(resp[0]) != SspStatus::Ok {
        return Err("Respuesta inválida en GET_ALL_LEVELS".into());
    }

    let count = resp[1] as usize;
    let expected_len = 2 + (count * 9);
    if resp.len() < expected_len {
        return Err(format!(
            "Trama GET_ALL_LEVELS truncada: esperados {} bytes para {} denominaciones, recibidos {}",
            expected_len,
            count,
            resp.len()
        ));
    }

    let mut niveles = Vec::with_capacity(count);
    let mut offset = 2;

    for _ in 0..count {
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

pub fn build_payout_by_denom_cmd(
    piezas: &[ffi::PayoutItem],
    country: &str,
) -> Result<Vec<u8>, String> {
    if piezas.is_empty() {
        return Err("Lista de piezas a dispensar vacía".into());
    }
    let num_denoms = u8::try_from(piezas.len())
        .map_err(|_| format!("Demasiadas denominaciones en un pago ({})", piezas.len()))?;
    let cc = validate_country_code(country)?;

    let mut cmd = Vec::with_capacity(3 + piezas.len() * 9);
    cmd.push(SspCommand::PayoutByDenom as u8);
    cmd.push(num_denoms);

    for item in piezas {
        if item.value_cents <= 0 {
            return Err(format!(
                "Denominación inválida en payout: {} centavos",
                item.value_cents
            ));
        }
        if item.count <= 0 {
            return Err(format!(
                "Cantidad inválida en payout para {} centavos: {}",
                item.value_cents, item.count
            ));
        }
        let count_u16 = u16::try_from(item.count)
            .map_err(|_| format!("Cantidad fuera de rango u16: {}", item.count))?;

        cmd.extend_from_slice(&count_u16.to_le_bytes());
        cmd.extend_from_slice(&item.value_cents.to_le_bytes());
        cmd.extend_from_slice(&cc);
    }

    cmd.push(0x58); // 0x58 = Ejecutar pago real (0x19 = Test)
    Ok(cmd)
}

#[derive(Debug, Default)]
struct PollParseOutput {
    events: Vec<ffi::CashEvent>,
    feeder_busy: bool,
}

pub fn parse_poll_response(
    resp: &[u8],
    device_id: &str,
    device_type: ffi::DeviceType,
    canales_centavos: &[i32],
) -> Result<Vec<ffi::CashEvent>, String> {
    Ok(parse_poll_response_detailed(resp, device_id, device_type, canales_centavos)?.events)
}

fn parse_poll_response_detailed(
    resp: &[u8],
    device_id: &str,
    device_type: ffi::DeviceType,
    canales_centavos: &[i32],
) -> Result<PollParseOutput, String> {
    if resp.is_empty() || SspStatus::from_byte(resp[0]) != SspStatus::Ok {
        return Err("Respuesta POLL inválida".into());
    }

    let mut out = PollParseOutput::default();
    let mut i = 1;

    while i < resp.len() {
        let ev_code = SspPollEvent::from_byte(resp[i]);
        match ev_code {
            // Eventos de 2 bytes (código + canal) sin CashEvent emitido
            SspPollEvent::ReadNote
            | SspPollEvent::NoteClearedFromFront
            | SspPollEvent::NoteClearedToCashbox => {
                if i + 1 < resp.len() {
                    i += 2;
                } else {
                    break;
                }
            }

            SspPollEvent::CreditNote => {
                if i + 1 < resp.len() {
                    let canal = resp[i + 1] as usize;
                    if canal > 0 && canal <= canales_centavos.len() {
                        let cents = canales_centavos[canal - 1];
                        out.events.push(ffi::CashEvent {
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

            // eSSP v6 CoinCredit (0xDF): exactamente 8 bytes [0xDF, val(4), country(3)]
            SspPollEvent::CoinCredit => {
                if i + 7 < resp.len() {
                    let cents =
                        i32::from_le_bytes([resp[i + 1], resp[i + 2], resp[i + 3], resp[i + 4]]);
                    if cents > 0 {
                        out.events.push(ffi::CashEvent {
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

            // SMART Coin System ValueAdded (0xBF): [0xBF, num_divisas, (val(4), country(3)) * n]
            SspPollEvent::ValueAdded => {
                if i + 1 < resp.len() {
                    let num_divisas = (resp[i + 1] as usize).max(1);
                    let total_len = 2 + (7 * num_divisas);
                    if i + total_len <= resp.len() {
                        for k in 0..num_divisas {
                            let base = i + 2 + (7 * k);
                            let cents = i32::from_le_bytes([
                                resp[base],
                                resp[base + 1],
                                resp[base + 2],
                                resp[base + 3],
                            ]);
                            if cents > 0 {
                                out.events.push(ffi::CashEvent {
                                    device_id: device_id.into(),
                                    device_type,
                                    kind: ffi::EventKind::Credit,
                                    value_cents: cents,
                                    value_pesos: cents / 100,
                                    message: "Moneda acreditada".into(),
                                });
                            }
                        }
                        i += total_len;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }

            SspPollEvent::CoinFeederRunning => {
                out.feeder_busy = true;
                i += 1;
            }

            // Eventos multi-divisa v6 (2 + 7*n) o legacy (5 bytes) que no emiten CashEvent
            SspPollEvent::CoinFeeding
            | SspPollEvent::Halted
            | SspPollEvent::Floating
            | SspPollEvent::Floated
            | SspPollEvent::CashboxPaid
            | SspPollEvent::SmartEmptying
            | SspPollEvent::SmartEmptied => {
                if ev_code == SspPollEvent::CoinFeeding {
                    out.feeder_busy = true;
                }
                if i + 8 < resp.len()
                    && resp[i + 1] > 0
                    && is_ascii_country_code(&resp[i + 6..i + 9])
                {
                    let num_divisas = resp[i + 1] as usize;
                    let total_len = 2 + (7 * num_divisas);
                    if i + total_len <= resp.len() {
                        i += total_len;
                    } else {
                        break;
                    }
                } else if i + 4 < resp.len() {
                    i += 5;
                } else {
                    break;
                }
            }

            SspPollEvent::Dispensed | SspPollEvent::Dispensing | SspPollEvent::Timeout => {
                let kind = match ev_code {
                    SspPollEvent::Dispensed => ffi::EventKind::Dispensed,
                    SspPollEvent::Timeout => ffi::EventKind::Timeout,
                    _ => ffi::EventKind::Dispensing,
                };

                if i + 8 < resp.len()
                    && resp[i + 1] > 0
                    && is_ascii_country_code(&resp[i + 6..i + 9])
                {
                    let num_divisas = resp[i + 1] as usize;
                    let total_len = 2 + (7 * num_divisas);
                    if i + total_len <= resp.len() {
                        for k in 0..num_divisas {
                            let base = i + 2 + (7 * k);
                            let cents = i32::from_le_bytes([
                                resp[base],
                                resp[base + 1],
                                resp[base + 2],
                                resp[base + 3],
                            ]);
                            let cc =
                                std::str::from_utf8(&resp[base + 4..base + 7]).unwrap_or("MXN");
                            let msg = if kind == ffi::EventKind::Timeout {
                                format!(
                                    "Timeout del hardware dispensando (entregado parcial: {})",
                                    format_currency(cents, cc)
                                )
                            } else {
                                String::new()
                            };
                            out.events.push(ffi::CashEvent {
                                device_id: device_id.into(),
                                device_type,
                                kind,
                                value_cents: cents,
                                value_pesos: cents / 100,
                                message: msg,
                            });
                        }
                        i += total_len;
                    } else {
                        break;
                    }
                } else if i + 4 < resp.len() {
                    // Formato protocolo < v6: [ev, val(4)]
                    let cents =
                        i32::from_le_bytes([resp[i + 1], resp[i + 2], resp[i + 3], resp[i + 4]]);
                    let msg = if kind == ffi::EventKind::Timeout {
                        format!(
                            "Timeout del hardware dispensando (entregado parcial: {})",
                            format_currency(cents, "MXN")
                        )
                    } else {
                        String::new()
                    };
                    out.events.push(ffi::CashEvent {
                        device_id: device_id.into(),
                        device_type,
                        kind,
                        value_cents: cents,
                        value_pesos: cents / 100,
                        message: msg,
                    });
                    i += 5;
                } else {
                    break;
                }
            }

            SspPollEvent::IncompletePayout => {
                if i + 12 < resp.len()
                    && resp[i + 1] > 0
                    && is_ascii_country_code(&resp[i + 10..i + 13])
                {
                    let num_divisas = resp[i + 1] as usize;
                    let total_len = 2 + (11 * num_divisas);
                    if i + total_len <= resp.len() {
                        for k in 0..num_divisas {
                            let base = i + 2 + (11 * k);
                            let pagado_cents = i32::from_le_bytes([
                                resp[base],
                                resp[base + 1],
                                resp[base + 2],
                                resp[base + 3],
                            ]);
                            let pedido_cents = i32::from_le_bytes([
                                resp[base + 4],
                                resp[base + 5],
                                resp[base + 6],
                                resp[base + 7],
                            ]);
                            let cc =
                                std::str::from_utf8(&resp[base + 8..base + 11]).unwrap_or("MXN");
                            out.events.push(ffi::CashEvent {
                                device_id: device_id.into(),
                                device_type,
                                kind: ffi::EventKind::IncompletePayout,
                                value_cents: pagado_cents,
                                value_pesos: pagado_cents / 100,
                                message: format!(
                                    "Pago incompleto: se entregaron {} de {} solicitados",
                                    format_currency(pagado_cents, cc),
                                    format_currency(pedido_cents, cc)
                                ),
                            });
                        }
                        i += total_len;
                    } else {
                        break;
                    }
                } else if i + 8 < resp.len() {
                    let pagado_cents =
                        i32::from_le_bytes([resp[i + 1], resp[i + 2], resp[i + 3], resp[i + 4]]);
                    let pedido_cents =
                        i32::from_le_bytes([resp[i + 5], resp[i + 6], resp[i + 7], resp[i + 8]]);
                    out.events.push(ffi::CashEvent {
                        device_id: device_id.into(),
                        device_type,
                        kind: ffi::EventKind::IncompletePayout,
                        value_cents: pagado_cents,
                        value_pesos: pagado_cents / 100,
                        message: format!(
                            "Pago incompleto: se entregaron {} de {} solicitados",
                            format_currency(pagado_cents, "MXN"),
                            format_currency(pedido_cents, "MXN")
                        ),
                    });
                    i += 9;
                } else {
                    break;
                }
            }

            SspPollEvent::IncompleteFloat => {
                if i + 12 < resp.len()
                    && resp[i + 1] > 0
                    && is_ascii_country_code(&resp[i + 10..i + 13])
                {
                    let num_divisas = resp[i + 1] as usize;
                    let total_len = 2 + (11 * num_divisas);
                    if i + total_len <= resp.len() {
                        i += total_len;
                    } else {
                        break;
                    }
                } else if i + 8 < resp.len() {
                    i += 9;
                } else {
                    break;
                }
            }

            SspPollEvent::FraudAttempt => {
                out.events.push(ffi::CashEvent {
                    device_id: device_id.into(),
                    device_type,
                    kind: ffi::EventKind::FraudAttempt,
                    value_cents: 0,
                    value_pesos: 0,
                    message: "Intento de fraude detectado".into(),
                });
                if i + 8 < resp.len()
                    && resp[i + 1] > 0
                    && is_ascii_country_code(&resp[i + 6..i + 9])
                {
                    let num_divisas = resp[i + 1] as usize;
                    let total_len = 2 + (7 * num_divisas);
                    if i + total_len <= resp.len() {
                        i += total_len;
                    } else {
                        break;
                    }
                } else if i + 1 < resp.len() {
                    i += 2;
                } else {
                    break;
                }
            }

            SspPollEvent::Jammed => {
                // En v6 SMART Hopper / SMART Payout, 0xD5 puede incluir monto dispensado antes del atasco
                if i + 8 < resp.len()
                    && resp[i + 1] > 0
                    && is_ascii_country_code(&resp[i + 6..i + 9])
                {
                    let num_divisas = resp[i + 1] as usize;
                    let total_len = 2 + (7 * num_divisas);
                    if i + total_len <= resp.len() {
                        let cents = i32::from_le_bytes([
                            resp[i + 2],
                            resp[i + 3],
                            resp[i + 4],
                            resp[i + 5],
                        ]);
                        out.events.push(ffi::CashEvent {
                            device_id: device_id.into(),
                            device_type,
                            kind: ffi::EventKind::Jammed,
                            value_cents: cents,
                            value_pesos: cents / 100,
                            message: "Atasco físico en dispensador".into(),
                        });
                        i += total_len;
                    } else {
                        break;
                    }
                } else {
                    out.events.push(ffi::CashEvent {
                        device_id: device_id.into(),
                        device_type,
                        kind: ffi::EventKind::Jammed,
                        value_cents: 0,
                        value_pesos: 0,
                        message: "Atasco físico detectado".into(),
                    });
                    i += 1;
                }
            }

            SspPollEvent::UnsafeNoteJam | SspPollEvent::SafeNoteJam => {
                out.events.push(ffi::CashEvent {
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
                out.events.push(ffi::CashEvent {
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
                out.events.push(ffi::CashEvent {
                    device_id: device_id.into(),
                    device_type,
                    kind: ffi::EventKind::CashboxReplaced,
                    value_cents: 0,
                    value_pesos: 0,
                    message: "CASHBOX_REPLACED".into(),
                });
                i += 1;
            }

            // Eventos conocidos estrictamente de 1 byte
            SspPollEvent::SlaveReset
            | SspPollEvent::Disabled
            | SspPollEvent::NoteRejecting
            | SspPollEvent::NoteRejected
            | SspPollEvent::NoteStacking
            | SspPollEvent::NoteStacked
            | SspPollEvent::StackerFull
            | SspPollEvent::NotePathOpen
            | SspPollEvent::NoteStoredInPayout
            | SspPollEvent::Emptying
            | SspPollEvent::Emptied
            | SspPollEvent::ChannelDisable => {
                i += 1;
            }

            // Evento desconocido: detener parseo del resto de la trama para evitar desalineación
            SspPollEvent::Unknown(_) => {
                break;
            }
        }
    }

    Ok(out)
}

/// Calcula las piezas a dispensar en **centavos** usando búsqueda exacta con poda:
/// 1) Minimiza el remanente sin pagar (idealmente 0).
/// 2) A igual remanente, minimiza la cantidad total de piezas físicas.
///
/// `forzar_fraccionamiento`: si es `true` (p. ej. cambiadora de billetes), impide devolver una
/// sola pieza igual al monto objetivo. Para cambio normal de una compra, pásalo en `false`.
pub fn calcular_cambio_nativo(
    cambio_restante_centavos: &mut u32,
    niveles: &[ffi::DenomLevel],
    forzar_fraccionamiento: bool,
) -> Vec<ffi::PayoutItem> {
    let objetivo = *cambio_restante_centavos;
    if objetivo == 0 {
        return Vec::new();
    }

    let mut denoms: Vec<(u32, u32)> = niveles
        .iter()
        .filter(|d| d.value_cents > 0 && d.stored_in_payout > 0)
        .filter(|d| !forzar_fraccionamiento || (d.value_cents as u32) < objetivo)
        .map(|d| (d.value_cents as u32, d.stored_in_payout as u32))
        .collect();

    denoms.sort_unstable_by(|a, b| b.0.cmp(&a.0));

    let mut mejor_remanente = objetivo;
    let mut mejor_piezas = u32::MAX;
    let mut mejor_sol = vec![0u32; denoms.len()];
    let mut actual = vec![0u32; denoms.len()];

    let mut max_desde = vec![0u64; denoms.len() + 1];
    for idx in (0..denoms.len()).rev() {
        max_desde[idx] = max_desde[idx + 1] + (denoms[idx].0 as u64) * (denoms[idx].1 as u64);
    }

    #[allow(clippy::too_many_arguments)]
    fn dfs(
        idx: usize,
        restante: u32,
        piezas_usadas: u32,
        denoms: &[(u32, u32)],
        max_desde: &[u64],
        actual: &mut [u32],
        mejor_remanente: &mut u32,
        mejor_piezas: &mut u32,
        mejor_sol: &mut [u32],
    ) {
        if restante < *mejor_remanente
            || (restante == *mejor_remanente && piezas_usadas < *mejor_piezas)
        {
            *mejor_remanente = restante;
            *mejor_piezas = piezas_usadas;
            mejor_sol.copy_from_slice(actual);
        }
        if idx == denoms.len() || restante == 0 {
            return;
        }
        if (restante as u64).saturating_sub(max_desde[idx]) > (*mejor_remanente as u64) {
            return;
        }

        let (valor, stock) = denoms[idx];
        let max_usar = (restante / valor).min(stock);

        for usar in (0..=max_usar).rev() {
            let nuevas_piezas = piezas_usadas + usar;
            if *mejor_remanente == 0 && nuevas_piezas >= *mejor_piezas {
                continue;
            }
            actual[idx] = usar;
            dfs(
                idx + 1,
                restante - usar * valor,
                nuevas_piezas,
                denoms,
                max_desde,
                actual,
                mejor_remanente,
                mejor_piezas,
                mejor_sol,
            );
        }
        actual[idx] = 0;
    }

    dfs(
        0,
        objetivo,
        0,
        &denoms,
        &max_desde,
        &mut actual,
        &mut mejor_remanente,
        &mut mejor_piezas,
        &mut mejor_sol,
    );

    *cambio_restante_centavos = mejor_remanente;
    denoms
        .iter()
        .zip(mejor_sol.iter())
        .filter(|&(_, &cnt)| cnt > 0)
        .map(|(&(val, _), &cnt)| ffi::PayoutItem {
            value_cents: val as i32,
            count: cnt as i32,
        })
        .collect()
}

// ==================== 4. SUITE DE PRUEBAS UNITARIAS ====================
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
    fn test_poll_credito_billete_y_moneda_y_eventos_concatenados() {
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

        // Trama concatenada: Floating (9B) + CoinCredit (8B) + ValueAdded (9B)
        let mut trama_mix = vec![0xF0, 0xD7, 0x01];
        trama_mix.extend_from_slice(&500i32.to_le_bytes());
        trama_mix.extend_from_slice(b"MXN");

        trama_mix.push(0xDF);
        trama_mix.extend_from_slice(&1000i32.to_le_bytes());
        trama_mix.extend_from_slice(b"MXN");

        trama_mix.extend_from_slice(&[0xBF, 0x01]);
        trama_mix.extend_from_slice(&500i32.to_le_bytes());
        trama_mix.extend_from_slice(b"MXN");

        let ev_monedas =
            parse_poll_response(&trama_mix, "COIN-01", ffi::DeviceType::Coin, &[]).unwrap();
        assert_eq!(ev_monedas.len(), 2);
        assert_eq!(ev_monedas[0].value_cents, 1000);
        assert_eq!(ev_monedas[1].value_cents, 500);
    }

    #[test]
    fn test_construccion_y_validacion_comandos_ruta_y_payout() {
        let cmd_reciclador = build_set_route_cmd(2000, "MXN", true).unwrap();
        assert_eq!(
            cmd_reciclador,
            vec![0x3B, 0x00, 0xD0, 0x07, 0x00, 0x00, b'M', b'X', b'N']
        );

        let cmd_cashbox = build_set_route_cmd(50000, "MXN", false).unwrap();
        assert_eq!(cmd_cashbox[0], 0x3B);
        assert_eq!(cmd_cashbox[1], 0x01);

        // Debe rechazar conteo negativo y divisa inválida
        let piezas_invalidas = vec![ffi::PayoutItem {
            value_cents: 1000,
            count: -1,
        }];
        assert!(build_payout_by_denom_cmd(&piezas_invalidas, "MXN").is_err());
        assert!(build_set_route_cmd(1000, "MX", true).is_err());

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
        let cmd_pago = build_payout_by_denom_cmd(&piezas, "MXN").unwrap();
        assert_eq!(cmd_pago[0], 0x46);
        assert_eq!(cmd_pago[1], 2);
        assert_eq!(*cmd_pago.last().unwrap(), 0x58);
    }

    #[test]
    fn test_parse_all_levels_rechaza_trama_truncada() {
        // Declara 2 denominaciones pero solo incluye bytes para 1
        let mut resp = vec![0xF0, 0x02];
        resp.extend_from_slice(&5u16.to_le_bytes());
        resp.extend_from_slice(&2000i32.to_le_bytes());
        resp.extend_from_slice(b"MXN");

        assert!(parse_all_levels(&resp).is_err());
    }

    #[test]
    fn test_calculo_cambio_exacto_supera_algoritmo_voraz() {
        // Caso donde el greedy falla: objetivo $60 (6000c) con 1x$50 y 3x$20 -> óptimo es 3x$20
        let niveles = vec![
            ffi::DenomLevel {
                stored_in_cashbox: 0,
                value_cents: 5000,
                value_pesos: 50,
                country_code: "MXN".into(),
                stored_in_payout: 1,
                route_to_payout: true,
            },
            ffi::DenomLevel {
                stored_in_cashbox: 0,
                value_cents: 2000,
                value_pesos: 20,
                country_code: "MXN".into(),
                stored_in_payout: 3,
                route_to_payout: true,
            },
        ];

        let mut remanente_centavos = 6000;
        let piezas = calcular_cambio_nativo(&mut remanente_centavos, &niveles, false);

        assert_eq!(remanente_centavos, 0);
        assert_eq!(piezas.len(), 1);
        assert_eq!(piezas[0].value_cents, 2000);
        assert_eq!(piezas[0].count, 3);
    }

    #[test]
    fn test_calculo_cambio_con_moneda_50_centavos_y_remanente() {
        let niveles = vec![
            ffi::DenomLevel {
                stored_in_cashbox: 0,
                value_cents: 50,
                value_pesos: 0,
                country_code: "MXN".into(),
                stored_in_payout: 2,
                route_to_payout: true,
            },
            ffi::DenomLevel {
                stored_in_cashbox: 0,
                value_cents: 5000,
                value_pesos: 50,
                country_code: "MXN".into(),
                stored_in_payout: 1,
                route_to_payout: true,
            },
        ];

        let mut remanente_centavos = 5150;
        let piezas = calcular_cambio_nativo(&mut remanente_centavos, &niveles, false);

        assert_eq!(remanente_centavos, 50);
        assert_eq!(piezas.len(), 2);
    }

    #[test]
    #[ignore]
    fn test_hardware_real_usb() {
        use std::fs;

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
                let tipo_str = if ssp == 16 {
                    "MONEDERO (SSP 16)"
                } else {
                    "BILLETERO (SSP 0)"
                };
                println!("   [OK] Detectado {} en {}", tipo_str, path);

                let rutas_prueba = if ssp == 0 {
                    vec![
                        ffi::RouteRule {
                            value_cents: 2000,
                            to_payout: true,
                        },
                        ffi::RouteRule {
                            value_cents: 5000,
                            to_payout: true,
                        },
                        ffi::RouteRule {
                            value_cents: 10000,
                            to_payout: true,
                        },
                        ffi::RouteRule {
                            value_cents: 20000,
                            to_payout: true,
                        },
                        ffi::RouteRule {
                            value_cents: 50000,
                            to_payout: false,
                        },
                        ffi::RouteRule {
                            value_cents: 100000,
                            to_payout: false,
                        },
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
                                if lvl.route_to_payout {
                                    "Reciclador"
                                } else {
                                    "Cashbox"
                                }
                            );
                        }

                        if ssp == 16 {
                            println!(
                                "\n   [1/3] Escuchando por 10 segundos (inserta monedas ahora)..."
                            );
                            for _ in 0..100 {
                                if let Ok(eventos) = unidad.hacer_poll() {
                                    for ev in eventos {
                                        println!(
                                            "        >>> [EVENTO] {:?} | ${} MXN ({})",
                                            ev.kind, ev.value_pesos, ev.message
                                        );
                                    }
                                }
                                std::thread::sleep(Duration::from_millis(100));
                            }

                            let niveles_actuales =
                                unidad.obtener_niveles().expect("Error leyendo niveles");
                            let ini_map: HashMap<i32, i32> = niveles_iniciales
                                .iter()
                                .map(|l| (l.value_cents, l.stored_in_payout))
                                .collect();
                            let mut piezas_a_devolver = Vec::new();

                            println!("   [OK] Niveles tras inserción:");
                            for act in &niveles_actuales {
                                let antes = ini_map.get(&act.value_cents).copied().unwrap_or(0);
                                let diferencia = act.stored_in_payout - antes;
                                println!(
                                    "        - ${:>4} {} | Antes: {:>3} -> Ahora: {:>3} (Entraron: +{})",
                                    act.value_pesos, act.country_code, antes, act.stored_in_payout, diferencia
                                );
                                if diferencia > 0 {
                                    piezas_a_devolver.push(ffi::PayoutItem {
                                        value_cents: act.value_cents,
                                        count: diferencia,
                                    });
                                }
                            }

                            if piezas_a_devolver.is_empty() {
                                println!("   [INFO] Sin monedas nuevas; devolviendo 5x$1 MXN ($5.00 MXN)...");
                                piezas_a_devolver.push(ffi::PayoutItem {
                                    value_cents: 100,
                                    count: 5,
                                });
                            }

                            println!("\n   [2/3] Dispensando monedas: {:?}...", piezas_a_devolver);
                            unidad
                                .dispensar_piezas(&piezas_a_devolver)
                                .expect("Fallo al enviar comando de dispensado (0x46)");

                            println!("   [3/3] Esperando entrega física (máx. 30s)...");
                            let mut entregado = false;
                            let mut error_pago: Option<String> = None;

                            for _ in 0..300 {
                                if let Ok(eventos) = unidad.hacer_poll() {
                                    for ev in eventos {
                                        println!(
                                            "        >>> [PAGO] {:?} | ${} MXN {}",
                                            ev.kind, ev.value_pesos, ev.message
                                        );
                                        match ev.kind {
                                            ffi::EventKind::Dispensed => entregado = true,
                                            ffi::EventKind::Timeout
                                            | ffi::EventKind::IncompletePayout
                                            | ffi::EventKind::Jammed
                                            | ffi::EventKind::Error => {
                                                error_pago =
                                                    Some(format!("{:?}: {}", ev.kind, ev.message));
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

                            if let Some(motivo) = error_pago {
                                println!(
                                    "   [ERROR DE PAGO] El hardware reportó fallo: {}",
                                    motivo
                                );
                            } else if !entregado {
                                println!("   [ERROR TIMEOUT] Tiempo agotado (30s). Abortando motor (0x38)...");
                                let _ = unidad.detener_pago();
                            } else {
                                println!("   [OK] Pago entregado exitosamente.");
                            }

                            if let Ok(niveles_finales) = unidad.obtener_niveles() {
                                println!(
                                    "   [OK] Niveles finales después de devolver el efectivo:"
                                );
                                for lvl in &niveles_finales {
                                    println!(
                                        "        - ${:>4} {} | En Reciclador: {:>3}",
                                        lvl.value_pesos, lvl.country_code, lvl.stored_in_payout
                                    );
                                }
                            }
                        }

                        unidad.detener_y_desconectar();
                        println!(
                            "   [OK] Dispositivo en {} desconectado limpiamente.\n",
                            path
                        );
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
