class Boleto
{
public:
    Boleto(const std::string &fechaExpiracion, StatusBoleto status)
        : id(Utils::generateUUID()), fechaExpiracion(fechaExpiracion), status(status) {};
    Boleto(const std::string &fechaExpiracion, StatusBoleto status, const std::string &activeDate)
        : id(Utils::generateUUID()), fechaExpiracion(fechaExpiracion), status(status), activeDate(activeDate) {}

    const std::string &getActiveDate() const { return activeDate; }
    void setActiveDate(const std::string &nuevaActiveDate) { activeDate = nuevaActiveDate; }

    const std::string &getId() const { return id; }
    const std::string &getFechaExpiracion() const { return fechaExpiracion; }
    StatusBoleto getStatus() const { return status; }

    void setId(const std::string &nuevoId) { id = nuevoId; }
    void setFechaExpiracion(const std::string &nuevaFecha) { fechaExpiracion = nuevaFecha; }
    void setStatus(StatusBoleto nuevoStatus) { status = nuevoStatus; }

    std::string statusToString() const
    {
        switch (status)
        {
        case StatusBoleto::Activo:
            return "Activo";
        case StatusBoleto::Nuevo:
            return "Nuevo";
        case StatusBoleto::Usado:
            return "Usado";
        default:
            return "Desconocido";
        }
    }

    void mostrar() const
    {
        std::cout << "====================================================================\n"
                  << "\033[1;35mID del Boleto:\033[0m \033[1;33m" << id << "\033[0m\n"
                  << "\033[1;35mFecha de Expiración:\033[0m \033[1;33m" << fechaExpiracion << "\033[0m\n"
                  << "\033[1;35mEstado:\033[0m \033[1;33m" << statusToString() << "\033[0m\n"
                  << "\033[1;35mFecha Activación:\033[0m \033[1;33m" << activeDate << "\033[0m\n"
                  << "--------------------------------------------------------------------\n";
    }

private:
    std::string id;
    std::string fechaExpiracion;
    StatusBoleto status;
    std::string activeDate;
};
