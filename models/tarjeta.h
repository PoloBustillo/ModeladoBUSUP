class TarjetaBancaria
{
public:
    TarjetaBancaria(const std::string &numero, const std::string &fechaExpiracion, const std::string &cvv)
        : numero(numero), fechaExpiracion(fechaExpiracion), cvv(cvv)
    {
        id = Utils::generateUUID();
    }

    const std::string &getNumero() const { return numero; }
    const std::string &getFechaExpiracion() const { return fechaExpiracion; }
    const std::string &getCvv() const { return cvv; }
    const std::string &getId() const { return id; }
    void setId(const std::string &nuevoId) { id = nuevoId; }

private:
    std::string id;
    std::string numero;
    std::string fechaExpiracion;
    std::string cvv;
};
