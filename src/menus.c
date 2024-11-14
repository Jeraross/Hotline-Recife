#include "screen.h"
#include "menus.h"
#include "keyboard.h"

void displayMenu() {
    screenClear();

    // Array do ASCII art com cada caractere colorido
    char *ascii_art[] = {
        "                           \x1b[32m#####\x1b[0m                                                                  ",
        "                       \x1b[32m#######\x1b[0m                                                                    ",
        "            \x1b[32m######\x1b[0m    \x1b[32m########\x1b[0m       \x1b[32m#####\x1b[0m                                                        ",
        "        \x1b[32m###########/#####\\##### #############\x1b[0m                                                   ",
        "    \x1b[32m############/##########--#####################\x1b[0m                                                ",
        "  \x1b[32m####\x1b[0m         \x1b[32m######################\x1b[0m          \x1b[32m#####\x1b[0m                                              ",
        " \x1b[32m##\x1b[0m          \x1b[32m####\x1b[0m      \x1b[32m##########/##\x1b[0m              \x1b[32m###\x1b[0m                                             ",
        " \x1b[32m#\x1b[0m          \x1b[32m####\x1b[0m        \x1b[32m,-.##/`.#\\####\x1b[0m               \x1b[32m#\x1b[0m                                            ",
        "          \x1b[32m###\x1b[0m         /  |\x1b[33m$\x1b[0m/  |,-. \x1b[32m####\x1b[0m              \x1b[35m___  _____    __    _____      __    __\x1b[0m      ",
        "         \x1b[32m##\x1b[0m           \\_,'\x1b[33m$\x1b[0m\\_,'|  \\\x1b[32m##\x1b[0m       \x1b[35m/\\  /\\  /___\\/__   \\  / /    \\_   \\  /\\ \\ \\  /__/\x1b[0m",
        "         \x1b[32m#\x1b[0m              \\_\x1b[33m$$$$$\x1b[0m`._/  \x1b[32m##\x1b[0m    \x1b[35m/ /_/ / //  //  / /\\/ / /      / /\\/ /  \\/ / /_\\\x1b[0m   ",
        "                          \x1b[33m$$$$$\x1b[0m_/      \x1b[32m#\x1b[0m  \x1b[35m/ __  / / \\_//  / /   / /___ /\\/ /_  / /\\  / //__\x1b[0m  ",
        "                          \x1b[33m$$$$$\x1b[0m        \x1b[32m#\x1b[0m  \x1b[35m\\/ /_/  \\___/   \\/    \\____/ \\____/  \\_\\ \\/  \\__/\x1b[0m",
        "                          \x1b[33m$$$$$\x1b[0m                        \x1b[35m__     __    ___   _____    ___    __\x1b[0m      ",
        "                         \x1b[33m$$$$$\x1b[0m                        \x1b[35m/__\\   /__\\  / __\\  \\_   \\  / __\\  /__/\x1b[0m ",
        "                         \x1b[33m$$$$$\x1b[0m        \x1b[33m___\x1b[0m            \x1b[35m/ \\//  /_\\   / /      / /\\/ / _\\   /_\\\x1b[0m  ",
        "                         \x1b[33m$$$$$\x1b[0m    \x1b[33m_.-'   `-._\x1b[0m       \x1b[35m/ _  \\ //__  / /___ /\\/ /_  / /    //__\x1b[0m     ",
        "                        \x1b[33m$$$$$\x1b[0m   \x1b[33m,'           `.\x1b[0m     \x1b[35m\\/ \\_/ \\__/  \\____/ \\____/  \\/     \\__/\x1b[0m ",
        "                        \x1b[33m$$$$$\x1b[0m  \x1b[33m/               \\\x1b[0m                                       	       ",
        "\x1b[34m~~~~~~~~~~~~~~~~~~~~~~~\x1b[0m\x1b[33m$$$$$\x1b[0m\x1b[34m~~~\x1b[0m\x1b[33m'\x1b[0m\x1b[34m~~~~~~~~~~~~~~~~\x1b[0m\x1b[33m'\x1b[0m\x1b[34m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\x1b[0m",
        "\x1b[34m   ~      ~  ~    ~  ~ \x1b[0m\x1b[33m$$$$$\x1b[0m\x1b[34m  ~   ~       ~          ~     ~     ~   ~       ~          ~    \x1b[0m",
        "\x1b[34m       ~ ~             \x1b[0m\x1b[33m$$$$$\x1b[0m\x1b[34m     ~    ~  ~        ~         ~      ~    ~  ~        ~        \x1b[0m",
        "\x1b[34m  ~            ~     ~ \x1b[0m\x1b[33m$$$$$\x1b[0m\x1b[34m       ~  ~       ~        ~        ~        ~         ~         \x1b[0m",
        "_______________________\x1b[33m$$$$$\x1b[0m_________________________________________________________________",
        "                       \x1b[33m$$$$$\x1b[0m                                                                      ",
        "                       \x1b[33m$$$$$\x1b[0m                                                                      ",
        "                      \x1b[33m$$$$$$$\x1b[0m                                                                     ",
        "                      \x1b[33m$$$$$$$\x1b[0m                    Pressione ENTER para continuar...                ",
        "                     \x1b[33m$$$$$$$$\x1b[0m                                                                     ",
        "                     \x1b[33m$$$$$$$$$\x1b[0m                                                                    ",
        "                     \x1b[33m$$$$$$$$$\x1b[0m                                                                    ",
    };

    // Imprime o ASCII art com as cores configuradas
    for (int i = 0; i < sizeof(ascii_art) / sizeof(ascii_art[0]); i++) {
        printf("%s\n", ascii_art[i]);
    }

    // Espera o usuário pressionar ENTER para continuar
    while (1) {
        if (keyhit()) {
            char key = readch();
            if (key == '\n' || key == '\r') {
                fflush(stdout);
                screenClear();
                break;
            }
        }
    }
}

void displayOpeningArt() {
    screenClear();
    char *ascii_art[] = {
    "⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⠤⠤⠤⢄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡠⢚⣕⣦⣿⣿⣿⣶⣆⡑⢦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣾⣿⠟⠉⠀⠉⠉⠉⠉⠉⠉⠳⣵⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡿⠘⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⡄⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣷⠀⢀⣤⣤⣤⡀⢀⣤⣤⣤⡀⢹⣿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⣹⠀⠹⠟⣿⠿⠂⢘⡿⢿⣿⢷⢸⡉⣳⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠇⠀⠀⠀⡤⠆⠀⢀⠳⣆⡀⠀⠨⢇⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠺⡷⢀⣿⣃⣉⣻⣛⣻⣮⣿⠃⣠⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                              OLÁ AGENTE!",
    "⠀⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⢀⠻⠛⠤⢤⠤⡶⢋⢷⡃⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                       TENHO UMA MISSÃO PRA VOCÊ.",
    "⠀⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⢿⣄⠁⠀⠉⡉⠄⣩⠞⢻⢇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                      Você está em Recife, PE. A cidade vive um caos silencioso.",
    "⠀⠀⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⡀⠻⣗⠦⣴⣴⠞⠉⢠⣿⣦⣑⠢⢄⡀⠀⠀⠀⠀⠀⠀⠀⠀                   Facções criminosas dominam as ruas, especialmente no Recife Antigo.                        ",
    "⠀⠀⠀    ⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⢔⣪⣵⣾⠁⠀⢈⣿⡉⠀⠀⣰⣿⣿⣿⣿⣿⣶⣮⣑⣂⠤⢄⣀⠀⠀⠀                            Sua missão é simples: eliminar os líderes da facção",
    "⠀⠀    ⠀⠀⠀⠀⠀⣀⢤⣒⣭⣶⣾⣿⣿⣿⣿⠀⣴⣿⣿⣷⣄⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣦⣕⢢⡀                                   e acabar com o crime na cidade. ",
    "⠀⠀    ⠀⠀⠀⢠⣾⣷⣿⣿⣿⣿⣿⣿⣿⣿⣿⠈⣿⣿⣿⠃⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⢣                               Use a cidade a seu favor, mas cuidado, ",
    "⠀⠀    ⠀⢀⣮⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣴⣿⣿⣿⣾⣿⣿⣿⣭⣽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣹                      os becos e ruas estreitas escondem mais do que você imagina. ",
    "⠀    ⠀⠀⠀⡜⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟   ",
    "    ⠀⠀⠀⢰⣽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡯                                          WASD para mover",
    "⠀    ⠀⠀⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⡳                                          SPACE para atacar  ",
    "⠀⠀    ⢀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡏⡏                              Se achar uma arma, use I,K,L,J para atirar  ",
    "⠀    ⠀⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⣻⣿⢿⣿⣿⣿⣿⡗⡇                              Você pode atirar em diagonal também (O,U)",
    "    ⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠡⠚⣉⡥⠾⣿⣿⣿⣿⡇⡇                                       T para trocar de arma",
    "    ⠀⣿⣿⣿⣿⣿⣿⣿⠿⠻⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠃⣨⠖⢋⣡⠴⣾⣿⣿⣿⣿⣿⠁                                          R para recarregar",
    "    ⢸⣿⣿⣿⣿⣿⣿⠇⡀⠀⠀⠀⡽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣯⣤⣚⣡⣞⣫⣤⣶⣿⣿⣿⣿⣿⡻⠀          ",
    "    ⣾⣿⣿⣿⣿⣿⣿⣷⠀⠉⠆⠈⢰⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢷⠃⠀                  ",
    "    ⢿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣿⣤⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢏⠏⠀⠀                                      ",
    "    ⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢏⠎⠀⠀⠀                             Boa sorte, Agente. Recife nunca foi fácil.",
    "    ⠈⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡞⠀⠀⠀⠀                                   Pressione ENTER para continuar...    ",
    "⠀    ⠈⠉⠉⠙⠛⢻⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⡇⠀⠀⠀⠀                                    ",
    "⠀⠀⠀    ⠀⠀⠀⢸⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⡇⠀⠀⠀⠀                               ",
    "⠀⠀⠀    ⠀⠀⠀⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢱⠀⠀⠀                                           ",
    "⠀⠀    ⠀⠀⠀⢀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⠆⠀⠀⠀",
    };

    for (int i = 0; i < sizeof(ascii_art) / sizeof(ascii_art[0]); i++) {
        printf("%s\n", ascii_art[i]);
    }

    while (1) {
        if (keyhit()) {
            char key = readch();
            if (key == '\n' || key == '\r') {
                screenClear();
                break;
            }
        }
}
}