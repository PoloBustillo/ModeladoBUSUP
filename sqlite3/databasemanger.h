#include "sqlite3.h"
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdio>

class Usuario;
class Cuenta;
class TarjetaBancaria;
class Transaccion;

class DatabaseManager
{
public:
    static DatabaseManager &getInstance()
    {
        static DatabaseManager instance;
        if (!instance.db)
        {
            instance.openDatabase(Config::getInstance().getDbName());
            instance.createTables();
        }
        return instance;
    }

    Usuario getUser(const std::string &matricula);
    Cuenta getAccount(const std::string &accountId);
    void registerTransaction(Transaccion transaction);
    std::vector<TarjetaBancaria> getCards(const std::string &accountId);
    Usuario createUser();
    Cuenta createAccount(const std::string &user, int tickets, const std::string &cards, double balance);
    TarjetaBancaria addBankCard(TarjetaBancaria newCard, const std::string &accountId);

    sqlite3 *getDB() const { return db; }

    void closeDatabase()
    {
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
        }
    }

private:
    DatabaseManager() : db(nullptr) {}
    ~DatabaseManager() { closeDatabase(); }

    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    sqlite3 *db;

    static int callback(void *NotUsed, int argc, char **argv, char **azColName)
    {
        for (int i = 0; i < argc; i++)
        {
            printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        printf("\n");
        return 0;
    }

    void createTables()
    {
        createTable("CREATE TABLE IF NOT EXISTS CUENTAS("
                    "ID TEXT PRIMARY KEY NOT NULL,"
                    "SALDO REAL NOT NULL);",
                    "Error creando tabla CUENTAS.");

        createTable("CREATE TABLE IF NOT EXISTS TARJETASBANCARIAS("
                    "ID TEXT PRIMARY KEY NOT NULL,"
                    "NUMERO TEXT NOT NULL,"
                    "EXPIRACION TEXT NOT NULL,"
                    "CUENTA TEXT NOT NULL,"
                    "FOREIGN KEY(CUENTA) REFERENCES CUENTAS(ID));",
                    "Error creando tabla TARJETASBANCARIAS.");

        createTable("CREATE TABLE IF NOT EXISTS BOLETOS("
                    "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "EXPIRACION TEXT NOT NULL,"
                    "STATUS TEXT NOT NULL CHECK(STATUS IN ('activo', 'usado', 'nuevo')),"
                    "CUENTA TEXT NOT NULL,"
                    "FOREIGN KEY(CUENTA) REFERENCES CUENTAS(ID));",
                    "Error creando tabla BOLETOS.");

        createTable("CREATE TABLE IF NOT EXISTS USERS("
                    "MATRICULA TEXT PRIMARY KEY NOT NULL,"
                    "NAME TEXT NOT NULL,"
                    "PASSWORD TEXT NOT NULL,"
                    "TELEFONO TEXT NOT NULL,"
                    "CUENTA TEXT NOT NULL,"
                    "FOREIGN KEY(CUENTA) REFERENCES CUENTAS(ID));",
                    "Error creando tabla USERS.");

        createTable("CREATE TABLE IF NOT EXISTS TRANSACCION("
                    "ID TEXT PRIMARY KEY NOT NULL,"
                    "MONTO REAL NOT NULL,"
                    "STATUS TEXT NOT NULL CHECK(STATUS IN ('pendiente', 'completada', 'fallida')),"
                    "USUARIO TEXT NOT NULL,"
                    "TARJETAID TEXT NOT NULL,"
                    "FOREIGN KEY(USUARIO) REFERENCES USERS(MATRICULA),"
                    "FOREIGN KEY(TARJETAID) REFERENCES TARJETASBANCARIAS(ID));",
                    "Error creando tabla TRANSACCION.");
    }

    void createTable(const char *sql, const char *errorMessage)
    {
        char *zErrMsg = nullptr;
        int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            sqlite3_free(zErrMsg);
            Utils::printError(errorMessage);
            throw std::runtime_error(errorMessage);
        }
        else
        {
            std::string tableName(sql);
            tableName = tableName.substr(tableName.find("EXISTS") + 7);
            tableName = tableName.substr(0, tableName.find('('));
            Utils::printSuccess("Tabla cargada correctamente: " + tableName);
        }
    }

    bool openDatabase(const std::string &dbName)
    {
        int rc = sqlite3_open(dbName.c_str(), &db);
        Utils::mostrarBarraDeCarga(1);
        if (rc)
        {
            Utils::printError("No se puede abrir la base de datos: " + std::string(sqlite3_errmsg(db)));
            return false;
        }
        else
        {
            Utils::printSuccess("¡Base de datos inicializada!");
            return true;
        }
    }
};
