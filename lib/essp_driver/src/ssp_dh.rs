use std::time::Duration;

use crate::ssp_wire::SspWire;
use crate::transport::SerialTransport;

pub const MIN_PRIME_NUMBER: u64 = 0x0100_0000; // 16,777,216 (evita módulos débiles)
pub const MAX_PRIME_NUMBER: u64 = 0x7FFF_FFFF; // 2^31 - 1 (límite protocolo ITL eSSP)
pub const MAX_RANDOM_INTEGER: u64 = 0x7FFF_FFFF;

pub fn pow_mod(mut base: u64, mut exp: u64, modulus: u64) -> u64 {
    if modulus <= 1 {
        return 0;
    }
    let m = modulus as u128;
    let mut result: u128 = 1;
    base %= modulus;
    let mut b = base as u128;

    while exp > 0 {
        if (exp & 1) == 1 {
            result = (result * b) % m;
        }
        b = (b * b) % m;
        exp >>= 1;
    }
    result as u64
}

fn random_u64() -> Result<u64, String> {
    let mut buf = [0u8; 8];
    getrandom::fill(&mut buf)
        .map_err(|e| format!("Error obteniendo entropía CSPRNG para DH: {}", e))?;
    Ok(u64::from_le_bytes(buf))
}

/// Genera un entero uniforme en `[min, max]` sin sesgo de módulo (rejection sampling).
fn random_range_inclusive(min: u64, max: u64) -> Result<u64, String> {
    if min >= max {
        return Ok(min);
    }
    let span = max - min + 1;
    let limit = u64::MAX - (u64::MAX % span);
    loop {
        let r = random_u64()?;
        if r < limit {
            return Ok(min + (r % span));
        }
    }
}

/// Miller-Rabin determinista para cualquier entero `n < 4,759,123,141` (cubre todo `u32`).
/// Cero falsos positivos y resistente a números de Carmichael.
pub fn is_prime_u32(n: u64) -> bool {
    if n < 2 {
        return false;
    }
    const SMALL_PRIMES: [u64; 12] = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37];
    for &p in &SMALL_PRIMES {
        if n == p {
            return true;
        }
        if n % p == 0 {
            return false;
        }
    }

    let mut d = n - 1;
    let mut s = 0u32;
    while (d & 1) == 0 {
        d >>= 1;
        s += 1;
    }

    'witness: for &a in &[2u64, 7, 61] {
        if a >= n {
            continue;
        }
        let mut x = pow_mod(a, d, n);
        if x == 1 || x == n - 1 {
            continue;
        }
        for _ in 1..s {
            x = pow_mod(x, 2, n);
            if x == n - 1 {
                continue 'witness;
            }
        }
        return false;
    }

    true
}

fn generate_prime() -> Result<u64, String> {
    let mut candidate = random_range_inclusive(MIN_PRIME_NUMBER, MAX_PRIME_NUMBER)? | 1;

    while !is_prime_u32(candidate) {
        candidate = candidate.saturating_add(2);
        if candidate > MAX_PRIME_NUMBER {
            candidate = MIN_PRIME_NUMBER | 1;
        }
    }
    Ok(candidate)
}

/// Negocia la llave de sesión Diffie-Hellman eSSP (`0x4A`, `0x4B`, `0x4C`).
/// Precondición: el llamador ya ejecutó `SYNC (0x11)` y obtuvo `0xF0`.
pub fn negotiate_ssp_encryption<T: SerialTransport + ?Sized>(
    wire: &mut SspWire,
    port: &mut T,
    timeout: Duration,
) -> Result<(u64, u32), String> {
    let mut generator = generate_prime()?;
    let mut modulus = generate_prime()?;
    while modulus == generator {
        modulus = generate_prime()?;
    }
    if generator > modulus {
        std::mem::swap(&mut generator, &mut modulus);
    }

    let max_exp = (modulus - 2).min(MAX_RANDOM_INTEGER - 1);
    let (host_random, host_inter) = loop {
        let r = random_range_inclusive(2, max_exp)?;
        let inter = pow_mod(generator, r, modulus);
        if inter > 1 {
            break (r, inter);
        }
    };

    let mut cmd_gen = [0u8; 9];
    cmd_gen[0] = 0x4A;
    cmd_gen[1..9].copy_from_slice(&generator.to_le_bytes());
    let resp_gen = wire.transact(port, &cmd_gen, timeout, 2)?;
    if resp_gen.first().copied() != Some(0xF0) {
        return Err(format!(
            "Validador rechazó SET_GENERATOR (0x4A): {:02X?}",
            resp_gen
        ));
    }

    let mut cmd_mod = [0u8; 9];
    cmd_mod[0] = 0x4B;
    cmd_mod[1..9].copy_from_slice(&modulus.to_le_bytes());
    let resp_mod = wire.transact(port, &cmd_mod, timeout, 2)?;
    if resp_mod.first().copied() != Some(0xF0) {
        return Err(format!(
            "Validador rechazó SET_MODULUS (0x4B): {:02X?}",
            resp_mod
        ));
    }

    let mut cmd_ex = [0u8; 9];
    cmd_ex[0] = 0x4C;
    cmd_ex[1..9].copy_from_slice(&host_inter.to_le_bytes());
    let resp_ex = wire.transact(port, &cmd_ex, timeout, 2)?;
    if resp_ex.first().copied() != Some(0xF0) || resp_ex.len() != 9 {
        return Err(format!(
            "Respuesta inválida en REQ_KEY_EXCHANGE (0x4C): {:02X?}",
            resp_ex
        ));
    }

    let slave_inter = u64::from_le_bytes([
        resp_ex[1], resp_ex[2], resp_ex[3], resp_ex[4], resp_ex[5], resp_ex[6], resp_ex[7],
        resp_ex[8],
    ]);
    if slave_inter <= 1 || slave_inter >= modulus {
        return Err(format!(
            "Clave intermedia inválida recibida del validador (slave_inter={})",
            slave_inter
        ));
    }

    let encrypt_key = pow_mod(slave_inter, host_random, modulus);
    Ok((encrypt_key, 0))
}