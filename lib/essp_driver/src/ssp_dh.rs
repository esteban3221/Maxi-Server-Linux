use crate::ssp_wire::SspWire;
use crate::transport::SerialTransport;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

pub const MAX_RANDOM_INTEGER: u64 = 2_147_483_648;
pub const MAX_PRIME_NUMBER: u64 = 2_147_483_648;

pub fn pow_mod(mut base: u64, mut exp: u64, modulus: u64) -> u64 {
    if modulus == 0 || modulus == 1 {
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

struct Rng64 {
    state: u64,
}

impl Rng64 {
    fn new() -> Self {
        let nanos = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map(|d| d.as_nanos() as u64)
            .unwrap_or(0x4159_4C49_94C4_9514);
        let pid = std::process::id() as u64;
        let mut s = nanos ^ pid.wrapping_mul(0x9E37_79B9_7F4A_7C15);
        if s == 0 {
            s = 0x4159_4C49_94C4_9514;
        }
        Self { state: s }
    }

    fn next_u64(&mut self) -> u64 {
        // xorshift64*
        let mut x = self.state;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        self.state = x;
        x.wrapping_mul(0x2545_F491_4F6C_DD1D)
    }

    fn gen_range_u64(&mut self, upper_excl: u64) -> u64 {
        if upper_excl <= 1 {
            return 0;
        }
        self.next_u64() % upper_excl
    }
}

fn is_probably_prime(rng: &mut Rng64, n: u64, trials: usize) -> bool {
    if n < 2 {
        return false;
    }
    if n == 2 || n == 3 {
        return true;
    }
    if n % 2 == 0 {
        return false;
    }

    for _ in 0..trials {
        let a = 2 + rng.gen_range_u64((n - 3).max(1));
        if pow_mod(a, n - 1, n) != 1 {
            return false;
        }
    }
    true
}

fn generate_prime(rng: &mut Rng64) -> u64 {
    let mut candidate = rng.gen_range_u64(MAX_PRIME_NUMBER);
    if candidate < 5 {
        candidate = 101;
    }
    if (candidate & 1) == 0 {
        candidate += 1;
    }

    while !is_probably_prime(rng, candidate, 5) {
        candidate = candidate.wrapping_add(2);
        if candidate >= MAX_PRIME_NUMBER || candidate < 5 {
            candidate = 101;
        }
    }
    candidate
}

pub fn negotiate_ssp_encryption<T: SerialTransport + ?Sized>(
    wire: &mut SspWire,
    port: &mut T,
    timeout: Duration,
) -> Result<(u64, u32), String> {
    let mut rng = Rng64::new();

    let mut generator = generate_prime(&mut rng);
    let mut modulus = generate_prime(&mut rng);
    if generator > modulus {
        std::mem::swap(&mut generator, &mut modulus);
    }

    let host_random = 1 + rng.gen_range_u64(MAX_RANDOM_INTEGER - 1);
    let host_inter = pow_mod(generator, host_random, modulus);

    let resp_sync = wire.transact(port, &[0x11], timeout, 2)?;
    if resp_sync.first().copied() != Some(0xF0) {
        return Err("Fallo en SYNC durante negociación eSSP".into());
    }

    let mut cmd_gen = Vec::with_capacity(9);
    cmd_gen.push(0x4A);
    cmd_gen.extend_from_slice(&generator.to_le_bytes());
    let resp_gen = wire.transact(port, &cmd_gen, timeout, 2)?;
    if resp_gen.first().copied() != Some(0xF0) {
        return Err("Validador rechazó SET_GENERATOR (0x4A)".into());
    }

    let mut cmd_mod = Vec::with_capacity(9);
    cmd_mod.push(0x4B);
    cmd_mod.extend_from_slice(&modulus.to_le_bytes());
    let resp_mod = wire.transact(port, &cmd_mod, timeout, 2)?;
    if resp_mod.first().copied() != Some(0xF0) {
        return Err("Validador rechazó SET_MODULUS (0x4B)".into());
    }

    let mut cmd_ex = Vec::with_capacity(9);
    cmd_ex.push(0x4C);
    cmd_ex.extend_from_slice(&host_inter.to_le_bytes());
    let resp_ex = wire.transact(port, &cmd_ex, timeout, 2)?;
    if resp_ex.first().copied() != Some(0xF0) || resp_ex.len() < 9 {
        return Err("Respuesta inválida en REQ_KEY_EXCHANGE (0x4C)".into());
    }

    let slave_inter = u64::from_le_bytes([
        resp_ex[1], resp_ex[2], resp_ex[3], resp_ex[4], resp_ex[5], resp_ex[6], resp_ex[7],
        resp_ex[8],
    ]);

    let encrypt_key = pow_mod(slave_inter, host_random, modulus);

    println!(
        "DH -> g={}, m={}, host_inter={}, slave_inter={}, encrypt_key={:#018x}",
        generator, modulus, host_inter, slave_inter, encrypt_key
    );
    Ok((encrypt_key, 0))
}
