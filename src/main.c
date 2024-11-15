#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define NUM_MAPS 1
#define MAP_HEIGHT 16
#define MAP_WIDTH 80
#define MAX_CLOUDS 20
#define MAX_TRAFFIC_LANES 21
#define MAX_CAR_ENEMIES 15
#define MAP_START_Y 1

#define COLOR_FLOOR LIGHTGRAY
#define COLOR_SKY BLACK
#define COLOR_PLAYER GREEN
#define COLOR_ENEMY MAGENTA
#define COLOR_TRAFFIC_LANE YELLOW
#define COLOR_MOON YELLOW
#define COLOR_CLOUDS WHITE

char maps[NUM_MAPS][MAP_HEIGHT][MAP_WIDTH] = {
    {
        "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
        "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
        "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
        "________________________________________________________________________________",
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
        "________________________________________________________________________________",

    }
};

typedef struct {
    int x, y;
} Carplayer;

Carplayer car_player = {7, MAP_START_Y + 4}; 

typedef struct {
    int x, y;
    int active;
} Carenemy;

Carenemy car_enemies[MAX_CAR_ENEMIES] = {0};

typedef struct {
    int x, y;
    int active;
} Moon;

Moon moon = {0};

typedef struct {
    int x, y;
    int active;
} Clouds;

Clouds clouds[MAX_CLOUDS] = {0};

typedef struct {
    int x, y;
    int active;
} Trafficlane;

Trafficlane traffic_lanes[MAX_TRAFFIC_LANES] = {{4,7,1},{12,7,1},{20,7,1},{28,7,1},{36,7,1},{44,7,1},{52,7,1},{60,7,1},{68,7,1},{76,7,1}};

int mapIndex = 0;

void screenDrawMinigameMap(int mapIndex);
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

int main() {
    srand(time(NULL));
    screenInit(0);
    keyboardInit();
    timerInit(25);

    int running = 1;

    int moon_velocity = 0;

    int spawn_clouds_counter = 0;
    int spawn_clouds_choice = 0;
    int clouds_velocity = 0;

    int spawn_car_counter = 0;
    int spawn_car_choice = 0;
    int car_velocity = 0;

    int spawn_traffic_lane_counter = 0;

    screenDrawMinigameMap(mapIndex);
    drawCarplayer();

    while (running) {
        if (keyhit()) {
            int ch = readch();
            if (ch == 'q') {
                running = 0; 
            } else if (ch == 'w') {
                moveCarplayer(0, -3); 
            } else if (ch == 's') {
                moveCarplayer(0, 3); 
            }
        }

        if (timerTimeOver()) {

            car_velocity += 25;

            if(car_velocity >= 50){
                moveCarenemy();
                car_velocity = 0;
            }

            spawnMoon();
            drawMoon();

            moveTrafficLane();
            drawTrafficLane();

            moon_velocity += 25;

            if (moon_velocity >= 500) {
                moveMoon();
                moon_velocity = 0;
            }

            clouds_velocity += 25;

            if (clouds_velocity >= 50) {
                moveClouds();
                clouds_velocity = 0;
            }

            spawn_clouds_counter += 25;

            if (spawn_clouds_choice < 4 && spawn_clouds_counter >= 600 && spawn_clouds_counter < 800) {
                if (rand() % 100 < 50) {  
                    spawnClouds();
                    spawn_clouds_counter = 0; 
                } else {
                    spawn_clouds_choice++;
                }
            }

            if (spawn_clouds_choice < 8 && spawn_clouds_counter >= 800 && spawn_clouds_counter < 1000) {
                if (rand() % 100 < 50) {  
                    spawnClouds();
                    spawnClouds();
                    spawn_clouds_counter = 0; 
                    spawn_clouds_choice = 0;
                } else {
                    spawn_clouds_choice++;
                }
            }

            if (spawn_clouds_counter >= 1000) {
                spawnClouds();
                spawnClouds();
                spawnClouds();
                spawn_clouds_counter = 0; 
                spawn_clouds_choice = 0;
            }
            
            spawn_car_counter += 25;

            if (spawn_car_choice < 4 && spawn_car_counter >= 300 && spawn_car_counter < 400) {
                if (rand() % 100 < 50) {  
                    spawnCarenemy();
                    spawn_car_counter = 0; 
                } else {
                    spawn_car_choice++;
                }
            }

            if (spawn_car_choice < 8 && spawn_car_counter >= 400 && spawn_car_counter < 500) {
                if (rand() % 100 < 50) {  
                    spawnCarenemy();
                    spawnCarenemy();
                    spawn_car_counter = 0; 
                    spawn_car_choice = 0;
                } else {
                    spawn_car_choice++;
                }
            }

            if (spawn_car_counter >= 500) {
                spawnCarenemy();
                spawnCarenemy();
                spawnCarenemy();
                spawn_car_counter = 0; 
                spawn_car_choice = 0;
            }

            spawn_traffic_lane_counter += 25;

            if (spawn_traffic_lane_counter >= 200) {
                spawnTrafficLane();
                spawn_traffic_lane_counter = 0; 
            }

            
        }

        checkCollision();
        usleep(5000);
    }

    screenDestroy();
    keyboardDestroy();
    timerDestroy();
    return 0;
}

void checkCollision() {
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        if (car_enemies[i].active && car_player.x == car_enemies[i].x && car_player.y == car_enemies[i].y) {
            screenClear();  
            screenGotoxy(MAP_WIDTH / 2 - 5, MAP_HEIGHT / 2 + MAP_START_Y);  
            screenSetColor(RED, BLACK);  
            printf("Game Over\n"); 
            screenSetColor(WHITE, BLACK); 
            fflush(stdout);
            sleep(2); 
            exit(0); 
        }
    }
}

void screenDrawMinigameMap(int mapIndex) {
    screenClear();
    
    for (int y = 0; y < MAP_HEIGHT; y++) {
        screenGotoxy(0, y + MAP_START_Y); 
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = maps[mapIndex][y][x];

            switch(cell) {
                case 'c':
                    screenSetColor(COLOR_SKY, BLACK);
                    printf(" ");
                    break;
                case '_':
                    screenSetColor(COLOR_SKY, BLACK);
                    printf("-");
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
            screenSetColor(COLOR_ENEMY, BLACK);
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
                exit(0);
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
            printf("       ");
            clouds[i].x--;

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

    if (newY >= MAP_START_Y + 4 && newY < MAP_HEIGHT) {
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
            moon.x = MAP_WIDTH - 7;
            moon.y = MAP_START_Y + 1;
            moon.active = 1;
            break;
        }
    }
}

void spawnClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (!clouds[i].active) {
            clouds[i].x = MAP_WIDTH - 5;
            clouds[i].y = MAP_START_Y + (rand() % 3); 
            clouds[i].active = 1;
            break;
        }
    }
}

void spawnTrafficLane() {
    for (int i = 0; i < MAX_TRAFFIC_LANES; i++) {
        if (!traffic_lanes[i].active) {
            traffic_lanes[i].x = MAP_WIDTH - 4;
            traffic_lanes[i].y = MAP_START_Y + 6;
            traffic_lanes[i].active = 1;
            break;
        }
    }
}

void spawnCarenemy() {
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        if (!car_enemies[i].active) {
            car_enemies[i].x = MAP_WIDTH - 5;
            car_enemies[i].y = MAP_START_Y + 4 + (rand() % 4)*3; 
            car_enemies[i].active = 1;
            break;
        }
    }
}


