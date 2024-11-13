#include "screen.h"
#include "menus.h"
#include "keyboard.h"

void displayMenu() {
    screenClear();
        char *ascii_art[] = {
        "                           #####                                                                  ",
        "                       #######                                                                    ",
        "            ######    ########       #####                                                        ",
        "        ###########/#####\\#####  #############                                                   ",
        "    ############/##########--#####################                                                ",
        "  ####         ######################          #####                                              ",
        " ##          ####      ##########/##              ###                                             ",
        " #          ####        ,-.##/`.#\\####              #                                            ",
        "          ###         /  |$/  |,-. ####              ___  _____    __    _____      __    __      ",
        "         ##           \\_,'$\\_,'|  \\##       /\\  /\\  /___\\/__   \\  / /    \\_   \\  /\\ \\ \\  /__/",
        "         #              \\_$$$$$`._/  ##    / /_/ / //  //  / /\\/ / /      / /\\/ /  \\/ / /_\\   ",
        "                          $$$$$_/      #  / __  / / \\_//  / /   / /___ /\\/ /_  / /\\  / //__  ",
        "                          $$$$$        #  \\/ /_/  \\___/   \\/    \\____/ \\____/  \\_\\ \\/  \\__/",
        "                          $$$$$                        __     __    ___   _____    ___    __      ",
        "                         $$$$$                        /__\\   /__\\  / __\\  \\_   \\  / __\\  /__/",
        "                         $$$$$        ___            / \\//  /_\\   / /      / /\\/ / _\\   /_\\  ",
        "                         $$$$$    _.-'   `-._       / _  \\ //__  / /___ /\\/ /_  / /    //__     ",
        "                        $$$$$   ,'           `.     \\/ \\_/ \\__/  \\____/ \\____/  \\/     \\__/",
        "                        $$$$$  /               \\                                       	       ",
        "~~~~~~~~~~~~~~~~~~~~~~~$$$$$~~~'~~~~~~~~~~~~~~~~`~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",
        "   ~      ~  ~    ~  ~ $$$$$  ~   ~       ~          ~     ~     ~   ~       ~          ~    ",
        "       ~ ~             $$$$$     ~    ~  ~        ~         ~      ~    ~  ~        ~        ",
        "  ~            ~     ~ $$$$$       ~  ~       ~        ~        ~        ~         ~         ",
        "_______________________$$$$$_________________________________________________________________     ",
        "                       $$$$$                                                                      ",
        "                       $$$$$                                                                      ",
        "                      $$$$$$$                    Pressione ENTER para jogar...                    ",
        "                     $$$$$$$$$                                                                    ",
    };

    for (int i = 0; i < sizeof(ascii_art) / sizeof(ascii_art[0]); i++) {
        screenGotoxy(0, i);
        screenSetColor(WHITE, BLACK);
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