#include "screen.h"
#include "keyboard.h"
#include "timer.h"
#include "characters.h"
#include <unistd.h>


const char masks[3][38][150] = {
    {
        "\033[35m",  // Magenta para a primeira arte
        "                                                       #*                                                                                        ",
        "                                                        #+*   #++                                                                                ",
        "                                                     #  %*=*  #++                                                                                ",
        "                                                     %*=+%#*= %*++                                                                               ",
        "                                                  %***+=*#*++#*++ %#+#                                                                           ",
        "                                                  %##**##**###*++%#*=                                                                            ",
        "                                                  %%*##*#**###*+*#*= %                                                                           ",
        "             %%#=**#%+                         @@%%%%##%#*****#***+#*-                                                                           ",
        "           @*.-*+=--:-.=                    @@%##**%%#*+*+*##****+***-                                                                           ",
        "           %%+:+*==:-+-.+                 %%%%**=+*##****++*##**##+*=*                          SELECIONE SEU PERSONAGEM!                        ",
        "           %%%%@%@%%%##*:*               @%%#*###**##**#*+*#*%#*##**+-:                                                                          ",
        "            %%%%@@%%%%@#+:=             %%%#*#*****+++==*#***##**#*+*+-                                                                          ",
        "                @@%@     +-+           @%#*#%*:=**##%%%##+*#**%******+=                                                                          ",
        "                          =-=        @%%###%%+--=**#%  ##%%%%%##*#***=+                    Q - BLOCO DO GALO: investida violenta!                ",
        "                           *--*     @%%#*+##%%#==**+#%##%%%%%####++++*                                                                           ",
        "                            %-=*    @%#*+#*#%**#+*#****#####%%##%#*++-                                                                           ",
        "                              =-+   %%%****#%*#*#**####*###****++=++*=+                              GALO DA MADRUGADA                           ",
        "                               *:*  %%#***%%%*%#%#*****%%####***=--::::+                   O Galo das madrugas tem mais recursos                 ",
        "                                %-=%%%#**##*%*##%%%#*#%%#*######*=*==-:-*                      para sobreviver à longa noite                     ",
        "                                  +-%#*+*+*%##*#%%%#%%%%#%%%%*####%%*+-:-                              (+2 AMMO CLIP)                            ",
        "                                  %*-***+=###*#%%%%##%%%%%%###%%%#%  %*-:                                                                        ",
        "                                @%#**-+#**%*%#%+*##***######+*%###%    **                                                                        ",
        "                              @%%%%#%#==*#%#%#*+#%%#+*####**####%#                             ENTER PARA SELECIONAR O GALO                      ",
        "                         *+===+--#+=*#+==*##++#+*%%%#*****#*%###**#                                                                              ",
        "                       #*#*==*+--*=--+#%+:*%%**###**#%%###%%%%#####%                            A OU D PARA MUDAR A SELEÇÃO                      ",
        "                      +###%*-=*%*---==-=#*-+*%####***#*%##%%#****%++==*%                                                                         ",
        "                     *##*##%*=-*%%%+-:--=*#--*###%%#***++###**+###=----==+#                                                                      ",
        "                    *#******%+=-%%%###*-:-*%=:**#++#%*##++*#*###%#***-=+==+=**                                                                   ",
        "                   +#***##*=##*-#%#*-=*%%#=-*+:=+==-=+%%#**#*#%*%%%%-+--=+*=+=*                                                                  ",
        "                  *#**++++*++##=#%#=-=*+*=-=%#*:-=----=+#%#*###=*%%*=#+-::==*--+                                                                 ",
        "                 *##***#*#+--*%+#%#=-=+-----=+##=-+=--:--+####+-=#%=++:=+--+=-=+*                                                                ",
        "                *%%%%%####%#+=##%%#**#**%%%+*-=%%+-*##*==--+##=---#=*-::++-=--+--*                                                               ",
        "                *%%%%##+**+%#=*%%%%#%%++#%%*=--%%%*-+***++:-=*=:=*+=+---=#+=--+*-*                                                               ",
        "                ######%##+-+%##%%%%%%%%%%%##+-=%%%%##*--+#=:----=%*-=::::+#=:#*--*                                                               ",
        "              #%%%%%%%%%%%+-*%%%%%#%%%%%%%%%#*+%%%%#**#+---=--+=-=%#-::-=*####--+*                                                               ",
        "              #%%#+=+*##%%%*+#%%%%%%%%%%%%%%#%#%%%#****#*==++==+--%*=:--=#*###*#+*                                                               ",
        "\033[0m",  // Reset color
    },
    {
        "\033[32m",  // Verde para a segunda arte
        "                                    %%%                                                                                                          ",
        "                                  %%%%%%%           %%#%%           %%%%%                                                                        ",
        "                                  %%%#*+*#%    %###**++++***#%%%% #***#%%%                                                                       ",
        "                                  %#*####%%%%##***+++=+*+++**+++*#*###%%%%                                                                       ",
        "                                  #======++***++*#*+++**********++*+==+=+#                                                                       ",
        "                                   *====++*++==++**+#########**++*==+===+%                                                                       ",
        "                                   #*+******=+++***#*#*###########+++*++#%                                                                       ",
        "                                   %#*####*#++*==+*#####%%      #*##*+*###                                                                       ",
        "                                  #*+**##*    **++***##%%        #*##=+*=*#                                                                      ",
        "                                =%**#*####   #+++-=**#*####%    ######++=*%%                    SELECIONE SEU PERSONAGEM!                        ",
        "                                *+##**##**####+++++++***##### %###*=*##*=+#%                                                                     ",
        "                               ##+**#++*###**++++==+==--+*+##***##*+#++*++#%                                                                     ",
        "                               %**#**++*++*+*+=+=+++++=++=+######*%#*+**==*#                                                                     ",
        "                               ##+*+++*#*+*+=+=---=*+-==++=-+++##==#*=#*==*#               Q - RUGIDO DO REI: Atordoa inimigos!                  ",
        "                               ##+**++****+=++**=*#++===++++*++++=*#=+#++=*%                                                                     ",
        "                               *#=+**++*##*+++++#*=++=++++++*#******=##+*+*#                                                                     ",
        "                               **+**##**#*##*#########++#***#++*+++###*#%###                         LEÃO DO CAZÁ                                ",
        "                                **###%#*#***.:*+*#**##-.-+*##**#%####**###%%              O Grande Leão tem a maior resistência                  ",
        "                                %#++*#####*#:.==*####**.=*+#+=*#*==+=*####%                     e força de todo o Recife.                        ",
        "                                 ###*++***#*=:#+*#**#**:++##*###*##*+%###%                             (+2 VIDA)                                 ",
        "                                *=*#***+**###-#+****#**#+*###+**+=+=+*#*+==+==+=+#                                                               ",
        "                               #+++*########+#%#=--=*=+*-*##*+====++*#+====-=*==+=                                                               ",
        "                               ++++++#####*####=-==-++=+=##%###**#%#**+==+++#-*==-             ENTER PARA SELECIONAR O LEÃO                      ",
        "                            +=+#*++++*#%##**#%%*=+=++--+#%%#=+#%%%#*++*++++++==---*                                                              ",
        "                         +++==++++******##*##*#==+-=+==.=###+*%#*=#--=#+++++++=---#             A OU D PARA MUDAR A SELEÇÃO                      ",
        "                       ++*+*++++#*++###**#####**--==+++=*##**###+**--=+**++++++---+                                                              ",
        "                      +#=+++##+#++++##**#%##%%#+=+*#%%#%#####%%*=+----=*#*++++++--=                                                              ",
        "                    =*-++++++###++++#***#%%%%%%#*#***#*=#%%%%%%#==-----=-++++++++--                                                              ",
        "                  ++=+++++++**+***++*#***###%%%%%%%%%##*%%%%%*##==-----=-++++++++=-=                                                             ",
        "                *++**++++**++##*++**##***#%#%%%%%%##%%%%%%%%%+*#=------==-++++++++--                                                             ",
        "                ##*+++++++*#*+##*+++##*++#%#%##%%%#*#%%%%#%%#+=*==--------=+++++++=-+                                                            ",
        "               %##%%%*+++#++##*#**+++#*++*##%%%%%%#*#%%%#==##+-*==----=----++++++++--                                                            ",
        "              %#++++**+++++**##*##+++*#*+*##%%%%%%#*#%%%=---==-=*=---------++++++++==+                                                           ",
        "             %%##%%%##+++##*+#%%*%#*+++*++#%%%%%%%#+*##=----==--##+--------+++++++++==                                                           ",
        "             %#*++++++*#**%#*+=---=##+++*+#%%%%%%%#+*#+------=--+#*=-------*+++++++++==                                                          ",
        "            %####%##*++++**+-------=##*++++*%%%%%%#+*#=----------+#=-------*++++++++++=*                                                         ",
        "\033[0m",  // Reset color
    },
    {
        "\033[34m",  // Azul para a terceira arte
        "                 @@@%%@                                                                                                                          ",
        "                 @@@%+*@         +==+                                 *+:=                                                                       ",
        "                 @@@@++@       *+=::-+                              ====+=                                                                       ",
        "                 @@@@+*@      *+-:-::-   %     %##**=-::+**:=#*+=+=-:-=+--                                                                       ",
        "                 %@@@#*#%     ++::::::-*##*##*##+-:::::::::--+*%#=::::-:-=-                                                                      ",
        "                  @@@%+#@      *-::-:.-*##*+#+==----:::-=-:=%%**-::::--:-=*                                                                      ",
        "                  @@@@+*@      +-::::=+#-+***=+=-=*-::::::=++##*-::--::----                                                                      ",
        "                  @@@@+*@      ---:-*=*#+==-=+:-::::==--:--==**@+:--------#                                                                      ",
        "                  @@@@##%       =-=+%%#=-=-=:::-::-+-:-----:=+#%#=:=----=*                                                                       ",
        "                  %@@@%+#@       ++#%*=+==+--::---:::-#*#++---+*=*%%*+=-*%                      SELECIONE SEU PERSONAGEM!                        ",
        "                   @@@@=*@        #%@%#+*#-=---:=+===-=-:-+#*=**+**##*#%@                                                                        ",
        "                   @@@@+#@        #%#*+#%+*::::--*#=:+=-::--##+==-=###%                                                                          ",
        "                   @@@@*#@       +==-:*%+++=-:-==+##=:::=+#+%#=--+#%*%%                                                                          ",
        "                    @@@%%%        %#+=##*=-:::::-=*%+=*%@@%+%%#+=*+*+#%%                   Q - PASSO FANTASMA: Atravessa paredes!                ",
        "                    @@@%%%%       %##%%+====-:--::+#++%  @%+#@#+=++*##%                                                                          ",
        "                    @@@@%@@%@    @%%%@#=+*----=:---#%#%@@@#+###*+*#**%@                                                                          ",
        "                  @%%@@@*%@@%    @%#*#%+++=-=-:::=+=*#*-=:=#%*#==+**%%                               TIMBU ALVIRRUBRO                            ",
        "                  @%%@@@=+%%@    @#*#%%=+==--:-::=++**+#%%%+==::=**#%%                          O Timbu é sorrateiro e tem                       ",
        "                  @@@@@@#*%@@     #*+*#+-==+-=-::-=+++===+=--===*#%%%%                        uma ótima habilidade com armas.                    ",
        "                   @@@@@%+#@#     %#*=*+*=--::::::==-=-==--=++******                             (PODE ATIRAR EM DIAGONAL)                       ",
        "                   @@@@@@+%@#@    %%#++=+=--=-:::::----==++**=*###%                                                                              ",
        "                 +=*@@@@@%%%%*:....#%#+===--=::-=---=++*#+****#%@%                                                                               ",
        "                 +-=%@@@@@*%*#+..*++%%*+===-:::=**#*****+++**#%%%                              ENTER PARA SELECIONAR O TIMBU                     ",
        "                ===+%@@@@@##++*::--*#*+**==-::=+===+#%%#*#%#%#*+=---::::::::--=++=                                                               ",
        "                --=-.*@@@@%%%+*:..-*#+======-==++==-*##%%%#*==----:::::::::::----++=            A OU D PARA MUDAR A SELEÇÃO                      ",
        "                -=+=-#@@@@@+*#+:..:+#*=*=====*===-++#%%%*=-:-::-=-=-::::::::---=-=*+                                                             ",
        "             -=+====-*%%%@@##+#*==*%@##%#**++++++++%@@#+:-:=%-::--:::::::::--==-*#%+                                                             ",
        "           -=====--=-+#*=#@%%*+#=*%@@@%=++-:+***#%@@%+=:-.:@@#-==-::::::::-=-+-===***                                                            ",
        "          ==--=#%#++-+**=-+%*%*+*@%%%%#+--=*@@@%@#*#=-::..+@%*+---::::::::--=+=-==-=*#                                                           ",
        "          +=--=#@@%%##%*=-=%*@%##@#=:::==%@@%*##@%+-:-:*..@@%-::--:::::::===#==-=*###@                                                           ",
        "          -=--+#@@%%%#*=--=%%@@%#%+-::::*+===+*-=---:--..*@%-::::-:::::+%#=#%*%%+-*###%                                                          ",
        "          -+---+*%%%#=---*%@@@@#@%+-:::==-----+*-::::-+=.@@%:::::--::-%%%#%@%%@#=-=*%@@@                                                         ",
        "           -+---=------=#@@@#@%%@%*-::-+=-===:-+::--=*-.##*%=--+%%%@@@@%@@@@%##%%#+-=%@@                                                         ",
        "            =-=-------=+%@@@%@%+#@*-:-+-:::::-=-:++---=:::-+#-=*#%%%%%#@@@@@#=#++*%@%%@@@                                                        ",
        "            =+-------=--#@@@@%@%@%*::*-::::::--=%+::::::::--#@@@@@%%@%@@@@@@#=+%%#=*%@@@@                                                        ",
        "            -+-------+++%@@@@##=*%%+=-::::::=+%@#---==::---==@@@@@%%%@@@@@@@%*==+%@#+*@@@@                                                       ",
        "\033[0m"  // Reset color
    }
};

void print_mask(int mask_index) {
    if (mask_index < 0 || mask_index >= 3) {
        printf("Índice inválido! Escolha um índice entre 0 e 2.\n");
        return;
    }

    for (int i = 0; i < 37; i++) {
        printf("%s\n", masks[mask_index][i]);
    }

    fflush(stdout);
}

// Função principal que controla a navegação entre as máscaras
int navigate_masks() {
    int current_mask = 0;
    char input;

    // Exibe a máscara inicial
    print_mask(current_mask);

    // Espera pela entrada do jogador para mudar a máscara
    while (1) {
        // Lê a tecla pressionada
        if (keyhit()) {
            input = readch();
            if (input == 'a' || input == 'A') {
                // Navega para a máscara anterior (circular)
                current_mask = (current_mask - 1 + 3) % 3;
                screenClear();
                print_mask(current_mask);  // Atualiza a máscara na tela
            } else if (input == 'd' || input == 'D') {
                // Navega para a próxima máscara (circular)
                current_mask = (current_mask + 1) % 3;
                screenClear();
                print_mask(current_mask);  // Atualiza a máscara na tela
            } else if (input == '\n' || input == '\t') {
                // Encerra a função e retorna a máscara escolhida
                break;
            }
        }

        usleep(100000); // Espera um pouco para evitar sobrecarga de CPU
    }
    screenDestroy();
    screenHideCursor();

    return current_mask;  // Retorna a máscara escolhida
}
