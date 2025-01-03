class Cuenta
{
public:
    Cuenta(double saldoInicial = 0.0)
        : saldo(saldoInicial)
    {
        id = Utils::generateUUID();
    }

    double getSaldo() const { return saldo; }
    std::vector<Boleto> getBoletosActuales() const { return boletosActuales; }
    const std::vector<TarjetaBancaria> &getTarjetasBancarias() const { return tarjetasBancarias; }
    const std::vector<Boleto> &getBoletos() const { return boletos; }
    const std::string &getId() const { return id; }

    void setSaldo(double nuevoSaldo) { saldo = nuevoSaldo; }
    void setBoletosActuales(const std::vector<Boleto> &nuevoId) { boletosActuales = nuevoId; }
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
                  << "\033[1;35mID del Boleto Actual:\033[0m \033[1;33m";
        for (const auto &boleto : boletosActuales)
        {
            std::cout << boleto.getId() << " ";
        }
        std::cout << "\033[0m\n"
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
                      << "\033[1;35mEstado:\033[0m \033[1;33m" << boleto.statusToString() << "\033[0m\n"
                      << "\033[1;35mFecha de Activación:\033[0m \033[1;33m" << boleto.getActiveDate() << "\033[0m\n";
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
    void eliminarTarjeta(const std::string &idTarjeta)
    {
        auto it = std::find_if(tarjetasBancarias.begin(), tarjetasBancarias.end(),
                               [&idTarjeta](const TarjetaBancaria &tarjeta)
                               { return tarjeta.getId() == idTarjeta; });

        if (it != tarjetasBancarias.end())
        {
            tarjetasBancarias.erase(it);
            Utils::printSuccess("Tarjeta eliminada exitosamente.");
        }
        else
        {
            Utils::printError("Tarjeta no encontrada.");
        }
    }
    void comprarBoleto()
    {
        std::string fechaExpiracion = Utils::getDate(10);
        Boleto nuevoBoleto(fechaExpiracion, StatusBoleto::Nuevo);
        boletos.push_back(nuevoBoleto);
        // Insertar el boleto en la base de datos
        std::string sqlInsertBoleto = "INSERT INTO BOLETOS (ID, EXPIRACION, STATUS, CUENTA, ACTIVE_DATE) VALUES ('" +
                                      nuevoBoleto.getId() + "', '" + fechaExpiracion + "', 'nuevo', '" + id + "', NULL);";
        DatabaseManager::getInstance().executeQuery(sqlInsertBoleto, "Error al insertar el boleto en la base de datos", "Boleto comprado exitosamente.");
        nuevoBoleto.mostrar();
    }

    int getTarjetaIndex()
    {
        std::cout << "\033[1;33mSeleccione la tarjeta para comprar boleto:\033[0m\n";
        mostrarTarjetas();
        int tarjetaIndex;
        std::cout << "\033[1;33mIngrese el índice de la tarjeta:\033[0m ";
        std::cin >> tarjetaIndex;
        if (tarjetaIndex < 0 || tarjetaIndex >= tarjetasBancarias.size())
        {

            throw std::runtime_error("Índice de tarjeta inválido.");
        }
        return tarjetaIndex;
    }

private:
    std::string id;
    double saldo;
    std::vector<TarjetaBancaria> tarjetasBancarias;
    std::vector<Boleto> boletos;
    std::vector<Boleto> boletosActuales;
};
;
