#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define NUM_MAPS 1
#define MAP_HEIGHT 4  // Mudado para 4 linhas
#define MAP_WIDTH 35
#define MAX_CAR_ENEMIES 10

#define COLOR_WALL YELLOW
#define COLOR_FLOOR LIGHTGRAY
#define COLOR_PLAYER GREEN
#define COLOR_ENEMY MAGENTA

char maps[NUM_MAPS][MAP_HEIGHT][MAP_WIDTH + 1] = {
    {
        "-----------------------------------",
        "-----------------------------------",
        "-----------------------------------",
        "-----------------------------------",
    }
};

typedef struct {
    int x, y;
} Carplayer;

Carplayer car_player = {0, 1}; 

typedef struct {
    int x, y;
    int active;
} Carenemy;

Carenemy car_enemies[MAX_CAR_ENEMIES] = {0};

int mapIndex = 0;

void screenDrawMap(int mapIndex);
void drawCarplayer();
void drawCarenemy();
void moveCarplayer(int dx, int dy);
void moveCarenemy();
void spawnCarenemy();
void checkCollision();

int main() {
    srand(time(NULL));
    screenInit(0);
    keyboardInit();
    timerInit(100);

    int running = 1;
    int spawnCounter = 0;

    screenDrawMap(mapIndex);
    drawCarplayer();

    while (running) {
        if (keyhit()) {
            int ch = readch();
            if (ch == 'q') {
                running = 0; 
            } else if (ch == 'w') {
                moveCarplayer(0, -1); 
            } else if (ch == 's') {
                moveCarplayer(0, 1); 
            }
        }

        if (timerTimeOver()) {
            moveCarenemy();
            spawnCounter += 100;

            if (spawnCounter >= 400 && spawnCounter < 800) {
                if (rand() % 100 < 50) {  
                    spawnCarenemy();
                    spawnCounter = 0; 
                }
            }

            if (spawnCounter >= 800) {
                spawnCarenemy();
                spawnCounter = 0; 
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
            screenGotoxy(MAP_WIDTH / 2 - 5, MAP_HEIGHT / 2);  
            screenSetColor(RED, BLACK);  
            printf("Game Over\n"); 
            screenSetColor(WHITE, BLACK); 
            fflush(stdout);
            sleep(2); 
            exit(0); 
        }
    }
}

void screenDrawMap(int mapIndex) {
    screenClear(); 
    
    for (int y = 0; y < MAP_HEIGHT; y++) {
        screenGotoxy(0, y); 
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = maps[mapIndex][y][x];

            switch(cell) {
                case '-':
                    screenSetColor(COLOR_FLOOR, BLACK);
                    printf(" ");
                    break;
                case '#':
                    screenSetColor(COLOR_WALL, BLACK);
                    printf("▓");
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

void drawCarplayer() {
    screenSetColor(COLOR_PLAYER, BLACK);
    screenGotoxy(car_player.x, car_player.y);
    printf("@");
    fflush(stdout);
}

void drawCarenemy() {
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        if (car_enemies[i].active) {
            screenSetColor(COLOR_ENEMY, BLACK);
            screenGotoxy(car_enemies[i].x, car_enemies[i].y);
            printf("E");
        }
    }
    fflush(stdout);
}

void moveCarplayer(int dx, int dy) {
    int newX = car_player.x + dx;
    int newY = car_player.y + dy;

    if (newY >= 0 && newY < MAP_HEIGHT) {
        screenGotoxy(car_player.x, car_player.y);
        printf(" ");
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
            printf(" ");
            car_enemies[i].x--;

            if (car_enemies[i].x < 0) {
                car_enemies[i].active = 0;
            }
        }
    }
    drawCarenemy();
    checkCollision();
}

void spawnCarenemy() {
    for (int i = 0; i < MAX_CAR_ENEMIES; i++) {
        if (!car_enemies[i].active) {
            car_enemies[i].x = MAP_WIDTH - 1;
            car_enemies[i].y = rand() % MAP_HEIGHT;
            car_enemies[i].active = 1;
            break;
        }
    }
}
