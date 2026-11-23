use aes::cipher::{BlockCipherDecrypt, BlockCipherEncrypt, KeyInit};
use aes::Aes128;

use crate::ssp_wire::{calc_ssp_crc, SSP_STEX};

/// Límite máximo de payload claro cifrable dentro de una trama SSP (<= 250 bytes totales):
/// 1 (STEX) + 240 (15 bloques AES de 16B) = 241 bytes; 240 - 1 (LEN) - 4 (COUNT) - 2 (CRC) = 233.
pub const MAX_CLEAR_PAYLOAD_LEN: usize = 233;

pub fn build_aes128_key(fixed_key: u64, encrypt_key: u64) -> [u8; 16] {
    let mut key = [0u8; 16];
    key[0..8].copy_from_slice(&fixed_key.to_le_bytes());
    key[8..16].copy_from_slice(&encrypt_key.to_le_bytes());
    key
}

/// Cifrador de sesión eSSP con key-schedule AES-128 cacheado.
pub struct SspCipher {
    cipher: Aes128,
}

impl SspCipher {
    pub fn new(fixed_key: u64, encrypt_key: u64) -> Self {
        let key = build_aes128_key(fixed_key, encrypt_key);
        Self::from_key(&key)
    }

    pub fn from_key(key: &[u8; 16]) -> Self {
        Self {
            cipher: Aes128::new(key.into()),
        }
    }

    fn encrypt_blocks_in_place(&self, buf: &mut [u8]) -> Result<(), String> {
        if buf.len() % 16 != 0 {
            return Err("La longitud a cifrar en AES-ECB debe ser múltiplo de 16 bytes".into());
        }
        for chunk in buf.chunks_exact_mut(16) {
            let block: &mut [u8; 16] = chunk.try_into().unwrap();
            self.cipher.encrypt_block(block.into());
        }
        Ok(())
    }

    fn decrypt_blocks_in_place(&self, buf: &mut [u8]) -> Result<(), String> {
        if buf.len() % 16 != 0 {
            return Err("La longitud a descifrar en AES-ECB debe ser múltiplo de 16 bytes".into());
        }
        for chunk in buf.chunks_exact_mut(16) {
            let block: &mut [u8; 16] = chunk.try_into().unwrap();
            self.cipher.decrypt_block(block.into());
        }
        Ok(())
    }

    /// Empaqueta payload claro (cmd + args) en formato cifrado eSSP:
    /// `[0x7E] + AES128_ECB([LEN_CLARO(1)][COUNT_LE(4)][PAYLOAD...][ePACKING_RND...][CRC_LE(2)])`
    pub fn encrypt_payload(
        &self,
        pkt_count: u32,
        clear_payload: &[u8],
    ) -> Result<Vec<u8>, String> {
        let clear_len = clear_payload.len();
        if clear_len == 0 || clear_len > MAX_CLEAR_PAYLOAD_LEN {
            return Err(format!(
                "Longitud de payload claro inválida para eSSP: {} bytes (rango 1..={})",
                clear_len, MAX_CLEAR_PAYLOAD_LEN
            ));
        }

        // 1 byte LEN + 4 bytes COUNT + clear_len + 2 bytes CRC
        let inner_len = 1 + 4 + clear_len + 2;
        let padded_len = inner_len.div_ceil(16) * 16;

        // Una sola asignación para STEX + bloques cifrados in-place
        let mut out = vec![0u8; 1 + padded_len];
        out[0] = SSP_STEX;

        let plain = &mut out[1..];
        plain[0] = clear_len as u8;
        plain[1..5].copy_from_slice(&pkt_count.to_le_bytes());
        plain[5..5 + clear_len].copy_from_slice(clear_payload);

        let crc_end = padded_len - 2;
        let padding_slice = &mut plain[5 + clear_len..crc_end];
        if !padding_slice.is_empty() {
            getrandom::fill(padding_slice)
                .map_err(|e| format!("Error generando padding aleatorio eSSP: {}", e))?;
        }

        let crc = calc_ssp_crc(&plain[0..crc_end]);
        plain[crc_end] = (crc & 0xFF) as u8;
        plain[crc_end + 1] = ((crc >> 8) & 0xFF) as u8;

        self.encrypt_blocks_in_place(plain)?;
        Ok(out)
    }

    /// Descifra y valida un payload recibido que comienza con `0x7E` (`SSP_STEX`).
    /// Valida CRC primero, luego longitud interna y por último `eCount`.
    pub fn decrypt_payload(
        &self,
        expected_count: u32,
        enc_payload: &[u8],
    ) -> Result<Vec<u8>, String> {
        if enc_payload.len() < 17 || enc_payload[0] != SSP_STEX {
            return Err("Trama cifrada inválida o sin cabecera STEX (0x7E)".into());
        }

        let body = &enc_payload[1..];
        if body.len() % 16 != 0 {
            return Err("Bloque cifrado no es múltiplo de 16 bytes".into());
        }

        let mut plain = body.to_vec();
        self.decrypt_blocks_in_place(&mut plain)?;

        // 1. Verificar integridad (CRC) primero: detecta llave incorrecta o corrupción real
        let crc_end = plain.len() - 2;
        let crc_calc = calc_ssp_crc(&plain[0..crc_end]);
        let crc_rx = u16::from_le_bytes([plain[crc_end], plain[crc_end + 1]]);
        if crc_calc != crc_rx {
            return Err(
                "Error de CRC dentro de paquete cifrado eSSP (llave AES inválida o trama corrupta)"
                    .into(),
            );
        }

        // 2. Validar longitud declarada del payload claro
        let clear_len = plain[0] as usize;
        if 5 + clear_len + 2 > plain.len() {
            return Err(format!(
                "Longitud interna descifrada fuera de rango ({} bytes en bloque de {})",
                clear_len,
                plain.len()
            ));
        }

        // 3. Verificar contador eCount
        let rx_count = u32::from_le_bytes([plain[1], plain[2], plain[3], plain[4]]);
        if rx_count != expected_count {
            return Err(format!(
                "Contador de paquete eSSP no coincide (recibido {}, esperado {})",
                rx_count, expected_count
            ));
        }

        // Reutiliza el mismo buffer sin segunda asignación
        plain.copy_within(5..5 + clear_len, 0);
        plain.truncate(clear_len);
        Ok(plain)
    }
}