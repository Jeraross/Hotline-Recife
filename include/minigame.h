#ifndef MINIGAME_H
#define MINIGAME_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define MAPS_NUM 1
#define MAP_H 16
#define MAP_W 80
#define MAX_CLOUDS 20
#define MAX_TRAFFIC_LANES 21
#define MAX_CAR_ENEMIES 15
#define MAP_START_Y 1

#define DEFAULT_MOVE_INTERVAL 600
#define TICK_RATE 10

#define SPEED_MOON 2
#define SPEED_CLOUDS 20
#define SPEED_CAR_ENEMIES 60
#define SPEED_TRAFFIC_LANES 50

#define COLOR_F DARKGRAY
#define COLOR_SKY BLACK
#define COLOR_CLOUDS WHITE
#define COLOR_STARS LIGHTGRAY
#define COLOR_PLAYER GREEN
#define COLOR_ENEMY1 RED
#define COLOR_ENEMY2 BROWN
#define COLOR_ENEMY3 BLUE
#define COLOR_ENEMY4 MAGENTA
#define COLOR_ENEMY5 LIGHTRED
#define COLOR_ENEMY6 CYAN
#define COLOR_TRAFFIC_LANE YELLOW
#define COLOR_MOON YELLOW

extern const int enemy_colors[];
#define NUM_ENEMY_COLORS 6

// Estruturas
typedef struct {
    int x, y;
} Carplayer;

typedef struct {
    int x, y;
    int active;
    int color;
} Carenemy;

typedef struct {
    int x, y;
    int active;
} Moon;

typedef struct {
    int x, y;
    int active;
    char trail[5];
    int trailIndex;
} Clouds;

typedef struct {
    int x, y;
    int active;
} Trafficlane;

// Funções do minigame
void screenDrawMinigameMap(int minimap);
void drawBorders();
void drawMoon();
void drawClouds();
void drawTrafficLane();
void drawCarplayer();
void drawCarenemy();
void moveMoon();
void moveClouds();
void moveTrafficLane();
void moveCarplayer(int dx, int dy);
void moveCarenemy();
void spawnMoon();
void spawnClouds();
void spawnTrafficLane();
void spawnCarenemy();
void checkCollision();

// Função principal do minigame
void startMinigame();

#endif // MINIGAME_H
