# Maxi-Server-Linux

Sistema de gestión de pagos e inventario de efectivo para terminales autónomas, integrado con GTK4 para la interfaz de usuario y Crow para servicios REST.

## 1. Instalación de Dependencias

El proyecto requiere las siguientes bibliotecas principales:

### GTKmm 4.0
Interfaz gráfica de usuario.

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install libgtkmm-4.0-dev
```

### CPR (C++ Requests)
Utilizado para las peticiones HTTP a la API de Mercado Pago.
```bash
sudo apt install libcpr-dev
```

### Crow
Micro-framework web para C++. Asegúrate de tener instaladas las dependencias de Boost y ASIO si compilas desde fuente, o utiliza el header-only proporcionado en el proyecto.

### SQLite-cpp
Wrapper para la persistencia de datos.
```bash
sudo apt install libsqlite3-dev
```

## 2. Compilación

El sistema utiliza CMake como sistema de construcción.

1. Crear directorio de construcción:
   ```bash
   mkdir build && cd build
   ```
2. Configurar el proyecto:
   ```bash
   cmake ..
   ```
3. Compilar:
   ```bash
   make -j$(nproc)
   ```

## 3. Flujo Técnico: Procesamiento de Pago con Tarjeta

El flujo de pago con tarjeta sigue una arquitectura de polling síncrono para garantizar que la interfaz de usuario espere la respuesta física de la terminal bancaria (Point).

### Paso 1: Inicialización desde la UI
Cuando el usuario selecciona "Tarjeta" en el `MetodoPago`, se instancia la clase `Tarjeta` y se llama al método `iniciar(t_log)`.

### Paso 2: Creación de la Orden (API Mercado Pago)
Se genera una petición `POST` a `https://api.mercadopago.com/v1/orders`.
- **Referencia Externa:** Se genera una llave de idempotencia basada en el ID del log y el monto para evitar cobros duplicados.
- **Configuración:** Se define que no se imprima ticket en la terminal (`no_ticket`) para manejar la impresión localmente.

### Paso 3: Interfaz de Usuario
Se activa la `ViewCarga` (Cortinilla de Carga). El método `modo(false)` configura el widget para mostrar mensajes específicos de la terminal bancaria ("Terminal de Pago Activa").

### Paso 4: Persistencia Inicial
Antes de confirmar el pago, se inserta un registro en la tabla `pago_terminal` mediante `OPagoTarjeta::insertar`. Esto permite rastrear transacciones pendientes incluso si hay una falla de energía.

### Paso 5: Polling de Estado
El método `get_order_status` entra en un bucle que consulta el estado de la orden cada segundo:
- Estados finales: `processed`, `expired`, `canceled`, `failed`.
- Durante el bucle, se actualiza el estado en la base de datos SQLite para mantener la trazabilidad.

### Paso 6: Cierre de Transacción y Base de Datos
Si el estado es `processed`:
1. Se actualiza el `m_ingreso` en el objeto `MLog`.
2. Se llama a `log.update_log(t_log)` para persistir el éxito en la base de datos principal.
3. `DetalleMovimiento` registra las diferencias si hubo algún movimiento de hardware adicional.
4. Se envía una notificación visual al usuario a través de `Global::System::showNotify`.

---
*Documentación generada para el equipo de desarrollo de Maxi-Server.*
