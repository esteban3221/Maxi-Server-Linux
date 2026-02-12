#pragma once
#include "sqlite.hpp"
#include <memory>

// Singleton de base de datos
class Database
{
private:
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    ~Database(void) {}

    Database(void)
    {
        sqlite3 = std::make_unique<SQLite3::SQLite>("data.db");
        this->sqlite3->open();

        if (not this->sqlite3->is_created())
        {
            this->sqlite3->command("CREATE TABLE usuarios (\n"
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                   "username TEXT NOT NULL UNIQUE,\n"
                                   "password TEXT NOT NULL UNIQUE\n"
                                   ")");
            this->sqlite3->command("CREATE TABLE roles (\n"
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                   "rol TEXT\n"
                                   ")");
            this->sqlite3->command("CREATE TABLE usuario_roles (\n"
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                   "id_usuario INTEGER,\n"
                                   "id_rol INTEGER,\n"
                                   "FOREIGN KEY (id_usuario) REFERENCES usuarios (id),\n"
                                   "FOREIGN KEY (id_rol) REFERENCES roles (id)\n"
                                   ")");
            
            this->sqlite3->command(R"(
                CREATE table terminales_pago(
            	id text not null PRIMARY KEY ,
            	tipo text not null,
                alias text,
            	modo text,
                acces_token text,
                predeterminado integer not null DEFAULT 0,
            	descripcion text,
            	fecha_creado text NOT NULL 
            );)");

            this->sqlite3->command(R"(
                CREATE TABLE pago_tarjeta (
                id                          INTEGER PRIMARY KEY AUTOINCREMENT,
                id_terminal                 TEXT NOT NULL,                      -- REFERENCES terminales_pago(id)
                id_order                    TEXT NOT NULL UNIQUE,               -- Tu ID interno (external_reference que envías)
                        
                mp_order_id                 TEXT NOT NULL UNIQUE,               -- "ORD01JSYD7XEZ2371TTHHMT5BWWTG" (el ID principal de la orden)
                mp_payment_id               TEXT,                               -- "PAY01JSYD7XEZ2371TTHHMVWYJYG6" (dentro de transactions.payments[0].id)
                        
                id_log                      INTEGER NOT NULL,                   -- enlace a tu log de validador/operación
                        
                monto                       REAL NOT NULL CHECK(monto > 0),     -- amount del payment
                tipo                        TEXT NOT NULL,                      -- ej: 'credit_card', 'debit_card' (de config.payment_method.default_type o posterior)
                estado                      TEXT NOT NULL DEFAULT 'created',    -- order.status: created, processed, paid, canceled...
                estado_detalle              TEXT DEFAULT 'created',             -- order.status_detail o payment.status_detail
                        
                mp_reference_id             TEXT,                               -- reference_id si aparece en payments
                mp_transaction_id           TEXT,                               -- transaction_id si llega (raro en Point, pero posible)
                last_four_digits            TEXT,                               -- si se llena en pagos con tarjeta
                issuer_name                 TEXT,                               -- nombre del banco emisor si disponible
                        
                fecha_creado                TEXT NOT NULL DEFAULT (datetime('now','localtime')),
                fecha_aprobacion            TEXT,                               -- date_approved o cuando status pasa a approved/paid
                fecha_ult_actualizacion     TEXT DEFAULT (datetime('now','localtime')),
                        
                descripcion                 TEXT,                               -- order.description o tu propia nota
                ultimo_error                TEXT,                               -- si status = rejected o error en detalle
                        
                mp_json_response            TEXT,                               -- opcional: guarda el JSON completo de la respuesta GET /orders para auditoría/debug
                        
                FOREIGN KEY (id_terminal) REFERENCES terminales_pago(id),
                FOREIGN KEY (id_log)      REFERENCES log(id)
            );)");

            this->sqlite3->command(R"(
                CREATE TABLE detalle_movimientos_dinero (
                id                INTEGER PRIMARY KEY AUTOINCREMENT,
                id_log            INTEGER NOT NULL,
                tipo_movimiento   TEXT NOT NULL CHECK(tipo_movimiento IN ('entrada', 'salida')),
                denominacion      INTEGER NOT NULL,
                cantidad          INTEGER NOT NULL DEFAULT 0,
                creado_en         DATETIME DEFAULT (datetime('now','localtime')),

                FOREIGN KEY (id_log) REFERENCES log(id),

                CHECK (cantidad >= 0)
            );)");

            this->sqlite3->command("insert into roles values \n"
                                   "(NULL,'Venta'),\n"
                                   "(NULL,'Pago'),\n"
                                   "(NULL,'Carga'),\n"
                                   "(NULL,'Retirada'),\n"
                                   "(NULL,'Cambio M'),\n"
                                   "(NULL,'Cambio A'),\n"
                                   "(NULL,'Ingresos'),\n"
                                   "(NULL,'Enviar a casette'),\n"
                                   "(NULL,'Retirada a casette'),\n"
                                   "(NULL,'Consulta de efectivo'),\n"
                                   "(NULL,'Mov. Pendientes'),\n"
                                   "(NULL,'Consulta de Movimientos'),\n"
                                   "(NULL,'Cierre con Faltantes'),\n"
                                   "(NULL,'Estadisticas'),\n"
                                   "(NULL,'Fianza'),\n"
                                   "(NULL,'Mostrar Reportes'),\n"
                                   "(NULL,'Configuracion'),\n"
                                   "(NULL,'Salir a Escritorio'),\n"
                                   "(NULL,'Apagar equipo');");
            this->sqlite3->command("insert into usuarios values (null,'admin','admin');");

            this->sqlite3->command("CREATE TABLE log (Id INTEGER PRIMARY KEY AUTOINCREMENT, IdUser INT, Tipo text, Descripcion text, Ingreso real, Cambio real, Total real, Estatus text, Fecha text , FOREIGN KEY (IdUser) REFERENCES usuarios (id))");
            this->sqlite3->command("CREATE TABLE pagoPendiente (Id INTEGER PRIMARY KEY AUTOINCREMENT, IdLog int, Remanente real, Estatus text, FOREIGN KEY (IdLog) REFERENCES log (Id))");

            this->sqlite3->command("CREATE TABLE IF NOT EXISTS usuarios_historial (\n"
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                   "original_id INTEGER NOT NULL,\n"
                                   "old_username VARCHAR(255),\n"
                                   "old_password VARCHAR(255),\n"
                                   "new_username VARCHAR(255),\n"
                                   "new_password VARCHAR(255),\n"
                                   "fecha_modificacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,\n"
                                   "usuario_operacion VARCHAR(255),\n"
                                   "tipo_cambio VARCHAR(10)\n"
                                   ");");

            this->sqlite3->command("CREATE TRIGGER IF NOT EXISTS after_usuarios_insert\n"
                                   "AFTER INSERT ON usuarios\n"
                                   "FOR EACH ROW\n"
                                   "BEGIN\n"
                                   "INSERT INTO usuarios_historial (\n"
                                   "original_id,\n"
                                   "new_username,\n"
                                   "new_password,\n"
                                   "usuario_operacion,\n"
                                   "tipo_cambio\n"
                                   ") VALUES (\n"
                                   "NEW.id,\n"
                                   "NEW.username,\n"
                                   "NEW.password,\n"
                                   "'system_user', \n"
                                   " 'INSERT'\n"
                                   ");\n"
                                   "END;");
            this->sqlite3->command("CREATE TRIGGER IF NOT EXISTS after_usuarios_update\n"
                                   "AFTER UPDATE ON usuarios\n"
                                   "FOR EACH ROW\n"
                                   "BEGIN\n"
                                   "INSERT INTO usuarios_historial (\n"
                                   "original_id,\n"
                                   "old_username,\n"
                                   "old_password,\n"
                                   "new_username,\n"
                                   "new_password,\n"
                                   "usuario_operacion,\n"
                                   "tipo_cambio\n"
                                   ") VALUES (\n"
                                   "NEW.id,\n"
                                   "OLD.username,\n"
                                   "OLD.password,\n"
                                   "NEW.username,\n"
                                   "NEW.password,\n"
                                   "'system_user',\n"
                                   "'UPDATE'\n"
                                   ");\n"
                                   "END;");

            this->sqlite3->command("CREATE TRIGGER IF NOT EXISTS after_usuarios_delete\n"
                                   "AFTER DELETE ON usuarios\n"
                                   "FOR EACH ROW\n"
                                   "BEGIN\n"
                                   "INSERT INTO usuarios_historial (\n"
                                   "original_id,\n"
                                   "old_username,\n"
                                   "old_password,\n"
                                   "usuario_operacion,\n"
                                   "tipo_cambio\n"
                                   ") VALUES (\n"
                                   "OLD.id,\n"
                                   "OLD.username,\n"
                                   "OLD.password,\n"
                                   "'system_user',\n"
                                   "'DELETE'\n"
                                   ");\n"
                                   "END;");

            this->sqlite3->command("CREATE TABLE IF NOT EXISTS log_historial (\n"
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                   "original_id INTEGER NOT NULL,\n"
                                   "old_id_user INTEGER,\n"
                                   "old_tipo VARCHAR(255),\n"
                                   "old_ingreso DECIMAL(10, 2),\n"
                                   "old_cambio DECIMAL(10, 2),\n"
                                   "old_total DECIMAL(10, 2),\n"
                                   "old_estatus VARCHAR(255),\n"
                                   "old_fecha TIMESTAMP,\n"
                                   "new_id_user INTEGER,\n"
                                   "new_tipo VARCHAR(255),\n"
                                   "new_ingreso DECIMAL(10, 2),\n"
                                   "new_cambio DECIMAL(10, 2),\n"
                                   "new_total DECIMAL(10, 2),\n"
                                   "new_estatus VARCHAR(255),\n"
                                   "new_fecha TIMESTAMP,\n"
                                   "fecha_modificacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,\n"
                                   "usuario_operacion VARCHAR(255),\n"
                                   "tipo_cambio VARCHAR(10)\n"
                                   ");");

            this->sqlite3->command("CREATE TRIGGER IF NOT EXISTS after_log_insert\n"
                                   "AFTER INSERT ON log\n"
                                   "FOR EACH ROW\n"
                                   "BEGIN\n"
                                   "INSERT INTO log_historial (\n"
                                   "original_id,\n"
                                   "new_id_user,\n"
                                   "new_tipo,\n"
                                   "new_ingreso,\n"
                                   "new_cambio,\n"
                                   "new_total,\n"
                                   "new_estatus,\n"
                                   "new_fecha,\n"
                                   "usuario_operacion,\n"
                                   "tipo_cambio\n"
                                   ") VALUES (\n"
                                   "NEW.Id,\n"
                                   "NEW.idUser,\n"
                                   "NEW.tipo,\n"
                                   "NEW.ingreso,\n"
                                   "NEW.cambio,\n"
                                   "NEW.total,\n"
                                   "NEW.estatus,\n"
                                   "NEW.fecha,\n"
                                   "'system_user',\n"
                                   "'INSERT'\n"
                                   " );\n"
                                   "END;");

            this->sqlite3->command("CREATE TRIGGER IF NOT EXISTS after_log_update\n"
                                   "AFTER UPDATE ON log\n"
                                   "FOR EACH ROW\n"
                                   "BEGIN\n"
                                   "INSERT INTO log_historial (\n"
                                   "original_id,\n"
                                   "old_id_user,\n"
                                   "old_tipo,\n"
                                   "old_ingreso,\n"
                                   "old_cambio,\n"
                                   "old_total,\n"
                                   "old_estatus,\n"
                                   "old_fecha,\n"
                                   "new_id_user,\n"
                                   "new_tipo,\n"
                                   "new_ingreso,\n"
                                   "new_cambio,\n"
                                   "new_total,\n"
                                   "new_estatus,\n"
                                   "new_fecha,\n"
                                   "usuario_operacion,\n"
                                   "tipo_cambio\n"
                                   ") VALUES (\n"
                                   "NEW.Id,\n"
                                   "OLD.idUser,\n"
                                   "OLD.tipo,\n"
                                   "OLD.ingreso,\n"
                                   "OLD.cambio,\n"
                                   "OLD.total,\n"
                                   "OLD.estatus,\n"
                                   "OLD.fecha,\n"
                                   "NEW.idUser,\n"
                                   "NEW.tipo,\n"
                                   "NEW.ingreso,\n"
                                   "NEW.cambio,\n"
                                   "NEW.total,\n"
                                   "NEW.estatus,\n"
                                   "NEW.fecha,\n"
                                   "'system_user',\n"
                                   "'UPDATE'\n"
                                   ");\n"
                                   "END;");

            this->sqlite3->command("CREATE TRIGGER IF NOT EXISTS after_log_delete\n"
                                   "AFTER DELETE ON log\n"
                                   "FOR EACH ROW\n"
                                   "BEGIN\n"
                                   "INSERT INTO log_historial (\n"
                                   "original_id,\n"
                                   "old_id_user,\n"
                                   "old_tipo,\n"
                                   "old_ingreso,\n"
                                   "old_cambio,\n"
                                   "old_total,\n"
                                   "old_estatus,\n"
                                   "old_fecha,\n"
                                   "usuario_operacion,\n"
                                   "tipo_cambio\n"
                                   ") VALUES (\n"
                                   "OLD.Id,\n"
                                   "OLD.idUser,\n"
                                   "OLD.tipo,\n"
                                   "OLD.ingreso,\n"
                                   "OLD.cambio,\n"
                                   "OLD.total,\n"
                                   "OLD.estatus,\n"
                                   "OLD.fecha,\n"
                                   "'system_user',\n"
                                   "'DELETE'\n"
                                   ");\n"
                                   "END;");

            std::string ROL_ADMIN{"INSERT INTO usuario_roles values"};
            for (size_t i = 0; i < 19; i++)
            {
                if (i != 18)
                    ROL_ADMIN += "(NULL,1," + std::to_string(i + 1) + "),\n";
                else
                    ROL_ADMIN += "(NULL,1," + std::to_string(i + 1) + ")";
            }
            this->sqlite3->command(ROL_ADMIN);

            this->sqlite3->command("create table Level_Bill (Denominacion INT, Cant_Alm INT, Cant_Recy INT ,Nivel_Inmo_Min int, Nivel_Inmo int, Nivel_Inmo_Max int)");
            this->sqlite3->command("create table Level_Coin (Denominacion INT, Cant_Alm INT, Cant_Recy INT ,Nivel_Inmo_Min int, Nivel_Inmo int, Nivel_Inmo_Max int)");
            this->sqlite3->command("insert INTO Level_Bill values (20,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Bill values (50,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Bill values (100,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Bill values (200,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Bill values (500,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Bill values (1000,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Coin values (1,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Coin values (2,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Coin values (5,0,0,20,20,20)");
            this->sqlite3->command("insert INTO Level_Coin values (10,0,0,20,20,20)");

            this->sqlite3->command("CREATE TABLE configuracion (\n"
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                   "descripcion TEXT NOT NULL,\n"
                                   "valor TEXT NOT NULL\n"
                                   ")");
            this->sqlite3->command("INSERT into configuracion values"
                                    //1 - 4
                                   "(null,'Puerto bill','/dev/ttyUSB0'),"
                                   "(null,'SSP bill','0'),"
                                   "(null,'Puerto coin','/dev/ttyUSB1'),"
                                   "(null,'SSP coin','16'),"
                                   // 5 - 9
                                   "(null,'Mostrar Notificacion','1'),"
                                   "(null,'Ruta logo incio','$HOME/Documentos/img/8121259.gif'),"
                                   "(null,'Mensaje Inicio','Bienvenido'),"
                                   "(null,'Ruta Carrusel','$HOME/Documentos/img_carrusel'),"
                                   "(null,'Temp. Carrusel','0'),"
                                   // 10 - 14
                                   "(null,'Razon Social',''),"
                                   "(null,'Direccion',''),"
                                   "(null,'RFC',''),"
                                   "(null,'Contacto',''),"
                                   "(null,'Agradecimiento',''),"
                                   // 15 - 21
                                   "(null,'Activa Impresion','1'),"
                                   "(null,'Visualiza Agradecimineto','1'),"
                                   "(null,'Visualiza Fecha','1'),"
                                   "(null,'Visualiza Direccion','1'),"
                                   "(null,'Visualiza RFC','1'),"
                                   "(null,'Visualiza Vendedor','1'),"
                                   "(null,'Visualiza Contacto','1'),"
                                   // nip 100
                                    "(100,'Nip','123456')");
        }
    }

public:
    std::unique_ptr<SQLite3::SQLite> sqlite3;
    static Database &getInstance()
    {
        static Database instance; // Instancia única de la clase
        return instance;
    }
};