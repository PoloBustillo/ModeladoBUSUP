#include <iostream>
#include <vector>
#include <string>
#include "sqlite3/sqlite3.h"
#include "colormode.h"
#include <chrono>
#include <thread>
#include <sys/ioctl.h>
#include <unistd.h>

class Usuario;
class Cuenta;

class Utils
{
public:
    static void mostrarBarraDeCarga(int duracionSegundos)
    {
        int anchoBarra = 30;
        std::cout << Utils::centerText("Inicia la base de datos...!");
        std::cout << "\033[5;34m"; // Blink text
        for (int i = 0; i <= anchoBarra; ++i)
        {
            std::cout << "\r["; // Shifted right to be centered
            for (int j = 0; j < i; ++j)
            {
                std::cout << (j % 7 == 0 ? "🚀" : j % 5 == 0 ? "🛸"
                                                             : "✨");
            }
            for (int j = i; j < anchoBarra; ++j)
            {
                std::cout << " ";
            }
            std::cout << "] " << (i * 100) / anchoBarra << "% " << (i == anchoBarra ? "🎉" : "👀");
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(duracionSegundos * 1000 / anchoBarra));
        }
        std::cout << "\n\n\033[0m" << std::endl; // Reset
    }

    static void printError(const std::string &msg)
    {
        std::cout << "\033[1;31m";
        std::cout << Utils::centerText("=====================================================\n");
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("\033[1;41;97m💩 " + msg) << "\033[0m" << "\n";
        std::cout << "\033[1;31m";
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("=====================================================\n");
        std::cout << "\033[0m";
    }

    static void printSuccess(const std::string &msg)
    {
        std::cout << "\033[1;32m"; // Bold Green
        std::cout << Utils::centerText("=====================================================\n");
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("\033[1;42;97m✅ " + msg) << "\033[0m" << "\n";
        std::cout << "\033[1;32m"; // Bold, Green background, White text
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("=====================================================\n");
        std::cout << "\033[0m"; // Reset
    }

    static void printMenu(const std::vector<std::string> &items)
    {
        std::cout << "\033[1;34m"; // Bold Blue
        std::cout << Utils::centerText("=====================================================\n");
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("          \033[1;44;97mMENÚ\033[0m            \n");
        std::cout << "\033[1;34m";
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("=====================================================\n");
        // Reset
        for (size_t i = 0; i < items.size(); ++i)
        {
            std::cout << Utils::centerText("\033[1;34m" + std::to_string(i + 1) + ". 🔸" + items[i] + "🔸\n");
        }
        std::cout << Utils::centerText("=====================================================");
        std::cout << "\033[0m"
                  << "\n\nIngrese su opción: ";
    }

    static std::string centerText(const std::string &text)
    {
        int width = 120;
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        if (w.ws_col)
        {
            width = w.ws_col;
        }
        int printableLength = 0;
        bool escapeSequence = false;
        for (size_t i = 0; i < text.size(); ++i)
        {
            if (text[i] == '\033')
            {
                escapeSequence = true;
            }
            else if (escapeSequence && text[i] == 'm')
            {
                escapeSequence = false;
            }
            else if (!escapeSequence)
            {
                ++printableLength;
            }
        }
        int spaces = (width - printableLength) / 2;
        return std::string(spaces, ' ') + text;
    }
};

class Config
{
public:
    static Config &getInstance()
    {
        static Config instance;
        return instance;
    }

    const std::string &getDbName() const { return dbName; }
    void setDbName(const std::string &name) { dbName = name; }

private:
    Config() : dbName("boletos.db") {}
    ~Config() {}

    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    std::string dbName;
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
              "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
              "USUARIO TEXT NOT NULL,"
              "BOLETOS INTEGER NOT NULL,"
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
              "MATRICULA INT PRIMARY        KEY      NOT NULL,"
              "NAME                         TEXT     NOT NULL,"
              "PASSWORD                     TEXT     NOT NULL,"
              "TELEFONO                     TEXT     NOT NULL,"
              "SALDO                        REAL     NOT NULL);";

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
    Cuenta(double saldoInicial, int cantidadBoletos, int idBoletoActual, const std::string &numeroTarjeta, const std::string &fechaExpiracion, const std::string &cvv, const std::string &cuentaPaypal, const std::string &referenciaPago)
        : saldo(saldoInicial), cantidadBoletos(cantidadBoletos), idBoletoActual(idBoletoActual), tarjetaBancaria({numeroTarjeta, fechaExpiracion, cvv}), cuentaPaypal(cuentaPaypal), referenciaPago(referenciaPago) {}

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
        if (saldo >= 50) // Suponiendo que cada boleto cuesta 50
        {
            saldo -= 50;
            cantidadBoletos++;
            idBoletoActual++;
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

    void agregarTarjetaBancaria(const std::string &numeroTarjeta, const std::string &fechaExpiracion, const std::string &cvv)
    {
        tarjetaBancaria = {numeroTarjeta, fechaExpiracion, cvv};
        Utils::printSuccess("Tarjeta bancaria agregada exitosamente.");
    }

    void agregarCuentaPaypal(const std::string &email)
    {
        cuentaPaypal = email;
        Utils::printSuccess("Cuenta PayPal agregada exitosamente.");
    }

    void agregarReferencia(const std::string &referencia)
    {
        referenciaPago = referencia;
        Utils::printSuccess("Referencia de pago agregada exitosamente.");
    }

private:
    double saldo;
    int cantidadBoletos;
    int idBoletoActual;
    struct TarjetaBancaria
    {
        std::string numero;
        std::string fechaExpiracion;
        std::string cvv;
    } tarjetaBancaria;
    std::string cuentaPaypal;
    std::string referenciaPago;
};

class Usuario
{
public:
    Usuario(const std::string &matricula, const std::string &nombre, const std::string &password, const std::string &telefono, double saldo, Cuenta cuenta)
        : matricula(matricula), nombre(nombre), password(password), telefono(telefono), saldo(saldo), cuenta(cuenta) {}

    const std::string &getMatricula() const { return matricula; }
    const std::string &getNombre() const { return nombre; }
    const std::string &getPassword() const { return password; }
    const std::string &getTelefono() const { return telefono; }

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
                  << "\033[1;35mSaldo:\033[0m \033[1;33m$" << saldo << "\033[0m\n"
                  << "===========================\n";
    }
    double getSaldo() const { return saldo; }

    void setSaldo(double nuevoSaldo) { saldo = nuevoSaldo; }
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
                saldo = usuarioFromDB.getSaldo();
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

    double saldo;
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
        double saldo = sqlite3_column_double(stmt, 4);
        sqlite3_finalize(stmt);
        return Usuario(matricula, nombre, password, telefono, saldo, Cuenta());
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

    std::string sqlInsert = "INSERT INTO USERS (MATRICULA, NAME, PASSWORD, TELEFONO, SALDO) VALUES (" +
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

    return Usuario(matricula, nombre, password, telefono, 0, Cuenta());
}

class Boleto
{
public:
    Boleto(int id, const std::string &nombrePasajero, const std::string &destino)
        : id(id), nombrePasajero(nombrePasajero), destino(destino) {}

    void mostrar() const
    {
        std::cout << "ID del Boleto: " << id << "\n"
                  << "Nombre del Pasajero: " << nombrePasajero << "\n"
                  << "Destino: " << destino << "\n";
    }

private:
    int id;
    std::string nombrePasajero;
    std::string destino;
};

class Autobus
{
public:
    void crearBoleto(const std::string &nombrePasajero, const std::string &destino)
    {
        int id = boletos.size() + 1;
        boletos.emplace_back(id, nombrePasajero, destino);
        std::cout << "¡Boleto creado exitosamente!\n";
    }

    void mostrarBoletos() const
    {
        for (const auto &boleto : boletos)
        {
            boleto.mostrar();
            std::cout << "-------------------\n";
        }
    }

private:
    std::vector<Boleto> boletos;
};

int main()
{
    sqlite3 *db = DatabaseManager::getInstance().getDB();

    int opcion;
    std::string matricula, contrasena;
    Usuario usuario("", "", "", "", 0.0, Cuenta());

    while (true)
    {
        Utils::printMenu({"Iniciar Sesión", "Crear Usuario", "Salir"});
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

    Autobus autobus;
    while (true)
    {
        Utils::printMenu({"Abonar a mi cuenta", "Eliminar mi cuenta", "Comprar boleto", "Usar boleto", "Salir"});
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
