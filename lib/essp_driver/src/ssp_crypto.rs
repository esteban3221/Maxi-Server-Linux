use aes::Aes128;
use aes::cipher::{BlockCipherDecrypt, BlockCipherEncrypt, KeyInit};
use crate::ssp_wire::{SSP_STEX, calc_ssp_crc};

pub fn build_aes128_key(fixed_key: u64, encrypt_key: u64) -> [u8; 16] {
    let mut key = [0u8; 16];
    key[0..8].copy_from_slice(&fixed_key.to_le_bytes());
    key[8..16].copy_from_slice(&encrypt_key.to_le_bytes());
    key
}

pub fn aes128_ecb_encrypt(key: &[u8; 16], plain: &[u8]) -> Result<Vec<u8>, String> {
    if plain.len() % 16 != 0 {
        return Err("La longitud a cifrar en AES-ECB debe ser múltiplo de 16 bytes".into());
    }
    let cipher = Aes128::new(key.into());
    let mut out = plain.to_vec();
    for chunk in out.chunks_exact_mut(16) {
        let block: &mut [u8; 16] = chunk.try_into().unwrap();
        cipher.encrypt_block(block.into());
    }
    Ok(out)
}

pub fn aes128_ecb_decrypt(key: &[u8; 16], encrypted: &[u8]) -> Result<Vec<u8>, String> {
    if encrypted.len() % 16 != 0 {
        return Err("La longitud a descifrar en AES-ECB debe ser múltiplo de 16 bytes".into());
    }
    let cipher = Aes128::new(key.into());
    let mut out = encrypted.to_vec();
    for chunk in out.chunks_exact_mut(16) {
        let block: &mut [u8; 16] = chunk.try_into().unwrap();
        cipher.decrypt_block(block.into());
    }
    Ok(out)
}

/// Empaqueta payload claro (cmd + args) en formato cifrado eSSP:
/// [0x7E][LEN_CLARO][COUNT_LE(4)][PAYLOAD...][CRC_LE(2)][PADDING_A_16B]
pub fn encrypt_ssp_payload(
    key: &[u8; 16],
    pkt_count: u32,
    clear_payload: &[u8],
) -> Result<Vec<u8>, String> {
    let clear_len = clear_payload.len();
    // 1 byte LEN + 4 bytes COUNT + clear_len + 2 bytes CRC
    let inner_len = 1 + 4 + clear_len + 2;
    let padded_len = inner_len.div_ceil(16) * 16;

    let mut plain = vec![0u8; padded_len];
    plain[0] = clear_len as u8;
    plain[1..5].copy_from_slice(&pkt_count.to_le_bytes());
    plain[5..5 + clear_len].copy_from_slice(clear_payload);

    let crc_end = padded_len - 2;
    let crc = calc_ssp_crc(&plain[0..crc_end]);
    plain[crc_end] = (crc & 0xFF) as u8;
    plain[crc_end + 1] = ((crc >> 8) & 0xFF) as u8;

    let cipher_bytes = aes128_ecb_encrypt(key, &plain)?;

    let mut out = Vec::with_capacity(1 + cipher_bytes.len());
    out.push(SSP_STEX);
    out.extend_from_slice(&cipher_bytes);
    Ok(out)
}

/// Descifra y valida un payload recibido que comienza con 0x7E (SSP_STEX)
pub fn decrypt_ssp_payload(
    key: &[u8; 16],
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

    let plain = aes128_ecb_decrypt(key, body)?;
    let clear_len = plain[0] as usize;
    if 5 + clear_len + 2 > plain.len() {
        return Err("Longitud interna descifrada fuera de rango".into());
    }

    let rx_count = u32::from_le_bytes([plain[1], plain[2], plain[3], plain[4]]);
    if rx_count != expected_count {
        return Err(format!(
            "Contador de paquete eSSP no coincide (recibido {}, esperado {})",
            rx_count, expected_count
        ));
    }

    let crc_end = plain.len() - 2;
    let crc_calc = calc_ssp_crc(&plain[0..crc_end]);
    let crc_rx = u16::from_le_bytes([plain[crc_end], plain[crc_end + 1]]);
    if crc_calc != crc_rx {
        return Err("Error de CRC dentro de paquete cifrado eSSP".into());
    }

    Ok(plain[5..5 + clear_len].to_vec())
}