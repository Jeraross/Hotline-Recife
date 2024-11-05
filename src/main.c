#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define MAP_WIDTH 55
#define MAP_HEIGHT 21
#define MAX_ENEMIES 8
#define MAX_AMMO 5
#define PLAYER_MAX_HEALTH 3
#define ENEMY_RESPAWN_INTERVAL 2
#define ENEMY_COOLDOWN_PERIOD 2  // Tempo que o inimigo fica parado ao colidir com o jogador
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

char map[MAP_HEIGHT][MAP_WIDTH] = {
    "#######################################################",
    "#######################################################",
    "##                 #                   #              #",
    "##    #            #                   #              #",
    "##    #            #                                  #",
    "##    ####   ################          ################",
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
};

typedef struct {
    int x, y;
    int active;
    int type; // 1 para munição, 2 para vida
} Drop;

Drop drops[MAX_ENEMIES] = {{0}}; // Inicializa os drops como inativos

// Definindo a estrutura Player
typedef struct {
    int x, y;
    int health;
    int hasWeapon;
    int ammo;
    int clips;
} Player;

Player player = {2, 2, PLAYER_MAX_HEALTH, 0, 5, 3}; // Inicializando o jogador

typedef struct {
    int x, y;
    int alive;
    int cooldown;
} Enemy;

Enemy enemies[MAX_ENEMIES] = { {5, 5, 1, 0}, {8, 2, 1, 0}, {15, 7, 1, 0}, {30, 15, 1, 0}, {35, 10, 1, 0} };

time_t lastEnemySpawn;
time_t lastKillTime;
time_t comboStartTime;
int score = 0, combo = 1, enemies_dead;
int comboColors[] = {COLOR_COMBO1, COLOR_COMBO2, COLOR_COMBO3};
int comboColorIndex = 0;

int porta_x, porta_y;

void screenDrawMap();
void drawHUD();
void drawComboHUD();
void drawPlayer();
void drawWeapon();
void drawEnemies();
void drawDrops();
void drawDoor();
void doorVerify();
void spawnDrop(int x, int y);
void updateScore(int points, int isEnemyKill);
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
    drawHUD();
    drawWeapon();
    drawDrops();

    time_t lastEnemyMove = time(NULL);
    lastEnemySpawn = time(NULL);
    porta_x = MAP_WIDTH - 2;
    porta_y = 10;
    enemies_dead = 0;

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
        drawWeapon();
        drawDrops();
        drawComboHUD();  // Atualiza o HUD do combo com cor e tempo
        // Verificar se o tempo do combo acabou
        if (combo >= 2 && difftime(time(NULL), comboStartTime) >= 5) {
            combo = 1;  // Reseta o combo se o tempo acabou
        }
        drawDoor();
        doorVerify();
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

void drawHUD() {
    screenGotoxy(0, MAP_HEIGHT);
    screenSetColor(WHITE, BLACK);
    printf("Vida: %d  Munição: %d/%d Score: %d \n",
           player.health, player.ammo, player.clips, score);
    fflush(stdout);
}

void drawComboHUD() {
    if (combo < 2) {
        // Limpa o HUD do combo quando o combo termina
        screenGotoxy(COMBO_HUD_X, COMBO_HUD_Y);
        printf("                  "); // Apaga o texto do combo
        fflush(stdout);
        return;
    }

    // Alternar cor
    comboColorIndex = (comboColorIndex + 1) % 3;  // Cíclico entre as três cores
    screenSetColor(comboColors[comboColorIndex], BLACK);

    screenGotoxy(COMBO_HUD_X, COMBO_HUD_Y);
    int remainingTime = 5 - (int)difftime(time(NULL), comboStartTime);
    printf("COMBO X%d TIMER %d ", combo, remainingTime);

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
        screenSetColor(WHITE, BLACK);
        screenGotoxy(32, 4);
        printf("W");
        fflush(stdout);
    }
}

void drawDrops() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (drops[i].active) {
            screenGotoxy(drops[i].x, drops[i].y);
            if (drops[i].type == 1) {
                screenSetColor(COLOR_DROP_AMMO, BLACK);
                printf("A"); // Drop de munição
            } else if (drops[i].type == 2) {
                screenSetColor(COLOR_DROP_HEALTH, BLACK);
                printf("H"); // Drop de vida
            }
            fflush(stdout);
        }
    }
}

void drawDoor() {
    if (enemies_dead < 10) return;
    screenGotoxy(porta_x, porta_y);
    screenSetColor(COLOR_DOOR, BLACK);
    printf("🚪");
    fflush(stdout);
}

void doorVerify() {
    if (enemies_dead >= 10 && player.x == porta_x && player.y == porta_y) {
        keyboardDestroy();
        screenClear();
        printf("Parabéns! Você completou o jogo!\n");
        exit(0); // Termina o jogo
    }
}

void spawnDrop(int x, int y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!drops[i].active) {
            drops[i].x = x;
            drops[i].y = y;
            drops[i].active = 1;
            drops[i].type = (rand() % 2) + 1; // 1 para munição, 2 para vida
            drawDrops();
            break;
        }
    }
}

void updateScore(int points, int isEnemyKill) {
    time_t currentTime = time(NULL);
    if (isEnemyKill) {
        if (difftime(currentTime, comboStartTime) < 5) {
            combo++;
        } else {
            combo = 1;
        }
        comboStartTime = currentTime;  // Reinicia o timer para o combo
        score += points * combo;
    } else {
        score += points;
    }
    drawHUD();
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

        if (player.x == 32 && player.y == 4) { // Verificando a coleta da arma
            player.hasWeapon = 1;
            player.ammo = MAX_AMMO;
            screenGotoxy(32, 4);
            printf(" ");
        }

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (drops[i].active && drops[i].x == player.x && drops[i].y == player.y) {
                if (drops[i].type == 1 && player.clips < MAX_CLIPS) {
                    player.clips++;
                } else if (drops[i].type == 2 && player.health < PLAYER_MAX_HEALTH) {
                    player.health++;
                }
                drops[i].active = 0;
                screenGotoxy(drops[i].x, drops[i].y);
                printf(" ");
            }
        }
    }

    drawPlayer();
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
            screenSetColor(RED, BLACK);
            screenGotoxy(player.x, player.y);
            printf("@");
            fflush(stdout);
            usleep(100000);  // Mantém o feedback vermelho por 100ms
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
            enemies_dead++;
            int dropChance = rand() % 100; // Gera um número aleatório de 0 a 99
            if (dropChance < DROP_CHANCE) { // Se o número gerado for menor que a chance de drop
                spawnDrop(enemies[i].x, enemies[i].y);  // Gera o drop na posição do inimigo derrotado
            }
            updateScore(100, 1);
        }
    }
    screenGotoxy(player.x, player.y);
    drawPlayer();

    usleep(50000);  // Aguarda para mostrar o feedback visual
    screenDrawMap();
    drawPlayer();
    drawEnemies();
    drawWeapon();
    drawDrops();
}

void playerShoot(int dx, int dy) {
    if (!player.hasWeapon || player.ammo <= 0) return;

    int x = player.x + dx;
    int y = player.y + dy;
    int range;
    if (dy == 0) range = 10; else range = 5;
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
                enemies_dead++;
                screenGotoxy(x, y);
                printf(" ");
                int dropChance = rand() % 100; // Gera um número aleatório de 0 a 99
                if (dropChance < DROP_CHANCE) { // Se o número gerado for menor que a chance de drop
                    spawnDrop(enemies[i].x, enemies[i].y);  // Gera o drop na posição do inimigo derrotado
                }
                updateScore(100, 1);
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
    drawDrops();
}

void reload() {
    if (player.clips > 0 && player.ammo < MAX_AMMO) {
        player.ammo = MAX_AMMO;
        player.clips--;
        drawHUD();
    }
}