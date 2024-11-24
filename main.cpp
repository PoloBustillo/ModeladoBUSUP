#include <iostream>
#include <vector>
#include <string>
#include "sqlite3/sqlite3.h"
#include "colormode.h"
#include "utils.h"
#include "configs.h"
#include <uuid/uuid.h>

class Usuario;
class Cuenta;

enum class StatusBoleto
{
    Activo,
    Nuevo,
    Usado
};
class Boleto
{
public:
    Boleto(const std::string &fechaExpiracion, StatusBoleto status)
        : fechaExpiracion(fechaExpiracion), status(status)
    {
        uuid_t uuid;
        uuid_generate(uuid);
        char uuid_str[37];
        uuid_unparse(uuid, uuid_str);
        id = std::string(uuid_str);
    }

    void mostrar() const
    {
        std::cout << "====================================================================\n"
                  << "|                                                                  |\n"
                  << "|                           \033[1;35mBoleto Info\033[0m                            |\n"
                  << "|                                                                  |\n"
                  << "====================================================================\n"
                  << "\033[1;35mID del Boleto:\033[0m \033[1;33m" << id << "\033[0m\n"
                  << "\033[1;35mFecha de Expiración:\033[0m \033[1;33m" << fechaExpiracion << "\033[0m\n"
                  << "\033[1;35mEstado:\033[0m \033[1;33m" << (status == StatusBoleto::Activo ? "Activo" : status == StatusBoleto::Nuevo ? "Nuevo"
                                                                                                                                         : "Usado")
                  << "\033[0m\n"
                  << "====================================================================\n";
    }

private:
    std::string id;
    std::string fechaExpiracion;
    StatusBoleto status;
};

class TarjetaBancaria
{
public:
    TarjetaBancaria(const std::string &numero, const std::string &fechaExpiracion, const std::string &cvv)
        : numero(numero), fechaExpiracion(fechaExpiracion), cvv(cvv) {}

    const std::string &getNumero() const { return numero; }
    const std::string &getFechaExpiracion() const { return fechaExpiracion; }
    const std::string &getCvv() const { return cvv; }

private:
    std::string numero;
    std::string fechaExpiracion;
    std::string cvv;
};

class DatabaseManager
{
public:
    Usuario obtenerUsuario(const std::string &matricula);
    Usuario crearUsuario();
    Cuenta crearCuenta(const std::string &usuario, int boletos, const std::string &tarjetas, double saldo);
    static DatabaseManager &getInstance()
    {
        static DatabaseManager instance;
        if (!instance.db)
        {
            instance.abrirBaseDeDatos(Config::getInstance().getDbName());
            instance.crearTablaUsuarios();
            instance.crearTablaBoletos();
            instance.crearTablaCuentas();
            instance.crearTablaTarjetasBancarias();
        }
        return instance;
    }
    sqlite3 *getDB() const
    {
        return db;
    }

    void cerrarBaseDeDatos()
    {
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
        }
    }
    static int callback(void *NotUsed, int argc, char **argv, char **azColName)
    {
        for (int i = 0; i < argc; i++)
        {
            printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        printf("\n");
        return 0;
    }

private:
    DatabaseManager() : db(nullptr) {}
    ~DatabaseManager()
    {
        cerrarBaseDeDatos();
    }

    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    sqlite3 *db;

    void crearTablaCuentas()
    {
        const char *sql;
        char *zErrMsg = 0;
        int rc;

        sql = "CREATE TABLE IF NOT EXISTS CUENTAS("
              "ID TEXT PRIMARY KEY NOT NULL,"
              "USUARIO TEXT NOT NULL,"
              "BOLETOS TEXT,"
              "TARJETAS TEXT,"
              "SALDO REAL NOT NULL,"
              "FOREIGN KEY(USUARIO) REFERENCES USERS(MATRICULA));";

        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            sqlite3_free(zErrMsg);
            Utils::printError("Error al crear la tabla de cuentas en la base de datos.");
            throw std::runtime_error("Error al crear la tabla de cuentas en la base de datos.");
        }
        else
        {
            Utils::printSuccess("Tabla de cuentas creada exitosamente!");
        }
    }
    void crearTablaTarjetasBancarias()
    {
        const char *sql;
        char *zErrMsg = 0;
        int rc;

        sql = "CREATE TABLE IF NOT EXISTS TARJETASBANCARIAS("
              "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
              "NUMERO_TARJETA TEXT NOT NULL,"
              "EXPIRACION TEXT NOT NULL);";

        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            sqlite3_free(zErrMsg);
            Utils::printError("Error al crear la tabla de tarjetas bancarias en la base de datos.");
            throw std::runtime_error("Error al crear la tabla de tarjetas bancarias en la base de datos.");
        }
        else
        {
            Utils::printSuccess("Tabla de tarjetas bancarias creada exitosamente!");
        }
    }
    void crearTablaBoletos()
    {
        const char *sql;
        char *zErrMsg = 0;
        int rc;

        sql = "CREATE TABLE IF NOT EXISTS BOLETOS("
              "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
              "EXPIRACION TEXT NOT NULL,"
              "STATUS TEXT NOT NULL CHECK(STATUS IN ('activo', 'usado', 'nuevo')));";

        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            sqlite3_free(zErrMsg);
            Utils::printError("Error al crear la tabla de boletos en la base de datos.");
            throw std::runtime_error("Error al crear la tabla de boletos en la base de datos.");
        }
        else
        {
            Utils::printSuccess("Tabla de boletos creada exitosamente!");
        }
    }
    void crearTablaUsuarios()
    {
        const char *sql;
        char *zErrMsg = 0;
        int rc;

        sql = "CREATE TABLE IF NOT EXISTS USERS("
              "MATRICULA TEXT PRIMARY KEY NOT NULL,"
              "NAME TEXT NOT NULL,"
              "PASSWORD TEXT NOT NULL,"
              "TELEFONO TEXT NOT NULL,"
              "CUENTA TEXT NOT NULL,"
              "FOREIGN KEY(CUENTA) REFERENCES CUENTAS(ID));";

        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            sqlite3_free(zErrMsg);
            Utils::printError("Error al crear la tabla de usuarios en la base de datos.");
            throw std::runtime_error("Error al crear la tabla de usuarios en la base de datos.");
        }
        else
        {
            Utils::printSuccess("Tabla de usuarios carga exitosamente!");
        }
    }

    bool abrirBaseDeDatos(const std::string &nombreDB)
    {
        int rc = sqlite3_open(nombreDB.c_str(), &db);
        Utils::mostrarBarraDeCarga(1);
        if (rc)
        {
            Utils::printError("No se pudo abrir: " + std::string(sqlite3_errmsg(db)));
            return false;
        }
        else
        {
            Utils::printSuccess("DB inicializada!");
            return true;
        }
    }
};

class Cuenta
{
public:
    Cuenta(double saldoInicial = 0.0)
        : saldo(saldoInicial), cantidadBoletos(0), idBoletoActual(0) {}
    Cuenta(double saldoInicial, int cantidadBoletos, int idBoletoActual)
        : saldo(saldoInicial), cantidadBoletos(cantidadBoletos), idBoletoActual(idBoletoActual) {}

    double getSaldo() const { return saldo; }
    int getCantidadBoletos() const { return cantidadBoletos; }
    int getIdBoletoActual() const { return idBoletoActual; }

    void abonar(double cantidad)
    {
        if (cantidad > 0)
        {
            saldo += cantidad;
            Utils::printSuccess("Abono realizado exitosamente.");
        }
        else
        {
            Utils::printError("Cantidad inválida para abonar.");
        }
    }

    void comprarBoleto()
    {
        if (saldo >= Config::getInstance().getBoletoCosto())
        {
            saldo -= Config::getInstance().getBoletoCosto();
            cantidadBoletos++;
            Utils::printSuccess("Boleto comprado exitosamente.");
        }
        else
        {
            Utils::printError("Saldo insuficiente para comprar un boleto.");
        }
    }

    void usarBoleto()
    {
        if (cantidadBoletos > 0)
        {
            cantidadBoletos--;
            Utils::printSuccess("Boleto usado exitosamente.");
        }
        else
        {
            Utils::printError("No tienes boletos disponibles para usar.");
        }
    }

private:
    double saldo;
    int cantidadBoletos;
    int idBoletoActual;
    std::vector<TarjetaBancaria> tarjetasBancarias;
};

class Usuario
{
public:
    Usuario(const std::string &matricula, const std::string &nombre, const std::string &password, const std::string &telefono, Cuenta cuenta)
        : matricula(matricula), nombre(nombre), password(password), telefono(telefono), cuenta(cuenta) {}

    const std::string &getMatricula() const { return matricula; }
    const std::string &getNombre() const { return nombre; }
    const std::string &getPassword() const { return password; }
    const std::string &getTelefono() const { return telefono; }
    Cuenta &getCuenta() { return cuenta; }

    void mostrar() const
    {
        std::cout << "===========================\n"
                  << "|                         |\n"
                  << "|      \033[1;35mUsuario Info\033[0m       |\n"
                  << "|                         |\n"
                  << "===========================\n"
                  << "\033[1;35mMatrícula:\033[0m \033[1;33m" << matricula << "\033[0m\n"
                  << "\033[1;35mNombre:\033[0m \033[1;33m" << nombre << "\033[0m\n"
                  << "\033[1;35mTeléfono:\033[0m \033[1;33m" << telefono << "\033[0m\n"
                  << "\033[1;35mSaldo:\033[0m \033[1;33m$" << cuenta.getSaldo() << "\033[0m\n"
                  << "===========================\n";
    }

    void setMatricula(const std::string &mat) { matricula = mat; }
    void setPassword(const std::string &pass) { password = pass; }

    bool iniciarSesion()
    {

        try
        {
            Usuario usuarioFromDB = DatabaseManager::getInstance().obtenerUsuario(matricula);
            if (usuarioFromDB.getPassword() == password)
            {
                Utils::printSuccess("¡Inicio de sesión exitoso!");
                nombre = usuarioFromDB.getNombre();
                telefono = usuarioFromDB.getTelefono();
                return true;
            }
            else
            {
                Utils::printError("Contraseña incorrecta");
                throw std::runtime_error("Contraseña incorrecta");
            }
        }
        catch (const std::runtime_error &e)
        {
            Utils::printError(e.what());
            throw std::runtime_error("Error al iniciar sesión.");
        }
    }
    void eliminarCuenta()
    {
        std::string sqlDelete = "DELETE FROM USERS WHERE MATRICULA = '" + matricula + "';";
        char *zErrMsg = 0;
        int rc = sqlite3_exec(DatabaseManager::getInstance().getDB(), sqlDelete.c_str(), DatabaseManager::callback, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            Utils::printError("Error al eliminar la cuenta: " + std::string(zErrMsg));
            sqlite3_free(zErrMsg);
            throw std::runtime_error("Error al eliminar la cuenta de la base de datos.");
        }
        else
        {
            Utils::printSuccess("Cuenta eliminada exitosamente");
        }
    }

private:
    std::string matricula;
    std::string nombre;
    std::string password;
    std::string telefono;
    Cuenta cuenta;
};

Usuario DatabaseManager::obtenerUsuario(const std::string &matricula)
{
    std::string sqlSelect = "SELECT * FROM USERS WHERE MATRICULA = '" + matricula + "';";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(getDB(), sqlSelect.c_str(), -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        Utils::printError("Error al preparar la consulta: " + std::string(sqlite3_errmsg(db)));
        throw std::runtime_error("Error al obtener el usuario de la base de datos.");
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        std::string nombre = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        std::string password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        std::string telefono = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        sqlite3_finalize(stmt);
        return Usuario(matricula, nombre, password, telefono, Cuenta());
    }

    sqlite3_finalize(stmt);
    throw std::runtime_error("Usuario no encontrado.");
}

Cuenta DatabaseManager::crearCuenta(const std::string &usuario, int boletos, const std::string &tarjetas, double saldo)
{
    std::string sqlInsert = "INSERT INTO CUENTAS (USUARIO, BOLETOS, TARJETAS, SALDO) VALUES ('" +
                            usuario + "', " + std::to_string(boletos) + ", '" + tarjetas + "', " + std::to_string(saldo) + ");";
    char *zErrMsg = 0;
    int rc = sqlite3_exec(getDB(), sqlInsert.c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        Utils::printError("SQL error: " + std::string(zErrMsg));
        sqlite3_free(zErrMsg);
        throw std::runtime_error("Error al crear la cuenta en la base de datos.");
    }
    else
    {
        Utils::printSuccess("Cuenta creada exitosamente");
        return Cuenta(saldo);
    }
}

Usuario DatabaseManager::crearUsuario()
{
    std::string matricula, nombre, password, telefono;
    double saldo;
    char *zErrMsg = 0;

    std::cout << "\033[1;33mIngrese la matrícula:\033[0m ";
    std::cin >> matricula;
    std::cout << "\033[1;33mIngrese el nombre:\033[0m ";
    std::cin.ignore();
    std::getline(std::cin, nombre);
    std::cout << "\033[1;33mIngrese la contraseña:\033[0m ";
    std::getline(std::cin, password);
    std::cout << "\033[1;33mIngrese el teléfono:\033[0m ";
    std::getline(std::cin, telefono);

    // TODO: CREAR CUENTA PARA INSERTARLA

    std::string sqlInsert = "INSERT INTO USERS (MATRICULA, NAME, PASSWORD, TELEFONO, CUENTA) VALUES (" +
                            matricula + ", '" + nombre + "', '" + password + "', '" + telefono + "', " + std::to_string(0) + ");";

    int rc = sqlite3_exec(getDB(), sqlInsert.c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        Utils::printError("SQL error: " + std::string(zErrMsg));
        sqlite3_free(zErrMsg);
        throw std::runtime_error("Error al crear el usuario en la base de datos.");
    }
    else
    {
        Utils::printSuccess("Usuario creado exitosamente");
    }

    return Usuario(matricula, nombre, password, telefono, Cuenta());
}

int main()
{
    sqlite3 *db = DatabaseManager::getInstance().getDB();

    int opcion;
    std::string matricula, contrasena;
    Usuario usuario("", "", "", "", Cuenta());

    while (true)
    {
        std::vector<std::string> menuOptions = {"Iniciar Sesión", "Crear Usuario", "Salir"};
        Utils::printMenu(menuOptions);
        std::cin >> opcion;

        if (opcion == 3)
        {
            Utils::printSuccess("Saliendo...");
            break;
        }

        try
        {
            switch (opcion)
            {
            case 1:
                std::cout << "\033[1;33mIngrese su nombre de matrícula:\033[0m ";
                std::cin >> matricula;
                std::cout << "\033[1;33mIngrese su contraseña:\033[0m ";
                std::cin >> contrasena;
                usuario.setMatricula(matricula);
                usuario.setPassword(contrasena);
                usuario.iniciarSesion();
                usuario.mostrar();
                break;
            case 2:
                usuario = DatabaseManager::getInstance().crearUsuario();
                usuario.mostrar();
                break;
            default:
                Utils::printError("Opción inválida. Intente de nuevo!");
                break;
            }
            if (usuario.getMatricula() != "")
            {
                break;
            }
        }
        catch (const std::runtime_error &e)
        {
            Utils::printError(e.what());
        }
    }

    while (true)
    {
        std::vector<std::string> menuOptions = {"Abonar a mi cuenta", "Eliminar mi cuenta", "Comprar boleto", "Usar boleto", "Salir"};
        Utils::printMenu(menuOptions);
        std::cin >> opcion;

        if (opcion == 5)
        {
            Utils::printSuccess("Saliendo...");
            break;
        }

        try
        {
            switch (opcion)
            {
            case 1:

                break;
            case 2:
                usuario.eliminarCuenta();
                break;
            case 3:
                if (usuario.getCuenta().getSaldo() >= Config::getInstance().getBoletoCosto())
                {
                    usuario.getCuenta().comprarBoleto();
                    std::time_t t = std::time(nullptr);
                    std::tm *tm = std::localtime(&t);
                    tm->tm_mday += 10;
                    std::mktime(tm);
                    char buffer[11];
                    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
                    std::string fechaExpiracion(buffer);
                    Boleto boleto(fechaExpiracion, StatusBoleto::Nuevo);
                    boleto.mostrar();
                }
                else
                {
                    Utils::printError("Saldo insuficiente para comprar un boleto.");
                }
                break;
            case 4:
                // Código para usar boleto
                Utils::printError("Funcionalidad no implementada.");
                break;
            default:
                Utils::printError("Opción inválida. Intente de nuevo!");
                break;
            }
        }
        catch (const std::runtime_error &e)
        {
            Utils::printError(e.what());
        }
    }
    return 0;
}
