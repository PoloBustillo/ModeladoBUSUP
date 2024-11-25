class Cuenta
{
public:
    Cuenta(double saldoInicial = 0.0)
        : saldo(saldoInicial), idBoletoActual(0)
    {
        id = Utils::generateUUID();
    }

    double getSaldo() const { return saldo; }
    int getIdBoletoActual() const { return idBoletoActual; }
    const std::vector<TarjetaBancaria> &getTarjetasBancarias() const { return tarjetasBancarias; }
    const std::vector<Boleto> &getBoletos() const { return boletos; }
    const std::string &getId() const { return id; }

    void setSaldo(double nuevoSaldo) { saldo = nuevoSaldo; }
    void setIdBoletoActual(int nuevoId) { idBoletoActual = nuevoId; }
    void setTarjetasBancarias(const std::vector<TarjetaBancaria> &nuevasTarjetas) { tarjetasBancarias = nuevasTarjetas; }
    void setBoletos(const std::vector<Boleto> &nuevosBoletos) { boletos = nuevosBoletos; }
    void setId(const std::string &nuevoId) { id = nuevoId; }

    void mostrar() const
    {
        std::cout << "====================================================================\n"
                  << "|                                                                  |\n"
                  << "|                           \033[1;35mCuenta Info\033[0m                            |\n"
                  << "|                                                                  |\n"
                  << "====================================================================\n"
                  << "\033[1;35mID de la Cuenta:\033[0m \033[1;33m" << id << "\033[0m\n"
                  << "\033[1;35mSaldo:\033[0m \033[1;33m$" << saldo << "\033[0m\n"
                  << "\033[1;35mID del Boleto Actual:\033[0m \033[1;33m" << idBoletoActual << "\033[0m\n"
                  << "====================================================================\n";
    }

    void mostrarTarjetas() const
    {
        std::cout << "====================================================================\n"
                  << "|                                                                  |\n"
                  << "|                           \033[1;35mTarjetas Info\033[0m                          |\n"
                  << "|                                                                  |\n"
                  << "====================================================================\n";
        for (const auto &tarjeta : tarjetasBancarias)
        {
            std::cout << "\033[1;35mID de la Tarjeta:\033[0m \033[1;33m" << tarjeta.getId() << "\033[0m\n"
                      << "\033[1;35mNúmero:\033[0m \033[1;33m" << tarjeta.getNumero() << "\033[0m\n"
                      << "\033[1;35mFecha de Expiración:\033[0m \033[1;33m" << tarjeta.getFechaExpiracion() << "\033[0m\n"
                      << "--------------------------------------------------------------------\n";
        }
        std::cout << "====================================================================\n";
    }
    void mostrarBoletos() const
    {
        std::cout << "====================================================================\n"
                  << "|                                                                  |\n"
                  << "|                           \033[1;35mBoletos Info\033[0m                           |\n"
                  << "|                                                                  |\n"
                  << "====================================================================\n";
        for (const auto &boleto : boletos)
        {
            std::cout << "\033[1;35mID del Boleto:\033[0m \033[1;33m" << boleto.getId() << "\033[0m\n"
                      << "\033[1;35mFecha de Expiración:\033[0m \033[1;33m" << boleto.getFechaExpiracion() << "\033[0m\n"
                      << "\033[1;35mEstado:\033[0m \033[1;33m" << boleto.statusToString() << "\033[0m\n";
            std::cout << "--------------------------------------------------------------------\n";
        }
        std::cout << "====================================================================\n";
    }
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

    Boleto comprarBoleto()
    {
        std::time_t t = std::time(nullptr);
        std::tm *tm = std::localtime(&t);
        tm->tm_mday += Config::getInstance().getExpiration();
        std::mktime(tm);
        char buffer[11];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
        std::string fechaExpiracion(buffer);
        Boleto nuevoBoleto(fechaExpiracion, StatusBoleto::Nuevo);
        boletos.push_back(nuevoBoleto);

        // Insertar el boleto en la base de datos
        std::string sqlInsertBoleto = "INSERT INTO BOLETOS (ID, EXPIRACION, STATUS, CUENTA) VALUES ('" +
                                      nuevoBoleto.getId() + "', '" + fechaExpiracion + "', 'nuevo', '" + id + "');";
        char *zErrMsg = 0;
        int rc = sqlite3_exec(DatabaseManager::getInstance().getDB(), sqlInsertBoleto.c_str(), nullptr, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            Utils::printError("Error al insertar el boleto en la base de datos: " + std::string(zErrMsg));
            sqlite3_free(zErrMsg);
            throw std::runtime_error("Error al insertar el boleto en la base de datos.");
        }
        else
        {
            Utils::printSuccess("Boleto insertado en la BD.");
            Utils::printSuccess("Boleto comprado exitosamente.");
            return nuevoBoleto;
        }
    }

private:
    std::string id;
    double saldo;
    int idBoletoActual;
    std::vector<TarjetaBancaria> tarjetasBancarias;
    std::vector<Boleto> boletos;
};
