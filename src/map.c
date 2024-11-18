#include "keyboard.h"
#include "screen.h"
#include "timer.h"
#include "map.h"
#include <stdio.h>


#define MAP_WIDTH 55
#define MAP_HEIGHT 21
#define NUM_MAPS 3
#define MAX_ENEMIES 8
#define MAX_AMMO 5
#define PLAYER_MAX_HEALTH 5
#define ENEMY_RESPAWN_INTERVAL 2
#define ENEMY_COOLDOWN_PERIOD 5
#define MAX_CLIPS 2
#define DROP_CHANCE 20
#define COMBO_HUD_X 0
#define COMBO_HUD_Y MAP_HEIGHT + 1

#define COLOR_WALL YELLOW
#define COLOR_FLOOR LIGHTGRAY
#define COLOR_PLAYER GREEN
#define COLOR_ENEMY MAGENTA
#define COLOR_ENEMY_HIT WHITE
#define COLOR_ATTACK CYAN
#define COLOR_DROP_AMMO BLUE
#define COLOR_DROP_HEALTH RED
#define COLOR_DOOR LIGHTGREEN
#define COLOR_COMBO1 CYAN
#define COLOR_COMBO2 GREEN
#define COLOR_COMBO3 MAGENTA

extern int ghostMode;

char maps[NUM_MAPS][MAP_HEIGHT][MAP_WIDTH] = {
    {
        "#######################################################",
        "#######################################################",
        "##                 #                   #              #",
        "##    #            #                   #              #",
        "##    #                                               #",
        "##    #######   #############          ################",
        "##    #                     #                         #",
        "##    #######               #                         #",
        "##                                                    #",
        "##      ########   #####    ##########            #####",
        "##                 #                              #   #",
        "##      ########   #                              #   #",
        "##      #                                    ######   #",
        "##      #      #########    #   ####         #        #",
        "##             #            #      #         #  #######",
        "##             #            #      #         #        #",
        "##                                 #         #######  #",
        "#######    ####     ###    ###     ###       #        #",
        "##                  #        #               #  #######",
        "##                  #        #                        #",
        "#######################################################"
    },
    {
        "#######################################################",
        "#######################################################",
        "##                                                    #",
        "#######    ####    ####           ####    ####    #####",
        "##                                                    #",
        "##         ########                  ########         #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##                                                    #",
        "##                                                    #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##         ########                  ########         #",
        "##                        #  #                        #",
        "##                       ##  ##                       #",
        "##                      ###  ###                      #", 
        "#######################################################"
    },
    {
        "#######################################################",
        "#######################################################",
        "##                                                    #",
        "##                                                    #",
        "##         #########                        #         #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##                                                    #",
        "##                                                    #",
        "##                                                    #",
        "##                                                    #",
        "##                                                    #",
        "##                                                    #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##         #                                #         #",
        "##         #                         ########         #",
        "##                                                    #",
        "##                                                    #",
        "#######################################################"
    }
};

void screenDrawMap(int mapIndex) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = maps[mapIndex][y][x];
            screenGotoxy(x, y);

            switch(cell) {
                case '#':
                    screenSetColor(COLOR_WALL, BLACK);
                    if (ghostMode) {
                        printf("░");
                    } else {
                        printf("▓");
                    }

                    break;
                default:
                    screenSetColor(COLOR_FLOOR, BLACK);
                    printf(" ");
                    break;
            }
        }
    }
    screenSetColor(WHITE, BLACK);
    fflush(stdout);
}
