use std::time::{Duration, Instant};
use crate::transport::SerialTransport;

pub const SSP_STX: u8 = 0x7F;
pub const SSP_STEX: u8 = 0x7E;
pub const CRC_SSP_SEED: u16 = 0xFFFF;
pub const CRC_SSP_POLY: u16 = 0x8005;


/// Cálculo CRC16 estándar de eSSP (semilla 0xFFFF, polinomio 0x8005)
pub fn calc_ssp_crc(data: &[u8]) -> u16 {
    let mut crc: u16 = CRC_SSP_SEED;
    for &b in data {
        crc ^= (b as u16) << 8;
        for _ in 0..8 {
            if (crc & 0x8000) != 0 {
                crc = (crc << 1) ^ CRC_SSP_POLY;
            } else {
                crc <<= 1;
            }
        }
    }
    crc
}

pub struct SspWire {
    pub address: u8,
    pub seq_bit: u8, // 0x80 o 0x00
}

impl SspWire {
    pub fn new(address: u8) -> Self {
        Self {
            address,
            seq_bit: 0x80,
        }
    }

    pub fn reset_seq(&mut self) {
        self.seq_bit = 0x80;
    }

    pub fn toggle_seq(&mut self) {
        self.seq_bit = if self.seq_bit == 0x80 { 0x00 } else { 0x80 };
    }

    /// Arma una trama eSSP lista para enviar: [0x7F][ADDR|SEQ][LEN][DATA...][CRC_L][CRC_H] con byte-stuffing
    pub fn build_frame(&mut self, payload: &[u8]) -> Result<Vec<u8>, String> {
        if payload.is_empty() || payload.len() > 250 {
            return Err("Longitud de payload eSSP inválida".into());
        }
        if payload[0] == 0x11 {
            // SSP_CMD_SYNC reinicia el bit de secuencia
            self.reset_seq();
        }

        let addr_seq = self.address | self.seq_bit;
        let len = payload.len() as u8;

        let mut raw = Vec::with_capacity(2 + payload.len() + 2);
        raw.push(addr_seq);
        raw.push(len);
        raw.extend_from_slice(payload);

        let crc = calc_ssp_crc(&raw);
        raw.push((crc & 0xFF) as u8);
        raw.push(((crc >> 8) & 0xFF) as u8);

        let mut framed = Vec::with_capacity(1 + raw.len() * 2);
        framed.push(SSP_STX);
        for &b in &raw {
            framed.push(b);
            if b == SSP_STX {
                framed.push(SSP_STX); // Byte stuffing de 0x7F
            }
        }
        Ok(framed)
    }

    /// Envía un paquete y espera una respuesta válida con reintentos y timeout
    pub fn transact<T: SerialTransport + ?Sized>(
        &mut self,
        port: &mut T,
        payload: &[u8],
        timeout: Duration,
        retries: u8,
    ) -> Result<Vec<u8>, String> {
        let frame = self.build_frame(payload)?;
        let attempts = retries.max(1);
        let mut last_err = String::new();

        for _ in 0..attempts {
            port.clear_rx();
            if let Err(e) = port.write_all(&frame) {
                last_err = format!("Error escribiendo al puerto: {}", e);
                continue;
            }

            match self.read_frame(port, timeout) {
                Ok(resp_payload) => {
                    self.toggle_seq();
                    return Ok(resp_payload);
                }
                Err(e) => {
                    last_err = e;
                }
            }
        }

        Err(last_err)
    }

    /// Lee una trama completa desde el transporte manejando STX, byte-unstuffing y CRC16
    fn read_frame<T: SerialTransport + ?Sized>(
        &self,
        port: &mut T,
        timeout: Duration,
    ) -> Result<Vec<u8>, String> {
        let deadline = Instant::now() + timeout;
        let mut tmp = [0u8; 64];

        let mut in_frame = false;
        let mut stuff_next = false;
        let mut unstuffed: Vec<u8> = Vec::with_capacity(64);
        let mut expected_total: Option<usize> = None; // addr_seq + len + data(len) + crc(2)

        while Instant::now() < deadline {
            let rem = deadline.saturating_duration_since(Instant::now());
            let n = port.read_some(&mut tmp, rem.min(Duration::from_millis(50)))?;
            if n == 0 {
                continue;
            }

            for &b in &tmp[..n] {
                if !in_frame {
                    if b == SSP_STX {
                        in_frame = true;
                        stuff_next = false;
                        unstuffed.clear();
                        expected_total = None;
                    }
                    continue;
                }

                if stuff_next {
                    stuff_next = false;
                    if b == SSP_STX {
                        unstuffed.push(SSP_STX);
                    } else {
                        // Un byte no-STX tras STX indica reinicio de trama
                        unstuffed.clear();
                        unstuffed.push(b);
                        expected_total = None;
                    }
                } else if b == SSP_STX {
                    stuff_next = true;
                } else {
                    unstuffed.push(b);
                }

                if unstuffed.len() == 2 && expected_total.is_none() {
                    let data_len = unstuffed[1] as usize;
                    expected_total = Some(2 + data_len + 2);
                }

                if let Some(total) = expected_total {
                    if unstuffed.len() == total {
                        let crc_calc = calc_ssp_crc(&unstuffed[..total - 2]);
                        let crc_rx = u16::from_le_bytes([unstuffed[total - 2], unstuffed[total - 1]]);

                        in_frame = false;
                        expected_total = None;

                        if (unstuffed[0] & 0x7F) == self.address && crc_calc == crc_rx {
                            let data_len = unstuffed[1] as usize;
                            return Ok(unstuffed[2..2 + data_len].to_vec());
                        } else {
                            unstuffed.clear();
                        }
                    }
                }
            }
        }

        Err("Timeout esperando respuesta eSSP".into())
    }
}