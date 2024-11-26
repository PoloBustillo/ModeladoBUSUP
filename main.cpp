#include <iostream>
#include <vector>
#include <string>
#include <uuid/uuid.h>
#include <regex>
#include "db/sqlite3.h"
#include "utilities/utils.h"
#include "utilities/configs.h"
#include "utilities/enums.h"
#include "db/databasemanger.h"
#include "models/tarjeta.h"
#include "models/boletos.h"
#include "models/transaccion.h"
#include "models/cuenta.h"
#include "models/usuario.h"
#include "db/methods.h"

int main()
{
    sqlite3 *db = DatabaseManager::getInstance().getDB();
    Usuario usuario("", "", "", "", Cuenta());
    int opcion;
    std::string matricula, contrasena;

    while (true)
    {
        std::vector<std::string> menuOptions = {"Iniciar Sesión", "Crear Usuario", "Salir"};
        Utils::printMenu(menuOptions);
        std::string input;
        std::cin >> input;
        if (std::all_of(input.begin(), input.end(), ::isdigit))
        {
            opcion = std::stoi(input);
        }
        else
        {
            opcion = -1; // Invalid option
        }

        if (opcion == 3)
        {
            Utils::printSuccess("Saliendo...");
            exit(0);
        }
        try
        {
            switch (opcion)
            {
            case 1:
                matricula = Utils::validateMatricula();
                std::cout
                    << "\033[1;33mIngrese su contraseña:\033[0m ";
                std::cin >> contrasena;
                usuario.setMatricula(matricula);
                usuario.setPassword(contrasena);
                usuario.iniciarSesion();
                usuario.mostrar();
                break;
            case 2:
                usuario = DatabaseManager::getInstance().createUser();
                usuario.mostrar();
                break;
            default:
                Utils::printError("Opción inválida. Intente de nuevo!");
                break;
            }
            if (!usuario.getMatricula().empty())
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
        std::vector<std::string> menuOptions = {"Abonar a mi cuenta", "Eliminar mi cuenta", "Comprar boleto", "Usar boleto", "Mostrar cuenta",
                                                "Agregar Tarjeta", "Mostrar tarjetas", "Mostrar Transacciones", "Mostrar Boletos", "Eliminar Tarjeta",
                                                "Mostrat boletos activos", "Salir"};
        Utils::printMenu(menuOptions);
        std::string input;
        std::cin >> input;
        if (std::all_of(input.begin(), input.end(), ::isdigit))
        {
            opcion = std::stoi(input);
        }
        else
        {
            opcion = -1; // Invalid option
        }

        if (opcion == 12)
        {
            Utils::printSuccess("Saliendo...");
            exit(0);
        }

        try
        {
            switch (opcion)
            {
            case 1: // Abonar cuenta

                try
                {
                    if (usuario.getCuenta().getTarjetasBancarias().empty())
                    {
                        Utils::printError("No hay tarjetas de crédito.");
                        break;
                    }
                    int tarjetaIndex = usuario.getCuenta().getTarjetaIndex();
                    double cantidad;
                    cantidad = Utils::validatePositiveNumber("\033[1;33mIngrese la cantidad a abonar:\033[0m ");
                    Transaccion transaccion(cantidad, StatusTransaccion::Abono, usuario.getMatricula(), usuario.getCuenta().getTarjetasBancarias()[tarjetaIndex].getId());
                    DatabaseManager::getInstance().registerTransaction(transaccion);
                    usuario.getCuenta().abonar(cantidad);
                }
                catch (const std::runtime_error &e)
                {
                    Utils::printError(e.what());
                    continue;
                }

                break;
            case 2: // Eliminar cuenta
                char confirmacion;
                std::cout << "\033[1;33m¿Está seguro que desea eliminar su cuenta? (s/n):\033[0m ";
                std::cin >> confirmacion;
                if (confirmacion == 's' || confirmacion == 'S')
                {
                    usuario.eliminarCuenta();
                    Utils::printSuccess("Cuenta eliminada exitosamente. Saliendo...");
                    exit(0);
                }
                else
                {
                    Utils::printSuccess("Eliminación de cuenta cancelada.");
                }
                break;
            case 3: // Comprar boleto
                std::cout << "\033[1;33mSeleccione el método de pago:\033[0m\n";
                std::cout << "1. Saldo\n";
                std::cout << "2. Tarjeta\n";
                int metodoPago;
                std::cin >> metodoPago;

                if (metodoPago == 1)
                {
                    if (usuario.getCuenta().getSaldo() >= Config::getInstance().getBoletoCosto())
                    {
                        double newSaldo = usuario.getCuenta().getSaldo() - Config::getInstance().getBoletoCosto();
                        usuario.getCuenta().setSaldo(newSaldo);
                        char *zErrMsg = 0;
                        std::string sqlUpdateSaldo = "UPDATE CUENTAS SET SALDO = " + std::to_string(newSaldo) + " WHERE ID = '" + usuario.getCuenta().getId() + "';";
                        DatabaseManager::getInstance().executeQuery(sqlUpdateSaldo, "Error al actualizar el saldo en la base de datos", "Saldo actualizado");
                        Transaccion transaccion(Config::getInstance().getBoletoCosto(), StatusTransaccion::Compra, usuario.getMatricula(), usuario.getCuenta().getId());
                        DatabaseManager::getInstance().registerTransaction(transaccion);
                        usuario.getCuenta().comprarBoleto();
                    }
                    else
                    {
                        Utils::printError("Saldo insuficiente para comprar un boleto.");
                    }
                }
                else if (metodoPago == 2)
                {
                    if (usuario.getCuenta().getTarjetasBancarias().empty())
                    {
                        Utils::printError("No hay tarjetas de crédito.");
                    }
                    else
                    {
                        try
                        {
                            int tarjetaIndex;
                            tarjetaIndex = usuario.getCuenta().getTarjetaIndex();
                            Transaccion transaccion(Config::getInstance().getBoletoCosto(), StatusTransaccion::Tarjeta, usuario.getMatricula(), usuario.getCuenta().getTarjetasBancarias()[tarjetaIndex].getId());
                            DatabaseManager::getInstance().registerTransaction(transaccion);
                            usuario.getCuenta().comprarBoleto();
                        }
                        catch (const std::runtime_error &e)
                        {
                            Utils::printError(e.what());
                            continue;
                        }
                    }
                }
                else
                {
                    Utils::printError("Método de pago inválido.");
                }
                break;
            case 4: // Usar boleto
            {
                std::string fechaActual = Utils::getDate();
                int boletoIndex;
                std::vector<Boleto> boletos = DatabaseManager::getInstance().getBoletos(usuario.getCuenta().getId());
                usuario.getCuenta().setBoletos(boletos);
                usuario.getCuenta().mostrarBoletos();

                std::cout << "\033[1;33mIngrese el índice del boleto a usar:\033[0m ";
                std::string boletoIndexStr;
                std::cin >> boletoIndexStr;
                while (!std::all_of(boletoIndexStr.begin(), boletoIndexStr.end(), ::isdigit))
                {
                    Utils::printError("Índice de boleto inválido. Ingrese solo dígitos.");
                    std::cout << "\033[1;33mIngrese el índice del boleto a usar:\033[0m ";
                    std::cin >> boletoIndexStr;
                }
                boletoIndex = std::stoi(boletoIndexStr);

                if (boletoIndex < 0 || boletoIndex >= boletos.size())
                {
                    Utils::printError("Índice de boleto inválido.");
                }
                else
                {
                    if (boletos[boletoIndex].getStatus() == StatusBoleto::Nuevo)
                    {
                        boletos[boletoIndex].setStatus(StatusBoleto::Activo);
                        DatabaseManager::getInstance().updateBoletoStatus(boletos[boletoIndex].getId(), StatusBoleto::Activo);
                        boletos[boletoIndex].setActiveDate(fechaActual);
                        DatabaseManager::getInstance().updateBoletoFechaUso(boletos[boletoIndex].getId(), fechaActual);
                        std::array<int, 3> activationTime = Config::getInstance().getActiveTime();
                        std::string newFechaExpiracion = Utils::getDate(activationTime[0], activationTime[1], activationTime[2]);
                        boletos[boletoIndex].setFechaExpiracion(newFechaExpiracion);
                        DatabaseManager::getInstance().updateBoletoFechaExpiracion(boletos[boletoIndex].getId(), newFechaExpiracion);
                        boletos[boletoIndex].mostrar();
                        usuario.getCuenta().setBoletos(boletos);
                    }
                    else
                    {
                        Utils::printError("Boleto no es nuevo");
                    }
                }
                break;
            }
            break;
            case 5:
                usuario.getCuenta().mostrar();
                break;
            case 6:
            {
                std::string numero, fechaExpiracion, cvv;
                std::cout << "\033[1;33mIngrese el número de la tarjeta:\033[0m ";
                std::cin >> numero;
                while (!std::regex_match(numero, std::regex("^(?:4[0-9]{12}(?:[0-9]{3})?|[25][1-7][0-9]{14}|6(?:011|5[0-9][0-9])[0-9]{12}|3[47][0-9]{13}|3(?:0[0-5]|[68][0-9])[0-9]{11}|(?:2131|1800|35[0-9]{3})[0-9]{11})$")))
                {
                    Utils::printError("Ingrese tarjeta válida (Visa o MasterCard).");
                    std::cout << "\033[1;33mIngrese el número de la tarjeta:\033[0m ";
                    std::cin >> numero;
                }
                std::cout << "\033[1;33mIngrese la fecha de expiración (MM/AA):\033[0m ";
                std::cin >> fechaExpiracion;
                while (!std::regex_match(fechaExpiracion, std::regex("^(0[1-9]|1[0-2])\\/([2-9][0-9])$")))
                {
                    Utils::printError("Ingrese una expiración en formato MM/YY.");
                    std::cout << "\033[1;33mIngrese la fecha de expiración (MM/YY):\033[0m ";
                    std::cin >> fechaExpiracion;
                }
                std::cout << "\033[1;33mIngrese el CVV:\033[0m ";
                std::cin >> cvv;
                TarjetaBancaria nuevaTarjeta(numero, fechaExpiracion, cvv);
                DatabaseManager::getInstance().addBankCard(nuevaTarjeta, usuario.getCuenta().getId());
                std::vector<TarjetaBancaria> tarjetas = usuario.getCuenta().getTarjetasBancarias();
                tarjetas.push_back(nuevaTarjeta);
                usuario.getCuenta().setTarjetasBancarias(tarjetas);
                Utils::printSuccess("Tarjeta añadida exitosamente.");
            }
            break;
            case 7:
                usuario.getCuenta().mostrarTarjetas();
                break;
            case 8:
            {
                std::string sqlSelectTransacciones = "SELECT * FROM TRANSACCION WHERE USUARIO = '" + usuario.getMatricula() + "';";
                sqlite3_stmt *stmtTransacciones;
                int rc = sqlite3_prepare_v2(DatabaseManager::getInstance().getDB(), sqlSelectTransacciones.c_str(), -1, &stmtTransacciones, 0);
                if (rc != SQLITE_OK)
                {
                    Utils::printError("Error al preparar la consulta de transacciones: " + std::string(sqlite3_errmsg(DatabaseManager::getInstance().getDB())));
                    throw std::runtime_error("Error al obtener las transacciones de la base de datos.");
                }

                std::vector<Transaccion> transacciones;
                while ((rc = sqlite3_step(stmtTransacciones)) == SQLITE_ROW)
                {
                    std::string idTransaccion = reinterpret_cast<const char *>(sqlite3_column_text(stmtTransacciones, 0));
                    double monto = sqlite3_column_double(stmtTransacciones, 1);
                    std::string statusStr = reinterpret_cast<const char *>(sqlite3_column_text(stmtTransacciones, 2));
                    StatusTransaccion status = (statusStr == "abono") ? StatusTransaccion::Abono : (statusStr == "tarjeta") ? StatusTransaccion::Tarjeta
                                                                                                                            : StatusTransaccion::Compra;
                    std::string usuarioTransaccion = reinterpret_cast<const char *>(sqlite3_column_text(stmtTransacciones, 3));
                    std::string tarjetaId = reinterpret_cast<const char *>(sqlite3_column_text(stmtTransacciones, 4));
                    transacciones.emplace_back(monto, status, usuarioTransaccion, tarjetaId);
                }
                sqlite3_finalize(stmtTransacciones);

                for (const auto &transaccion : transacciones)
                {
                    transaccion.mostrar();
                }
            }
            break;
            case 9:
            {
                std::vector<Boleto> boletos = DatabaseManager::getInstance().getBoletos(usuario.getCuenta().getId());
                usuario.getCuenta().setBoletos(boletos);
                usuario.getCuenta().mostrarBoletos();
            }
            break;
            case 10: // Eliminar Tarjeta
            {
                try
                {
                    if (usuario.getCuenta().getTarjetasBancarias().empty())
                    {
                        Utils::printError("No hay tarjetas de crédito.");
                        break;
                    }
                    int tarjetaIndex = usuario.getCuenta().getTarjetaIndex();

                    std::string tarjetaId = usuario.getCuenta().getTarjetasBancarias()[tarjetaIndex].getId();
                    DatabaseManager::getInstance().deleteBankCard(tarjetaId);
                    usuario.getCuenta().eliminarTarjeta(tarjetaId);
                    Utils::printSuccess("Tarjeta eliminada exitosamente.");
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
            break;
            case 11: // Mostrar boletos activos
            {

                std::vector<Boleto> boletos = DatabaseManager::getInstance().getBoletos(usuario.getCuenta().getId());
                usuario.getCuenta().setBoletos(boletos);
                for (const auto &boleto : boletos)
                {
                    if (boleto.getStatus() == StatusBoleto::Activo)
                    {
                        std::string command = "qrcode-terminal '" + boleto.getId() + "',{small: true}";
                        system(command.c_str());
                        boleto.mostrar();
                    }
                }
            }
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
    DatabaseManager::getInstance().closeDatabase();
    return 0;
}
