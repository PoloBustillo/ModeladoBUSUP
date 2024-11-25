class Transaccion
{
public:
    Transaccion(double monto, StatusTransaccion status, const std::string &usuario, const std::string &tarjetaId)
        : monto(monto), status(status), usuario(usuario), tarjetaId(tarjetaId)
    {
        id = Utils::generateUUID();
    }

    const std::string &getId() const { return id; }
    double getMonto() const { return monto; }
    StatusTransaccion getStatus() const { return status; }
    const std::string &getUsuario() const { return usuario; }
    const std::string &getTarjetaId() const { return tarjetaId; }

    void mostrar() const
    {
        std::cout << "====================================================================\n"
                  << "|                                                                  |\n"
                  << "|                       \033[1;35mTransacción Info\033[0m                            |\n"
                  << "|                                                                  |\n"
                  << "====================================================================\n"
                  << "\033[1;35mID de la Transacción:\033[0m \033[1;33m" << id << "\033[0m\n"
                  << "\033[1;35mMonto:\033[0m \033[1;33m$" << monto << "\033[0m\n"
                  << "\033[1;35mEstado:\033[0m \033[1;33m" << (status == StatusTransaccion::Abono ? "Abono" : status == StatusTransaccion::Tarjeta ? "Tarjeta"
                                                                                                                                                   : "Compra")
                  << "\033[0m\n"
                  << "\033[1;35mUsuario:\033[0m \033[1;33m" << usuario << "\033[0m\n"
                  << "\033[1;35mID de la Tarjeta:\033[0m \033[1;33m" << tarjetaId << "\033[0m\n"
                  << "====================================================================\n";
    }

private:
    std::string id;
    double monto;
    StatusTransaccion status;
    std::string usuario;
    std::string tarjetaId;
};
