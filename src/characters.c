#include "screen.h"
#include "keyboard.h"
#include "timer.h"
#include "characters.h"
#include <unistd.h>


const char masks[3][38][150] = {
    {
        "\033[35m",
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
        "                          =-=        @%%###%%+--=**#%  ##%%%%%##*#***=+                              GALO DA MADRUGADA                           ",
        "                           *--*     @%%#*+##%%#==**+#%##%%%%%####++++*                                                                           ",
        "                            %-=*    @%#*+#*#%**#+*#****#####%%##%#*++-                                                                           ",
        "                              =-+   %%%****#%*#*#**####*###****++=++*=+                    Q - BLOCO DO GALO: investida violenta!                ",
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
        "\033[0m",
    },
    {
        "\033[32m",
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
        "                               ##+*+++*#*+*+=+=---=*+-==++=-+++##==#*=#*==*#                           LEÃO DO CAZÁ                              ",
        "                               ##+**++****+=++**=*#++===++++*++++=*#=+#++=*%                                                                     ",
        "                               *#=+**++*##*+++++#*=++=++++++*#******=##+*+*#                                                                     ",
        "                               **+**##**#*##*#########++#***#++*+++###*#%###               Q - RUGIDO DO REI: Atordoa inimigos!                  ",
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
        "\033[0m",
    },
    {
        "\033[34m", 
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
        "                    @@@%%%        %#+=##*=-:::::-=*%+=*%@@%+%%#+=*+*+#%%                              TIMBU ALVIRRUBRO                           ",
        "                    @@@%%%%       %##%%+====-:--::+#++%  @%+#@#+=++*##%                                                                          ",
        "                    @@@@%@@%@    @%%%@#=+*----=:---#%#%@@@#+###*+*#**%@                                                                          ",
        "                  @%%@@@*%@@%    @%#*#%+++=-=-:::=+=*#*-=:=#%*#==+**%%                     Q - PASSO FANTASMA: Atravessa paredes!                ",
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
        "\033[0m"
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
int navigate_masks() {
    int current_mask = 0;
    char input;

    print_mask(current_mask);

    while (1) {
        if (keyhit()) {
            input = readch();
            if (input == 'a' || input == 'A') {
                current_mask = (current_mask - 1 + 3) % 3;
                screenClear();
                print_mask(current_mask);
            } else if (input == 'd' || input == 'D') {
                current_mask = (current_mask + 1) % 3;
                screenClear();
                print_mask(current_mask);
            } else if (input == '\n' || input == '\t') {
                break;
            }
        }

        usleep(100000);
    }
    screenDestroy();
    screenHideCursor();

    return current_mask;
}
