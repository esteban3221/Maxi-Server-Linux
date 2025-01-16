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

            this->sqlite3->command("CREATE TABLE log (Id INTEGER PRIMARY KEY AUTOINCREMENT, IdUser INT, Tipo text, Ingreso real, Cambio real, Total real, Estatus text, Fecha text , FOREIGN KEY (IdUser) REFERENCES usuarios (id))");
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

            this->sqlite3->command("create table Level_Bill (Denominacion INT, Cant_Alm INT, Cant_Recy INT , Nivel_Inmo int)");
            this->sqlite3->command("create table Level_Coin (Denominacion INT, Cant_Alm INT, Cant_Recy INT , Nivel_Inmo int)");
            this->sqlite3->command("insert INTO Level_Bill values (20,0,0,0)");
            this->sqlite3->command("insert INTO Level_Bill values (50,0,0,0)");
            this->sqlite3->command("insert INTO Level_Bill values (100,0,0,0)");
            this->sqlite3->command("insert INTO Level_Bill values (200,0,0,0)");
            this->sqlite3->command("insert INTO Level_Bill values (500,0,0,0)");
            this->sqlite3->command("insert INTO Level_Bill values (1000,0,0,0)");
            this->sqlite3->command("insert INTO Level_Coin values (1,0,0,0)");
            this->sqlite3->command("insert INTO Level_Coin values (2,0,0,0)");
            this->sqlite3->command("insert INTO Level_Coin values (5,0,0,0)");
            this->sqlite3->command("insert INTO Level_Coin values (10,0,0,0)");

            this->sqlite3->command("CREATE TABLE configuracion (\n"
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                   "descripcion TEXT NOT NULL,\n"
                                   "valor TEXT NOT NULL\n"
                                   ")");
            this->sqlite3->command("INSERT into configuracion values(null,'Puerto bill','/dev/ttyUSB0'),"
                                   "(null,'SSP bill','0'),"
                                   "(null,'Puerto coin','/dev/ttyUSB1'),"
                                   "(null,'SSP coin','16')");
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