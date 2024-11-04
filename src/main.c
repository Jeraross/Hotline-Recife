#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define MAP_WIDTH 45
#define MAP_HEIGHT 21
#define MAX_ENEMIES 5
#define MAX_AMMO 10
#define PLAYER_MAX_HEALTH 3
#define ENEMY_RESPAWN_INTERVAL 2
#define ENEMY_COOLDOWN_PERIOD 2  // Tempo que o inimigo fica parado ao colidir com o jogador

#define COLOR_WALL RED
#define COLOR_FLOOR LIGHTGRAY
#define COLOR_PLAYER GREEN
#define COLOR_ENEMY MAGENTA
#define COLOR_ENEMY_HIT YELLOW // Cor temporária quando o inimigo atinge o jogador
#define COLOR_ATTACK CYAN

char map[MAP_HEIGHT][MAP_WIDTH] = {
    "#############################################",
    "#############################################",
    "##                                          #",
    "##    #            #                        #",
    "##    #            #                        #",
    "##    ####  ################   #######      #",
    "##                         #                #",
    "##    #######              #                #",
    "##                         #                #",
    "##    ######################   ##############",
    "##                 #                        #",
    "##       ########  #                        #",
    "##       #      #           #               #",
    "##       #      #########   #   ####        #",
    "##              #           #      #        #",
    "#######         #           #      #        #",
    "##                                          #",
    "##  ####   ####     ###   ####    ####      #",
    "##                  #        #              #",
    "##                  #        #              #",
    "#############################################"
};

// Definindo a estrutura Player
typedef struct {
    int x;
    int y;
    int health;
    int hasWeapon;
    int ammo;
} Player;

Player player = {2, 2, PLAYER_MAX_HEALTH, 0, 0}; // Inicializando o jogador

typedef struct {
    int x, y;
    int alive;
    int cooldown;
} Enemy;

Enemy enemies[MAX_ENEMIES] = { {5, 5, 1, 0}, {8, 2, 1, 0}, {15, 7, 1, 0}, {30, 15, 1, 0}, {35, 10, 1, 0} };

time_t lastEnemySpawn;

void screenDrawMap();
void drawHUD();
void drawPlayer();
void drawEnemies();
void drawWeapon();
int isOccupiedByEnemy(int x, int y);
void movePlayer(int dx, int dy);
void moveEnemies();
void spawnEnemies();
void showAttackFeedback();
void playerAttack();
void playerShoot(int dx, int dy);
void reload();


int main() {
    keyboardInit();
    screenInit(0);
    srand(time(NULL));

    screenDrawMap();
    drawPlayer();
    drawEnemies();
    drawWeapon();
    drawHUD();

    time_t lastEnemyMove = time(NULL);
    lastEnemySpawn = time(NULL);

    while (1) {
        if (keyhit()) {
            char key = readch();

            switch (key) {
                case 'w': movePlayer(0, -1); break;
                case 's': movePlayer(0, 1); break;
                case 'a': movePlayer(-1, 0); break;
                case 'd': movePlayer(1, 0); break;
                case ' ': playerAttack(); break;
                case 'i': playerShoot(0, -1); break;
                case 'k': playerShoot(0, 1); break;
                case 'j': playerShoot(-1, 0); break;
                case 'l': playerShoot(1, 0); break;
                case 'r': reload(); break;
                case 'q':
                    keyboardDestroy();
                    screenDestroy();
                    return 0;
            }
        }

        if (difftime(time(NULL), lastEnemyMove) >= 1) {
            moveEnemies();
            lastEnemyMove = time(NULL);
        }

        spawnEnemies();
        drawHUD();
    }
}
void screenDrawMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = map[y][x];
            screenGotoxy(x, y);

            switch(cell) {
                case '#':
                    screenSetColor(COLOR_WALL, BLACK);
                    printf("#");
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

void drawHUD() {
    screenGotoxy(0, MAP_HEIGHT);
    screenSetColor(WHITE, BLACK);
    printf("Vida: %d  Munição: %d\n", player.health, player.ammo);
    fflush(stdout);
}

void drawPlayer() {
    screenSetColor(COLOR_PLAYER, BLACK);
    screenGotoxy(player.x, player.y);
    printf("@");
    fflush(stdout);
}

void drawEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            screenGotoxy(enemies[i].x, enemies[i].y);

            if (enemies[i].cooldown > 0) {
                screenSetColor(COLOR_ENEMY_HIT, BLACK);
            } else {
                screenSetColor(COLOR_ENEMY, BLACK);
            }

            printf("E");
        }
    }
    fflush(stdout);
}

void drawWeapon() {
    if (!player.hasWeapon) {
        screenSetColor(YELLOW, BLACK);
        screenGotoxy(20, 10);
        printf("W");
        fflush(stdout);
    }
}

int isOccupiedByEnemy(int x, int y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

void movePlayer(int dx, int dy) {
    int newX = player.x + dx;
    int newY = player.y + dy;

    if (map[newY][newX] != '#' && !isOccupiedByEnemy(newX, newY)) {
        screenGotoxy(player.x, player.y);
        printf(" ");

        player.x = newX;
        player.y = newY;

        if (player.x == 20 && player.y == 10) { // Verificando a coleta da arma
            player.hasWeapon = 1;
            player.ammo = MAX_AMMO;
            screenGotoxy(20, 10);
            printf(" ");
        }

        drawPlayer();
    }
}

void moveEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        if (enemies[i].cooldown > 0) {
            enemies[i].cooldown--;
            continue;
        }

        int dx = 0, dy = 0;
        int randomMove = rand() % 100;

        if (randomMove < 20) {
            // 20% de chance de movimento aleatório
            dx = (rand() % 3) - 1;
            dy = (rand() % 3) - 1;
        } else {
            // Persegue o jogador
            if (map[enemies[i].y][enemies[i].x + 1] != '#' && enemies[i].x < player.x) dx = 1;
            else if (map[enemies[i].y][enemies[i].x - 1] != '#' && enemies[i].x > player.x) dx = -1;

            if (map[enemies[i].y + 1][enemies[i].x] != '#' && enemies[i].y < player.y) dy = 1;
            else if (map[enemies[i].y - 1][enemies[i].x] != '#' && enemies[i].y > player.y) dy = -1;
        }

        int nextX = enemies[i].x + dx;
        int nextY = enemies[i].y + dy;

        if (nextX == player.x && nextY == player.y) {
            player.health--;
            enemies[i].cooldown = ENEMY_COOLDOWN_PERIOD;
            drawHUD();
            if (player.health <= 0) {
                printf("Game Over!\n");
                exit(0);
            }
            continue;
        }

        if (map[nextY][nextX] != '#' && !isOccupiedByEnemy(nextX, nextY)) {
            screenGotoxy(enemies[i].x, enemies[i].y);
            printf(" ");
            enemies[i].x = nextX;
            enemies[i].y = nextY;
        }
    }
    drawEnemies();
}


void spawnEnemies() {
    if (difftime(time(NULL), lastEnemySpawn) < ENEMY_RESPAWN_INTERVAL) return;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) {
            int spawnX, spawnY;
            do {
                spawnX = rand() % MAP_WIDTH;
                spawnY = rand() % MAP_HEIGHT;
            } while (isOccupiedByEnemy(spawnX, spawnY) || abs(spawnX - player.x) < 5 || abs(spawnY - player.y) < 5 || map[spawnY][spawnX] == '#');

            enemies[i].x = spawnX;
            enemies[i].y = spawnY;
            enemies[i].alive = 1;
            lastEnemySpawn = time(NULL);
            break;
        }
    }
}

void showAttackFeedback() {
    screenSetColor(COLOR_ATTACK, BLACK);
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            int x = player.x + dx;
            int y = player.y + dy;
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && map[y][x] != '#') {
                screenGotoxy(x, y);
                printf("*");  // Feedback visual do ataque
            }
        }
    }
    fflush(stdout);
}

void playerAttack() {
    showAttackFeedback();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        if (abs(enemies[i].x - player.x) <= 1 && abs(enemies[i].y - player.y) <= 1) {
            screenGotoxy(enemies[i].x, enemies[i].y);
            printf(" ");
            enemies[i].alive = 0;

        }
    }
    screenGotoxy(player.x, player.y);
    drawPlayer();

    usleep(50000);  // Aguarda para mostrar o feedback visual
    screenDrawMap();
    drawPlayer();
    drawEnemies();
    drawWeapon();
}

void playerShoot(int dx, int dy) {
    if (!player.hasWeapon || player.ammo <= 0) return;

    int x = player.x + dx;
    int y = player.y + dy;
    int range = 5;
    player.ammo--;

    char shotChar = (dx == 0) ? '|' : '-';

    screenSetColor(CYAN, BLACK);

    for (int step = 0; step < range; step++) {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT || map[y][x] == '#') {
            break;
        }

        screenGotoxy(x, y);
        printf("%c", shotChar);
        fflush(stdout);
        usleep(50000);

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
                enemies[i].alive = 0;
                screenGotoxy(x, y);
                printf(" ");
                return;
            }
        }

        screenGotoxy(x, y);
        printf(" ");
        fflush(stdout);

        x += dx;
        y += dy;
    }

    screenDrawMap();
    drawPlayer();
    drawEnemies();
}

void reload() {
    player.ammo = MAX_AMMO;
    drawHUD();
}
