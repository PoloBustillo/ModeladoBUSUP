class Cuenta;
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
            Usuario usuarioFromDB = DatabaseManager::getInstance().getUser(matricula);
            if (usuarioFromDB.getPassword() == password)
            {
                Utils::printSuccess("¡Inicio de sesión exitoso!");
                nombre = usuarioFromDB.getNombre();
                telefono = usuarioFromDB.getTelefono();
                cuenta = usuarioFromDB.getCuenta();
                return true;
            }
        }
        catch (const std::runtime_error &e)
        {
            Utils::printError(e.what());
            throw std::runtime_error("Error al iniciar sesión.");
        }
        return false;
    }

    void eliminarCuenta()
    {
        std::string sqlDelete = "DELETE FROM USERS WHERE MATRICULA = '" + matricula + "';";
        DatabaseManager::getInstance().executeQuery(sqlDelete, "Error al eliminar la cuenta", "Cuenta eliminada exitosamente");
    }

private:
    std::string matricula;
    std::string nombre;
    std::string password;
    std::string telefono;
    Cuenta cuenta;
};