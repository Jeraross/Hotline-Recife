#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define MAP_WIDTH 55
#define MAP_HEIGHT 21
#define NUM_MAPS 2
#define MAX_ENEMIES 5
#define MAX_AMMO 5
#define PLAYER_MAX_HEALTH 5
#define ENEMY_RESPAWN_INTERVAL 2
#define ENEMY_COOLDOWN_PERIOD 5  // Tempo que o inimigo fica parado ao colidir com o jogador
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

char maps[NUM_MAPS][MAP_HEIGHT][MAP_WIDTH] = {
    {
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
    },
    {
        "#######################################################",
        "#######################################################",
        "##                                                    #",
        "####################                ###################",
        "##                                                    #",
        "##         #########                ########          #",
        "##         #                               #          #",
        "##         #                               #          #",
        "##         #                               #          #",
        "##         #                               #          #",
        "##                                                    #",
        "##                                                    #",
        "##         #                               #          #",
        "##         #                               #          #",
        "##         #                               #          #",
        "##         #                               #          #",
        "##         ########                 ########          #",
        "##                        #  #                        #",
        "##                       ##  ##                       #",
        "##                      ###  ###                      #", 
        "#######################################################"
    }
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

Player player = {2, 2, PLAYER_MAX_HEALTH, 0, 5, 2}; // Inicializando o jogador

typedef struct {
    int x, y;
    int alive;
    int cooldown;
    int type;
    int moves;
} Enemy;

Enemy enemies[MAX_ENEMIES] = { {10, 5, 1, 0, 0}, {8, 2, 1, 0, 0}, {15, 7, 1, 0, 0}, {30, 15, 1, 0, 0}, {35, 10, 1, 0, 0} };


time_t lastEnemySpawn;
time_t lastKillTime;
time_t comboStartTime;

int score = 0, combo = 1, pontosGanhos = 0, playerDetected = 0, px = 0, py = 0, enemies_dead;
int comboColors[] = {COLOR_COMBO1, COLOR_COMBO2, COLOR_COMBO3};
int comboColorIndex = 0;

int porta_x, porta_y;
int mapIndex;

void screenDrawMap(int mapIndex);
void drawHUD();
void drawComboHUD();
void drawPlayer();
void drawGun();
void drawEnemies();
void drawDrops();
void drawDoor();
int doorVerify();
void spawnDrop(int x, int y);
void updateScore(int points, int isEnemyKill);
int isOccupiedByEnemy(int x, int y);
void movePlayer(int dx, int dy);
void moveEnemies();
void spawnEnemies();
void showAttackFeedback();
void playerAttack();
void playerShoot(int dx, int dy);
void enemyShoot(int enemyIndex, int plx, int ply);
void reload();

int main() {
    keyboardInit();
    screenInit(0);
    srand(time(NULL));
    mapIndex = 0;

    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawHUD();
    drawGun();
    drawDrops();

    clock_t lastEnemyMove = clock();
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
                case 'u': playerShoot(-1, -1); break;  // Diagonal superior esquerda
                case 'o': playerShoot(1, -1); break;   // Diagonal superior direita
                case 'm': playerShoot(-1, 1); break;   // Diagonal inferior esquerda
                case ',': playerShoot(1, 1); break;    // Diagonal inferior direita
                case 'r': reload(); break;
                case 'q':
                    keyboardDestroy();
                    screenDestroy();
                    return 0;
            }
        }

        if (((clock() - lastEnemyMove) / (double) CLOCKS_PER_SEC >= 0.6) && mapIndex == 0 ) {
            moveEnemies();
            lastEnemyMove = clock();
        }
        else if (((clock() - lastEnemyMove) / (double) CLOCKS_PER_SEC >= 0.3) && mapIndex == 1 ) {
            moveEnemies();
            lastEnemyMove = clock();
        }

        spawnEnemies();
        drawComboHUD();  // Atualiza o HUD do combo com cor e tempo
        drawHUD();
        drawGun();
        drawDrops();

        // Verificar se o tempo do combo acabou
        if (combo >= 2 && difftime(time(NULL), comboStartTime) >= 5) {
            combo = 1;  // Reseta o combo se o tempo acabou
        }

        drawDoor();

        if (doorVerify()){
            // Reinicia o array de inimigos
            memset(enemies, 0, sizeof(enemies)); // Zera todos os inimigos

            // Reinicia o array de drops
            memset(drops, 0, sizeof(drops)); // Zera todos os drops

            // Reinicia o jogador e o cenário
            player.x = 2;
            player.y = 2;
            porta_x = 27;
            porta_y = 19;
            player.hasWeapon = 0;

            // Limpa a tela e desenha o novo mapa
            screenClear();
            screenDrawMap(mapIndex);
            drawPlayer();
            drawEnemies();
            drawHUD();
            drawGun();
            drawDrops();
        }
    }
}

void screenDrawMap(int mapIndex) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = maps[mapIndex][y][x];  // Acessa o mapa específico usando mapIndex
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
    printf("Vida: %d  Munição: %d/%d Score: %d ",
           player.health, player.ammo, player.clips, score);
    fflush(stdout);
}

void drawComboHUD() {
    if (combo < 2) {
        // Limpa o HUD do combo quando o combo termina
        screenGotoxy(COMBO_HUD_X, COMBO_HUD_Y);
        printf("                               "); // Apaga o texto do combo
        fflush(stdout);
        return;
    }

    // Alternar cor
    comboColorIndex = (comboColorIndex + 1) % 3;  // Cíclico entre as três cores
    screenSetColor(comboColors[comboColorIndex], BLACK);

    screenGotoxy(COMBO_HUD_X, COMBO_HUD_Y);
    int remainingTime = 5 - (int)difftime(time(NULL), comboStartTime);
    printf("COMBO X%d   TIMER %d   +%d ", combo, remainingTime, pontosGanhos);

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
            }
            else if (enemies[i].moves <= 5 && enemies[i].moves != 0 && enemies[i].type == 2) {
                screenSetColor(RED, BLACK);  // Muda para branco antes de atirar
            }
            else {
                screenSetColor(COLOR_ENEMY, BLACK);
            }

            if (enemies[i].type == 0) {
                printf("E");
            }
            if (enemies[i].type == 1) {
                printf("D");
            }
            if (enemies[i].type == 2) {
                printf("C");
            }
        }
    }
    fflush(stdout);
}

void drawGun() {
    if (!player.hasWeapon && mapIndex == 0) {
        screenSetColor(WHITE, BLACK);
        screenGotoxy(53, 3);
        printf("G");
        fflush(stdout);
    }

    if (!player.hasWeapon && mapIndex == 1) {
        screenSetColor(WHITE, BLACK);
        screenGotoxy(28, 10);
        printf("S");
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

int doorVerify() {
    if (enemies_dead >= 10 && player.x == porta_x && player.y == porta_y) {
        mapIndex++;
        enemies_dead = 0;
        player.x = 1;
        player.y = 1;
        if (mapIndex >= NUM_MAPS) {
            screenClear();
            keyboardDestroy();
            screenGotoxy(0, MAP_HEIGHT + 1);
            printf("Fim de jogo!");
            exit(0);
        }
        else{
            return 1;
        }
    }
    else{
        return 0;
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

    if (maps[mapIndex][newY][newX] != '#' && !isOccupiedByEnemy(newX, newY)) {
        screenGotoxy(player.x, player.y);
        printf(" ");

        player.x = newX;
        player.y = newY;

        if (mapIndex==0 &&player.x == 53 && player.y == 3) { // Verificando a coleta da arma
            player.hasWeapon = 1;
            player.ammo = MAX_AMMO;
            screenGotoxy(32, 4);
            printf(" ");
        }

        if (mapIndex==1 &&player.x == 28 && player.y == 10) { // Verificando a coleta da arma
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

        if (enemies[i].type == 2) {

            if (!playerDetected) {
                // Verifica se o jogador está na mesma linha, coluna ou diagonal
                // Horizontal (mesma linha)
                if (enemies[i].y == player.y) {
                    int x = enemies[i].x < player.x ? enemies[i].x + 1 : enemies[i].x - 1;
                    while (x >= 0 && x < MAP_WIDTH && maps[mapIndex][enemies[i].y][x] != '#') {
                        if (x == player.x) {
                            px = player.x;
                            py = player.y;
                            playerDetected = 1;
                            break;
                        }
                        x += (enemies[i].x < player.x ? 1 : -1);
                    }
                }

                // Vertical (mesma coluna)
                if (!playerDetected && enemies[i].x == player.x) {
                    int y = enemies[i].y < player.y ? enemies[i].y + 1 : enemies[i].y - 1;
                    while (y >= 0 && y < MAP_HEIGHT && maps[mapIndex][y][enemies[i].x] != '#') {
                        if (y == player.y) {
                            px = player.x;
                            py = player.y;
                            playerDetected = 1;
                            break;
                        }
                        y += (enemies[i].y < player.y ? 1 : -1);
                    }
                }

                // Diagonal
                if (!playerDetected) {
                    int dx = (enemies[i].x < player.x) ? 1 : -1;
                    int dy = (enemies[i].y < player.y) ? 1 : -1;
                    int x = enemies[i].x + dx, y = enemies[i].y + dy;
                    while (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && maps[mapIndex][y][x] != '#') {
                        if (x == player.x && y == player.y) {
                            px = player.x;
                            py = player.y;
                            playerDetected = 1;
                            break;
                        }
                        x += dx;
                        y += dy;
                    }
                }
            }


            // Se o jogador foi detectado, muda a cor do inimigo para vermelho
            if (playerDetected) {
                enemies[i].moves++;  // Inicia a contagem para o tiro
            }

            // No 4º movimento, o inimigo atira
            if (enemies[i].moves == 6) {
                enemyShoot(i, px, py);  // Chama a função para o tiro
                enemies[i].moves = 0;  // Reseta
                playerDetected = 0; // Reseta
            }
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
            if (maps[mapIndex][enemies[i].y][enemies[i].x + 1] != '#' && enemies[i].x < player.x) 
                dx = 1;
            else if (maps[mapIndex][enemies[i].y][enemies[i].x - 1] != '#' && enemies[i].x > player.x) 
                dx = -1;

            if (maps[mapIndex][enemies[i].y + 1][enemies[i].x] != '#' && enemies[i].y < player.y) 
                dy = 1;
            else if (maps[mapIndex][enemies[i].y - 1][enemies[i].x] != '#' && enemies[i].y > player.y) 
                dy = -1;

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

        if (maps[mapIndex][nextY][nextX] != '#' && !isOccupiedByEnemy(nextX, nextY)) {
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
    int randomEnemy = rand() % 100;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) {
            int spawnX, spawnY;
            do {
                spawnX = rand() % MAP_WIDTH;
                spawnY = rand() % MAP_HEIGHT;
            } while (isOccupiedByEnemy(spawnX, spawnY) || abs(spawnX - player.x) < 5 || abs(spawnY - player.y) < 5 || maps[mapIndex][spawnY][spawnX] == '#');

            enemies[i].x = spawnX;
            enemies[i].y = spawnY;
            if (mapIndex == 0) {
                enemies[i].type = 0;
            }
            else if (mapIndex == 1) {
                if (randomEnemy < 50) {
                    enemies[i].type = 1;
                    enemies[i].moves = 0;
                } else {
                    enemies[i].type = 2;
                    enemies[i].moves = 0;
                }
            }
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
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && maps[mapIndex][y][x] != '#') {
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
            pontosGanhos = 100 * combo;
            updateScore(100, 1);
        }
    }
    screenGotoxy(player.x, player.y);
    drawPlayer();

    usleep(25000);  // Aguarda para mostrar o feedback visual
    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawGun();
    drawDrops();
}

void playerShoot(int dx, int dy) {
    if (!player.hasWeapon || player.ammo <= 0) return;

    int x = player.x + dx;
    int y = player.y + dy;
    int range = 10;

    player.ammo--;

    char shotChar;
    if (dx == 0 || dy == 0) {
        shotChar = (dx == 0) ? '|' : '-';  // Vertical ou horizontal
    } else {
        shotChar = (dx == dy) ? '\\' : '/';  // Diagonais
    }

    screenSetColor(CYAN, BLACK);

    for (int step = 0; step < range; step++) {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT || maps[mapIndex][y][x] == '#') {
            break;
        }

        screenGotoxy(x, y);
        printf("%c", shotChar);
        fflush(stdout);
        usleep(25000);

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
                pontosGanhos = 100 * combo;
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

    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawDrops();
}

void enemyShoot(int enemyIndex, int plx, int ply) {
    int dx = plx - enemies[enemyIndex].x;
    int dy = ply - enemies[enemyIndex].y;

    int range = 10;

    int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
    int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

    int x = enemies[enemyIndex].x + stepX;
    int y = enemies[enemyIndex].y + stepY;

    char shotChar;
    if (stepX == 0 || stepY == 0) {
        shotChar = (stepX == 0) ? '|' : '-';  // Vertical ou horizontal
    } else {
        shotChar = (stepX == stepY) ? '\\' : '/';  // Diagonais
    }

    screenSetColor(RED, BLACK);

    for (int step = 0; step < range; step++) {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT || maps[mapIndex][y][x] == '#') {
            break;
        }

        screenGotoxy(x, y);
        printf("%c", shotChar);
        fflush(stdout);
        usleep(25000);

        if (x == player.x && y == player.y) {
            player.health--;  // O jogador perde vida
            screenGotoxy(x, y);
            printf(" ");
            drawHUD();  // Atualiza o HUD

            if (player.health <= 0) {
                printf("Game Over!\n");
                exit(0);
            }
            break;  // O tiro atinge o jogador, então interrompe o loop
        }

        screenGotoxy(x, y);
        printf(" ");
        fflush(stdout);

        x += stepX;
        y += stepY;
    }

    screenDrawMap(mapIndex);
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