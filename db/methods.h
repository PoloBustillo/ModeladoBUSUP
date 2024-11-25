Usuario DatabaseManager::getUser(const std::string &matricula)
{
    std::string sqlSelect = "SELECT * FROM USERS WHERE MATRICULA = '" + matricula + "';";
    sqlite3_stmt *stmt = DatabaseManager::getInstance().prepareQuery(sqlSelect, "Usuario no Encontrado");

    std::string nombre = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    std::string password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    std::string telefono = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
    std::string cuentaId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));

    Cuenta nuevaCuenta = getAccount(cuentaId);
    sqlite3_finalize(stmt);
    return Usuario(matricula, nombre, password, telefono, nuevaCuenta);
}

std::vector<Boleto> DatabaseManager::getBoletos(const std::string &cuentaId)
{
    std::string sqlSelectBoletos = "SELECT * FROM BOLETOS WHERE CUENTA = '" + cuentaId + "';";
    sqlite3_stmt *stmtBoletos;
    int rc = sqlite3_prepare_v2(getDB(), sqlSelectBoletos.c_str(), -1, &stmtBoletos, 0);
    if (rc != SQLITE_OK)
    {
        Utils::printError("Error al preparar la consulta de boletos: " + std::string(sqlite3_errmsg(db)));
        throw std::runtime_error("Error al obtener los boletos de la base de datos.");
    }

    std::vector<Boleto> boletos;
    while ((rc = sqlite3_step(stmtBoletos)) == SQLITE_ROW)
    {
        std::string idBoleto = reinterpret_cast<const char *>(sqlite3_column_text(stmtBoletos, 0));
        std::string expiracion = reinterpret_cast<const char *>(sqlite3_column_text(stmtBoletos, 1));
        std::string statusStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtBoletos, 2));
        const unsigned char *activeDateText = sqlite3_column_text(stmtBoletos, 4);
        std::string activeDate = activeDateText ? reinterpret_cast<const char *>(activeDateText) : "";
        std::string currentDate = Utils::getDate();
        std::string hourAhead = Utils::getDate(0, 0, 3);
        StatusBoleto status;

        Utils::printSuccess(activeDate);
        Utils::printSuccess(expiracion);
        Utils::printSuccess(activeDate < expiracion ? "true" : "false");

        if (statusStr == "nuevo" && expiracion < currentDate)
        {
            status = StatusBoleto::Usado;
        }
        else if (statusStr == "activo" && expiracion < currentDate)
        {
            status = StatusBoleto::Usado;
        }
        else if (statusStr == "nuevo")
        {
            status = StatusBoleto::Nuevo;
        }
        else if (statusStr == "activo")
        {

            status = StatusBoleto::Activo;
        }
        else if (statusStr == "usado")
        {
            status = StatusBoleto::Usado;
        }

        updateBoletoStatus(idBoleto, status);
        Boleto boleto(expiracion, status);
        boleto.setActiveDate(activeDate);
        boleto.setId(idBoleto);
        boletos.push_back(boleto);
    }
    sqlite3_finalize(stmtBoletos);

    return boletos;
}

Cuenta DatabaseManager::getAccount(const std::string &cuentaId)
{
    std::string sqlSelectCuenta = "SELECT * FROM CUENTAS WHERE ID = '" + cuentaId + "';";
    sqlite3_stmt *stmtCuenta = DatabaseManager::getInstance().prepareQuery(sqlSelectCuenta, "Cuenta no Encontrada");

    std::string idCuenta = reinterpret_cast<const char *>(sqlite3_column_text(stmtCuenta, 0));
    double saldo = sqlite3_column_double(stmtCuenta, 1);

    Cuenta nuevaCuenta(saldo);
    nuevaCuenta.setId(idCuenta);
    nuevaCuenta.setTarjetasBancarias(getCards(idCuenta));
    std::vector<Boleto> boletos = getBoletos(idCuenta);
    std::vector<Boleto> boletosActivos;
    for (const auto &boleto : boletos)
    {
        if (boleto.getStatus() == StatusBoleto::Activo)
        {
            boletosActivos.push_back(boleto);
        }
    }
    nuevaCuenta.setBoletosActuales(boletosActivos);
    nuevaCuenta.setBoletos(boletos);

    sqlite3_finalize(stmtCuenta);
    return nuevaCuenta;
}

std::vector<TarjetaBancaria> DatabaseManager::getCards(const std::string &cuentaId)
{
    std::string sqlSelectTarjetas = "SELECT * FROM TARJETASBANCARIAS WHERE CUENTA = '" + cuentaId + "';";
    sqlite3_stmt *stmtTarjetas;
    int rc = sqlite3_prepare_v2(getDB(), sqlSelectTarjetas.c_str(), -1, &stmtTarjetas, 0);
    if (rc != SQLITE_OK)
    {
        Utils::printError("Error al preparar la consulta de tarjetas: " + std::string(sqlite3_errmsg(db)));
        throw std::runtime_error("Error al obtener las tarjetas de la base de datos.");
    }
    std::vector<TarjetaBancaria> tarjetas;
    while ((rc = sqlite3_step(stmtTarjetas)) == SQLITE_ROW)
    {
        std::string idTarjeta = reinterpret_cast<const char *>(sqlite3_column_text(stmtTarjetas, 0));
        std::string numero = reinterpret_cast<const char *>(sqlite3_column_text(stmtTarjetas, 1));
        std::string expiracion = reinterpret_cast<const char *>(sqlite3_column_text(stmtTarjetas, 2));
        TarjetaBancaria tarjeta(numero, expiracion, "");
        tarjeta.setId(idTarjeta);
        tarjetas.push_back(tarjeta);
    }
    sqlite3_finalize(stmtTarjetas);

    return tarjetas;
}

Cuenta DatabaseManager::createAccount(const std::string &id, double saldo)
{
    std::string sqlInsertCuenta = "INSERT INTO CUENTAS (ID, SALDO) VALUES ('" + id + "', " + std::to_string(saldo) + ");";
    DatabaseManager::getInstance().executeQuery(sqlInsertCuenta, "Error al crear la cuenta en la base de datos.", "Cuenta creada exitosamente");
    return Cuenta(saldo);
}

Usuario DatabaseManager::createUser()
{
    std::string matricula, nombre, password, telefono;
    double saldo;
    char *zErrMsg = 0;

    matricula = Utils::validateMatricula();
    std::cout << "\033[1;33mIngrese el nombre:\033[0m ";
    std::cin.ignore();
    std::getline(std::cin, nombre);
    std::cout << "\033[1;33mIngrese la contraseña:\033[0m ";
    std::cin >> password;
    std::cout << "\033[1;33mIngrese el teléfono:\033[0m ";
    std::cin.ignore();
    std::cin >> telefono;
    while (telefono.length() != 10 || !std::all_of(telefono.begin(), telefono.end(), ::isdigit))
    {
        Utils::printError("El teléfono debe tener 10 dígitos.");
        std::cout << "\033[1;33mIngrese el teléfono:\033[0m ";
        std::cin >> telefono;
    }

    Cuenta nuevaCuenta(0.0);
    std::string id = nuevaCuenta.getId();
    createAccount(id, nuevaCuenta.getSaldo());

    std::string sqlInsert = "INSERT INTO USERS (MATRICULA, NAME, PASSWORD, TELEFONO, CUENTA) VALUES ('" +
                            matricula + "', '" + nombre + "', '" + password + "', '" + telefono + "', '" + id + "');";

    DatabaseManager::getInstance().executeQuery(sqlInsert, "Error al crear el usuario en la base de datos.", "Usuario creado exitosamente");

    return Usuario(matricula, nombre, password, telefono, nuevaCuenta);
}

TarjetaBancaria DatabaseManager::addBankCard(TarjetaBancaria nuevaTarjeta, const std::string &cuentaId)
{
    std::string sqlInsert = "INSERT INTO TARJETASBANCARIAS (ID, NUMERO, EXPIRACION, CUENTA) VALUES ('" +
                            nuevaTarjeta.getId() + "', '" + nuevaTarjeta.getNumero() + "', '" + nuevaTarjeta.getFechaExpiracion() + "', '" + cuentaId + "');";

    DatabaseManager::getInstance().executeQuery(sqlInsert, "Error al agregar la tarjeta bancaria en la base de datos.", "Tarjeta bancaria agregada exitosamente");
    return nuevaTarjeta;
}

void DatabaseManager::registerTransaction(Transaccion transaccion)
{
    std::string sqlInsert = "INSERT INTO TRANSACCION (ID, MONTO, STATUS, USUARIO, TARJETAID) VALUES ('" +
                            transaccion.getId() + "', " + std::to_string(transaccion.getMonto()) + ", '" +
                            (transaccion.getStatus() == StatusTransaccion::Abono ? "abono" : transaccion.getStatus() == StatusTransaccion::Tarjeta ? "tarjeta"
                                                                                                                                                   : "compra") +
                            "', '" +
                            transaccion.getUsuario() + "', '" + transaccion.getTarjetaId() + "');";
    DatabaseManager::getInstance().executeQuery(sqlInsert, "Error al registrar la transacción en la base de datos.", "Transacción registrada exitosamente");

    if (transaccion.getStatus() == StatusTransaccion::Abono)
    {
        std::string sqlUpdate = "UPDATE CUENTAS SET SALDO = SALDO + " + std::to_string(transaccion.getMonto()) +
                                " WHERE ID = (SELECT CUENTA FROM USERS WHERE MATRICULA = '" + transaccion.getUsuario() + "');";
        DatabaseManager::getInstance().executeQuery(sqlUpdate, "Error al actualizar el saldo de la cuenta en la base de datos.", "Saldo de la cuenta actualizado exitosamente");
    }
}

void DatabaseManager::updateBoletoStatus(const std::string &boletoId, StatusBoleto newStatus)
{
    std::string statusStr = (newStatus == StatusBoleto::Nuevo ? "nuevo" : (newStatus == StatusBoleto::Activo ? "activo" : "usado"));
    std::string sqlUpdateStatus = "UPDATE BOLETOS SET STATUS = '" + statusStr + "' WHERE ID = '" + boletoId + "';";
    DatabaseManager::getInstance().executeQuery(sqlUpdateStatus, "Error al actualizar el estado del boleto en la base de datos.", "Estado del boleto actualizado exitosamente");
}
void DatabaseManager::updateBoletoFechaUso(const std::string &boletoId, const std::string &fechaUso)
{
    std::string sqlUpdateFechaUso = "UPDATE BOLETOS SET ACTIVE_DATE = '" + fechaUso + "' WHERE ID = '" + boletoId + "';";
    DatabaseManager::getInstance().executeQuery(sqlUpdateFechaUso, "Error al actualizar la fecha de uso del boleto en la base de datos.", "Fecha de uso del boleto actualizada exitosamente");
}
void DatabaseManager::updateBoletoFechaExpiracion(const std::string &boletoId, const std::string &nuevaFechaExpiracion)
{
    std::string sqlUpdateFechaExpiracion = "UPDATE BOLETOS SET EXPIRACION = '" + nuevaFechaExpiracion + "' WHERE ID = '" + boletoId + "';";
    DatabaseManager::getInstance().executeQuery(sqlUpdateFechaExpiracion, "Error al actualizar la fecha de expiración del boleto en la base de datos.", "Fecha de expiración del boleto actualizada exitosamente");
}

void DatabaseManager::deleteBankCard(const std::string &tarjetaId)
{
    std::string sqlDelete = "DELETE FROM TARJETASBANCARIAS WHERE ID = '" + tarjetaId + "';";
    DatabaseManager::getInstance().executeQuery(sqlDelete, "Error al eliminar la tarjeta bancaria de la base de datos.", "Tarjeta bancaria eliminada exitosamente");
}
