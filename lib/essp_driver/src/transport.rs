use std::time::Duration;

pub trait SerialTransport {
    fn write_all(&mut self, data: &[u8]) -> Result<(), String>;
    fn read_some(&mut self, buf: &mut [u8], timeout: Duration) -> Result<usize, String>;
    fn clear_rx(&mut self);
}

#[cfg(target_os = "espidf")]
pub mod esp_io {
    use super::*;
    use std::ffi::{CString, c_char, c_int, c_uchar};

    unsafe extern "C" {
        fn esp_ssp_port_open(port: *const c_char) -> c_int;
        fn esp_ssp_port_close(port: c_int);
        fn esp_ssp_port_write(port: c_int, buf: *const c_uchar, len: c_int) -> c_int;
        fn esp_ssp_port_read(port: c_int, buf: *mut c_uchar, len: c_int, timeout_ms: c_int) -> c_int;
    }

    pub struct EspFtdiPort {
        handle: c_int,
    }

    impl EspFtdiPort {
        pub fn open(port_name: &str) -> Result<Self, String> {
            let c_name = CString::new(port_name).map_err(|e| e.to_string())?;
            let h = unsafe { esp_ssp_port_open(c_name.as_ptr()) };
            if h < 0 {
                return Err(format!("No se pudo abrir canal FTDI {}", port_name));
            }
            Ok(Self { handle: h })
        }
    }

    impl Drop for EspFtdiPort {
        fn drop(&mut self) {
            if self.handle >= 0 {
                unsafe { esp_ssp_port_close(self.handle) };
                self.handle = -1;
            }
        }
    }

    impl SerialTransport for EspFtdiPort {
        fn write_all(&mut self, data: &[u8]) -> Result<(), String> {
            let mut sent = 0;
            while sent < data.len() {
                let w = unsafe {
                    esp_ssp_port_write(
                        self.handle,
                        data[sent..].as_ptr(),
                        (data.len() - sent) as c_int,
                    )
                };
                if w <= 0 {
                    return Err("Error escribiendo a FTDI en ESP32-P4".into());
                }
                sent += w as usize;
            }
            Ok(())
        }

        fn read_some(&mut self, buf: &mut [u8], timeout: Duration) -> Result<usize, String> {
            let ms = timeout.as_millis().min(c_int::MAX as u128) as c_int;
            let r = unsafe {
                esp_ssp_port_read(self.handle, buf.as_mut_ptr(), buf.len() as c_int, ms.max(1))
            };
            if r < 0 {
                Err("Error leyendo de FTDI en ESP32-P4".into())
            } else {
                Ok(r as usize)
            }
        }

        fn clear_rx(&mut self) {
            let mut tmp = [0u8; 64];
            while let Ok(n) = self.read_some(&mut tmp, Duration::from_millis(5)) {
                if n == 0 {
                    break;
                }
            }
        }
    }
}

#[cfg(not(target_os = "espidf"))]
pub mod pc_io {
    use super::*;
    use std::io::{Read, Write};

    pub struct PcSerialPort {
        inner: Box<dyn serialport::SerialPort>,
    }

    impl PcSerialPort {
        pub fn open(port_name: &str) -> Result<Self, String> {
            let p = serialport::new(port_name, 9600)
                .data_bits(serialport::DataBits::Eight)
                .stop_bits(serialport::StopBits::Two)
                .parity(serialport::Parity::None)
                .timeout(Duration::from_millis(100))
                .open()
                .map_err(|e| format!("Error abriendo {}: {}", port_name, e))?;
            Ok(Self { inner: p })
        }
    }

    impl SerialTransport for PcSerialPort {
        fn write_all(&mut self, data: &[u8]) -> Result<(), String> {
            self.inner.write_all(data).map_err(|e| e.to_string())
        }

        fn read_some(&mut self, buf: &mut [u8], timeout: Duration) -> Result<usize, String> {
            let _ = self.inner.set_timeout(timeout);
            match self.inner.read(buf) {
                Ok(n) => Ok(n),
                Err(ref e) if e.kind() == std::io::ErrorKind::TimedOut => Ok(0),
                Err(e) => Err(e.to_string()),
            }
        }

        fn clear_rx(&mut self) {
            let _ = self.inner.clear(serialport::ClearBuffer::Input);
        }
    }
}