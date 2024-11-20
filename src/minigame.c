#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "menus.h"
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

#define SPEED_MOON 4
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

const int enemy_colors[] = {COLOR_ENEMY1, COLOR_ENEMY2, COLOR_ENEMY3, COLOR_ENEMY4, COLOR_ENEMY5, COLOR_ENEMY6};
#define NUM_ENEMY_COLORS 6

char mapa[MAPS_NUM][MAP_H][MAP_W] = {
    {
        "cccccc!@cccccccccc@cccccccc!cccccccccc@ccccccc!cc@ccccccccc@c!ccccccccc!ccccc@cc",
        "ccc!cccccc@cccccccc!ccccc@cccc!cccc!ccccc@c@ccccc!cccccccc!ccccccccc@ccccccccc!c",
        "cc@ccccccccc!cccccc@ccc!ccccccccccccc@cccc!cccccccccccc@cccccccccccc!ccc@ccccccc",
        "--------------------------------------------------------------------------------",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "                                                                      ",
        "--------------------------------------------------------------------------------",
    }
};

typedef struct {
    int x, y;
} Carplayer;

Carplayer car_player = {7, MAP_START_Y + 4}; 

typedef struct {
    int x, y;
    int active;
    int color;
} Carenemy;

int getRandomColor() {
    return enemy_colors[rand() % NUM_ENEMY_COLORS];
}

Carenemy car_enemies[MAX_CAR_ENEMIES] = {0};

typedef struct {
    int x, y;
    int active;
} Moon;

Moon moon = {0};

typedef struct {
    int x, y;
    int active;
    char trail[5];
    int trailIndex;
} Clouds;

Clouds clouds[MAX_CLOUDS] = {0};

typedef struct {
    int x, y;
    int active;
} Trafficlane;

Trafficlane traffic_lanes[MAX_TRAFFIC_LANES] = {{5,7,1},{15,7,1},{25,7,1},{35,7,1},{45,7,1},{55,7,1},{65,7,1},{75,7,1}};

int minimap = 0, finishgame;

void resetGameState();
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
void startMinigame() {
    srand(time(NULL));
    timerInit(TICK_RATE);

    resetGameState();

    int running = 1;

    int spawn_car_enemy_counter = 0;
    int spawn_clouds_counter = 0;
    int spawn_traffic_lane_counter = 0;

    int spawn_car_enemy_choice = 0;
    int spawn_clouds_choice = 0;

    int move_interval_car_enemy = 0;
    int move_interval_clouds = 0;
    int move_interval_moon = 0;
    int move_interval_traffic_lanes = 0;

    screenDrawMinigameMap(minimap);
    drawBorders();
    drawCarplayer();

    while (running) {
        if (finishgame == 1) {
          break;
        }
        if (keyhit()) {
            int ch = readch();
            if (ch == 'w') {
                moveCarplayer(0, -3);
            } else if (ch == 's') {
                moveCarplayer(0, 3);
            }
        }

        if (timerTimeOver()) {
            spawnMoon();
            move_interval_traffic_lanes += TICK_RATE;

            if(move_interval_traffic_lanes >= DEFAULT_MOVE_INTERVAL/SPEED_TRAFFIC_LANES){
                moveTrafficLane();
                move_interval_traffic_lanes = 0;
            }

            move_interval_car_enemy += TICK_RATE;

            if(move_interval_car_enemy >= DEFAULT_MOVE_INTERVAL/SPEED_CAR_ENEMIES){
                moveCarenemy();
                move_interval_car_enemy = 0;
            }

            move_interval_moon += TICK_RATE;

            if (move_interval_moon >= DEFAULT_MOVE_INTERVAL/SPEED_MOON) {
                moveMoon();
                move_interval_moon = 0;
            }

            move_interval_clouds += TICK_RATE;

            if (move_interval_clouds >= DEFAULT_MOVE_INTERVAL/SPEED_CLOUDS) {
                moveClouds();
                move_interval_clouds = 0;
            }

            spawn_clouds_counter += TICK_RATE;

            if (spawn_clouds_choice < 2 && spawn_clouds_counter >= 400 && spawn_clouds_counter < 600) {
                if (rand() % 100 < 50) {
                    spawnClouds();
                    spawn_clouds_counter = 0;
                } else {
                    spawn_clouds_choice++;
                }
            }

            if (spawn_clouds_choice < 4 && spawn_clouds_counter >= 600 && spawn_clouds_counter < 750) {
                if (rand() % 100 < 50) {
                    spawnClouds();
                    spawnClouds();
                    spawn_clouds_counter = 0;
                    spawn_clouds_choice = 0;
                } else {
                    spawn_clouds_choice++;
                }
            }

            if (spawn_clouds_counter >= 750) {
                spawnClouds();
                spawnClouds();
                spawnClouds();
                spawn_clouds_counter = 0;
                spawn_clouds_choice = 0;
            }

            spawn_car_enemy_counter += TICK_RATE;

            if (spawn_car_enemy_choice < 2 && spawn_car_enemy_counter >= 200 && spawn_car_enemy_counter < 300) {
                if (rand() % 100 < 50) {
                    spawnCarenemy();
                    spawn_car_enemy_counter = 0;
                } else {
                    spawn_car_enemy_choice++;
                }
            }

            if (spawn_car_enemy_choice < 4 && spawn_car_enemy_counter >= 300 && spawn_car_enemy_counter < 400) {
                if (rand() % 100 < 50) {
                    spawnCarenemy();
                    spawnCarenemy();
                    spawn_car_enemy_counter = 0;
                    spawn_car_enemy_choice = 0;
                } else {
                    spawn_car_enemy_counter++;
                }
            }

            if (spawn_car_enemy_counter >= 500) {
                spawnCarenemy();
                spawnCarenemy();
                spawnCarenemy();
                spawn_car_enemy_counter = 0;
                spawn_car_enemy_counter = 0;
            }

            spawn_traffic_lane_counter += TICK_RATE;

            if (spawn_traffic_lane_counter >= 200) {
                spawnTrafficLane();
                spawn_traffic_lane_counter = 0;
            }

            drawTrafficLane();
            drawMoon();

        }

        checkCollision();
        usleep(5000);
    }

    timerDestroy();
}

void resetGameState() {
    // Resetando jogador
    car_player.x = 7;
    car_player.y = MAP_START_Y + 4;

    // Resetando inimigos
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        car_enemies[i].x = 0;
        car_enemies[i].y = 0;
        car_enemies[i].active = 0;
        car_enemies[i].color = 0;
    }

    // Resetando lua
    moon.x = 0;
    moon.y = 0;
    moon.active = 0;

    // Resetando contadores e flags
    minimap = 0;
    finishgame = 0;
}

void checkCollision() {
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        if (car_enemies[i].active && car_player.x == car_enemies[i].x && car_player.y == car_enemies[i].y) {
            char final;
            gameover();
            scanf("%c", &final);
            if (final == '\n') {
                screenDestroy();
                keyboardDestroy();
                exit(0);
            }
        }
    }
}

void addCloudTrail(Clouds *cloud, char newChar) {
    cloud->trail[cloud->trailIndex] = newChar;
    cloud->trailIndex = (cloud->trailIndex + 1) % 5;
}

void printCloudTrailTail(Clouds *cloud) {
    int tailIndex = (cloud->trailIndex + 1) % 5;
    char tailChar = cloud->trail[tailIndex];

    const char *symbolToPrint;
    if (tailChar == 'C') {
        symbolToPrint = " ";  
    } else if (tailChar == '!') {
        symbolToPrint = "⋆";  
    } else if (tailChar == '@') {
        symbolToPrint = "✶";  
    } else {
        symbolToPrint = " ";   
    }

    screenGotoxy(cloud->x + 5, cloud->y);
    printf("%s", symbolToPrint); 
}


void screenDrawMinigameMap(int minimap) {
    screenClear();
    
    for (int y = 0; y < MAP_H; y++) {
        screenGotoxy(0, y + MAP_START_Y); 
        for (int x = 0; x < MAP_W; x++) {
            char cell = mapa[minimap][y][x];

            switch(cell) {
                case 'c':
                    screenSetColor(COLOR_SKY, BLACK);
                    printf(" ");
                    break;
                case '!':
                    screenSetColor(COLOR_STARS, BLACK);
                    printf("⋆");
                    break;
                case '@':
                    screenSetColor(COLOR_STARS, BLACK);
                    printf("✶");
                    break;
                case '-':
                    screenSetColor(COLOR_SKY, BLACK);
                    printf("-");
                    break;
                default:
                    screenSetColor(COLOR_F, DARKGRAY);
                    printf(" ");
                    break;
            }
        }
    }

    screenGotoxy(0, MAP_H + MAP_START_Y + 1);
    screenSetColor(WHITE, BLACK);
    printf("Indo para a próxima missão...");

    screenSetColor(WHITE, BLACK);
    fflush(stdout);
}

void drawBorders(){
    for (int i = 0; i < 1; i++) {
        screenSetColor(COLOR_F, BLACK);
        screenGotoxy(1, MAP_START_Y + 3);
        printf("________________________________________________________________________________");
        screenGotoxy(1, MAP_H);
        printf("________________________________________________________________________________");
    }
    fflush(stdout);
}

void drawMoon(){
    for (int i = 0; i < 1; i++) {
        if (moon.active) {
            screenSetColor(COLOR_MOON, YELLOW);
            screenGotoxy(moon.x, moon.y);
            printf("🌙");
        }
    }
    fflush(stdout);
}

void drawClouds(){
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (clouds[i].active) {
            screenSetColor(COLOR_CLOUDS, WHITE);
            screenGotoxy(clouds[i].x, clouds[i].y);
            printf("·☁︎°｡");
        }
    }
    fflush(stdout);
}

void drawTrafficLane(){
    for (int i = 0; i < MAX_TRAFFIC_LANES; i++) {
        if (traffic_lanes[i].active) {
            screenSetColor(COLOR_TRAFFIC_LANE, YELLOW);
            screenGotoxy(traffic_lanes[i].x, traffic_lanes[i].y);
            printf("--- ");
            screenGotoxy(traffic_lanes[i].x, traffic_lanes[i].y + 3);
            printf("--- ");
            screenGotoxy(traffic_lanes[i].x, traffic_lanes[i].y + 6);
            printf("--- ");
        }
    }
    fflush(stdout);
}

void drawCarplayer() {
    screenSetColor(COLOR_PLAYER, BLACK);
    screenGotoxy(car_player.x - 4, car_player.y);
    printf(",︵_  ");
    screenGotoxy(car_player.x - 4, car_player.y + 1);
    printf("┗O=Oˡ");
    fflush(stdout);
}

void drawCarenemy() {
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        if (car_enemies[i].active) {
            screenSetColor(car_enemies[i].color, BLACK);
            screenGotoxy(car_enemies[i].x, car_enemies[i].y);
            printf(" _︵,");
            screenGotoxy(car_enemies[i].x, car_enemies[i].y + 1);
            printf("ˡO=O┛");
        }
    }
    fflush(stdout);
}

void moveMoon() {
    for (int i = 0; i < 1; i++) {
        if (moon.active) {
            screenSetColor(COLOR_MOON, YELLOW);
            screenGotoxy(moon.x, moon.y);
            printf("  ");
            moon.x--;

            if (moon.x < 0) {
                moon.x = 0;
                screenClear();
                finishgame = 1;
                break;
            }
        }
    }
    drawMoon();
}

void moveClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (clouds[i].active) {
            screenSetColor(COLOR_CLOUDS, WHITE);
            screenGotoxy(clouds[i].x, clouds[i].y);
            printf("     ");
            
            clouds[i].x--;
            
            char currentChar = mapa[minimap][clouds[i].y-1][clouds[i].x];
            
            addCloudTrail(&clouds[i], currentChar);

            printCloudTrailTail(&clouds[i]);

            if (clouds[i].x < 0) {
                clouds[i].x = 0;
                clouds[i].active = 0;
            }
        }
    }
    drawClouds();
}

void moveTrafficLane() {
    for (int i = 0; i < MAX_TRAFFIC_LANES; i++) {
        if (traffic_lanes[i].active) {
            screenGotoxy(traffic_lanes[i].x, traffic_lanes[i].y);
            printf("     ");
            screenGotoxy(traffic_lanes[i].x, traffic_lanes[i].y + 3);
            printf("     ");
            screenGotoxy(traffic_lanes[i].x, traffic_lanes[i].y + 6);
            printf("     ");
            traffic_lanes[i].x--;

            if (traffic_lanes[i].x < 0) {
                traffic_lanes[i].x = 0;
                traffic_lanes[i].active = 0;
            }
        }
    }
    drawTrafficLane();
}


void moveCarplayer(int dx, int dy) {
    int newX = car_player.x + dx;
    int newY = car_player.y + dy;

    if (newY >= MAP_START_Y + 4 && newY < MAP_H) {
        screenGotoxy(car_player.x - 4, car_player.y);
        printf("     ");
        screenGotoxy(car_player.x - 4, car_player.y + 1);
        printf("     ");
        car_player.x = newX;
        car_player.y = newY;
        drawCarplayer();
        checkCollision();
    }
}

void moveCarenemy() {
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        if (car_enemies[i].active) {
            screenGotoxy(car_enemies[i].x, car_enemies[i].y);
            printf("     ");
            screenGotoxy(car_enemies[i].x, car_enemies[i].y + 1);
            printf("     ");
            car_enemies[i].x--;

            if (car_enemies[i].x < 0) {
                car_enemies[i].active = 0;
            }
        }
    }
    drawCarenemy();
    checkCollision();
}

void spawnMoon() {
    for (int i = 0; i < 1; i++) {
        if (!moon.active) {
            moon.x = MAP_W - 7;
            moon.y = MAP_START_Y + 1;
            moon.active = 1;
            break;
        }
    }
}

void spawnClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (!clouds[i].active) {
            clouds[i].x = MAP_W - 5;
            clouds[i].y = MAP_START_Y + (rand() % 3); 
            clouds[i].active = 1;
            break;
        }
    }
}

void spawnTrafficLane() {
    for (int i = 0; i < MAX_TRAFFIC_LANES; i++) {
        if (!traffic_lanes[i].active) {
            traffic_lanes[i].x = MAP_W - 4;
            traffic_lanes[i].y = MAP_START_Y + 6;
            traffic_lanes[i].active = 1;
            break;
        }
    }
}

void spawnCarenemy() {
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        if (!car_enemies[i].active) {
            car_enemies[i].x = MAP_W - 5;
            car_enemies[i].y = MAP_START_Y + 4 + (rand() % 4)*3;
            car_enemies[i].color = getRandomColor(); 
            car_enemies[i].active = 1;
            break;
        }
    }
}

