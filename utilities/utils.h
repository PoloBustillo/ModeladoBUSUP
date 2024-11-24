#include <chrono>
#include <thread>
#include <unistd.h>
#pragma comment(lib, "rpcrt4.lib")
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

class Utils
{
public:
    static void mostrarBarraDeCarga(int duracionSegundos)
    {
        int anchoBarra = 30;
        std::cout << Utils::centerText("Inicia la base de datos...!");
        std::cout << "\033[5;34m";
        for (int i = 0; i <= anchoBarra; ++i)
        {
            std::cout << "\r[";
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
        std::cout << "\033[1;32m";
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("=====================================================\n");
        std::cout << "\033[0m";
    }

    static std::string generateUUID()
    {
        uuid_t uuid;
        uuid_generate(uuid);
        char uuid_str[37];
        uuid_unparse(uuid, uuid_str);
        return std::string(uuid_str);
    }

    static void printMenu(const std::vector<std::string> &items)
    {
        std::cout << "\033[1;34m";
        std::cout << Utils::centerText("=====================================================\n");
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("          \033[1;44;97mMENÚ\033[0m            \n");
        std::cout << "\033[1;34m";
        std::cout << Utils::centerText("|                                                   |\n");
        std::cout << Utils::centerText("=====================================================\n");

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
