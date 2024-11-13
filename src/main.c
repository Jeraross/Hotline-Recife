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
#define NUM_MAPS 3
#define MAX_ENEMIES 8
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
        "##         #########                                  #",
        "##         #                                          #",
        "##         #                                          #",
        "##         #                                          #",
        "##                                                    #",
        "##                                                    #",
        "##                                                    #",
        "##                                                    #",
        "##                                                    #",
        "##                                                    #",
        "##                                          #         #",
        "##                                          #         #",
        "##                                          #         #",
        "##                                   ########         #",
        "##                                                    #",
        "##                                                    #",
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
    int hasShotgun;
    int ammo;
    int clips;
    int currentWeapon; // 0 para pistola, 1 para shotgun
} Player;


Player player = {2, 2, PLAYER_MAX_HEALTH, 0, 5, 2}; // Inicializando o jogador

typedef struct {
    int x, y;
    int alive;
    int cooldown;
    int type;
    int moves; // atirador
    int playerDetected;
    int px, py;
} Enemy;

Enemy enemies[MAX_ENEMIES] = { {10, 5, 1, 0, 0}, {8, 2, 1, 0, 0}, {15, 7, 1, 0, 0}, {30, 15, 1, 0, 0}, {35, 10, 1, 0, 0} };

typedef struct {
    int x, y; //core matriz
    int health;
    int cooldown;
    int move;
    int tick;
} Boss;

Boss tanque = {MAP_WIDTH / 2, MAP_HEIGHT / 2, 30, 0, 1, 0};

time_t lastEnemySpawn;
time_t lastDropSpawn;
time_t comboStartTime;
clock_t lastEnemyMove;

int score = 0, combo = 1, pontosGanhos = 0, px = 0, py = 0, enemies_dead;
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
void drawBoss(int x, int y);
void drawDrops();
void drawDoor();
int doorVerify();
void spawnDrop(int x, int y);
void updateScore(int points, int isEnemyKill);
int isOccupiedByEnemy(int x, int y);
void movePlayer(int dx, int dy);
void moveEnemies();
void moveBoss();
void bossShoot(int direction);
void tripleBossShoot(int direction);
void spawnEnemies();
void showAttackFeedback();
void playerAttack();
void playerShoot(int dx, int dy);
void playerShotgunShoot(int dx, int dy);
void handlePlayerHit();
void enemyShoot(int enemyIndex, int plx, int ply);
void reload();

int main() {
    keyboardInit();
    screenInit(0);
    srand(time(NULL));
    mapIndex = 2;
    player.ammo = MAX_AMMO;
    player.clips = 2;
    player.hasWeapon = 0;
    player.hasShotgun = 0;
    player.currentWeapon = -1; // Começa sem arma

    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawHUD();
    drawGun();
    drawDrops();
    if (mapIndex == 2) {
      drawBoss(tanque.x, tanque.y);
    }

    lastEnemyMove = clock();
    lastEnemySpawn = time(NULL);
    lastDropSpawn = time(NULL);
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
                case 'i': if (player.currentWeapon == 1) playerShotgunShoot(0, -1); else playerShoot(0, -1); break;
                case 'k': if (player.currentWeapon == 1) playerShotgunShoot(0, 1); else playerShoot(0, 1); break;
                case 'j': if (player.currentWeapon == 1) playerShotgunShoot(-1, 0); else playerShoot(-1, 0); break;
                case 'l': if (player.currentWeapon == 1) playerShotgunShoot(1, 0); else playerShoot(1, 0); break;
                case 'u': playerShoot(-1, -1); break;   // Diagonal superior esquerda
                case 'o': playerShoot(1, -1); break;   // Diagonal superior direita
                case 'm': playerShoot(-1, 1); break;   // Diagonal inferior esquerda
                case ',': playerShoot(1, 1); break;    // Diagonal inferior direita
                case 'r': reload(); break;
                case 't':
                    if (player.hasWeapon && player.hasShotgun) {
                        player.currentWeapon = 1 - player.currentWeapon; // Alterna entre 0 e 1
                    }
                    break;

                case 'q':
                    keyboardDestroy();
                    screenDestroy();
                    return 0;
            }
        }

        if (((clock() - lastEnemyMove) / (double) CLOCKS_PER_SEC >= 0.5) && mapIndex == 0 ) {
            moveEnemies();
            lastEnemyMove = clock();
        }
        else if (((clock() - lastEnemyMove) / (double) CLOCKS_PER_SEC >= 0.3) && mapIndex == 1 ) {
            moveEnemies();
            lastEnemyMove = clock();
        } else if (mapIndex == 2) {
            if (((clock() - lastEnemyMove) / (double) CLOCKS_PER_SEC >= 0.5)) {
                  moveBoss();
                  lastEnemyMove = clock();
            }
            if (difftime(time(NULL), lastDropSpawn) >= 20) {
                  spawnDrop(-1, -1);
                  lastDropSpawn = time(NULL);
            }
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

            // Limpa a tela e desenha o novo mapa
            screenClear();
            screenDrawMap(mapIndex);
            drawPlayer();
            drawEnemies();
            drawHUD();
            drawGun();
            drawDrops();
            if (mapIndex == 2) {
                drawBoss(tanque.x, tanque.y);
            }
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
    const char *currentWeaponName = (player.currentWeapon == 0) ? "Pistola" :
                                    (player.currentWeapon == 1) ? "Shotgun" : "Nada";
    printf("Vida: %d  Munição: %d/%d  Score: %d  Arma: %s",
           player.health, player.ammo, player.clips, score, currentWeaponName);
}


void drawComboHUD() {
    if (combo < 2) {
        // Limpa o HUD do combo quando o combo termina
        screenGotoxy(COMBO_HUD_X, COMBO_HUD_Y);
        printf("                                 "); // Apaga o texto do combo
        fflush(stdout);
        return;
    }

    // Alternar cor
    comboColorIndex = (comboColorIndex + 1) % 3;  // Cíclico entre as três cores
    screenSetColor(comboColors[comboColorIndex], BLACK);

    screenGotoxy(COMBO_HUD_X, COMBO_HUD_Y);
    int remainingTime = 5 - (int)difftime(time(NULL), comboStartTime);
    printf("COMBO X%d    TIMER %d    +%d ", combo, remainingTime, pontosGanhos);

    fflush(stdout);
}

void drawPlayer() {
    screenSetColor(COLOR_PLAYER, BLACK);
    screenGotoxy(player.x, player.y);
    printf("ඞ");
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

void drawBoss(int x, int y) {
    // Configuração de cor padrão do chefe
    if (tanque.cooldown > 0) {
        screenSetColor(COLOR_ENEMY_HIT, BLACK);
    } else if (tanque.tick <= 2 && tanque.tick != 0) {
        screenSetColor(RED, BLACK);
    } else {
        screenSetColor(COLOR_ENEMY, BLACK);
    }

    // Segunda linha da matriz do boss
    screenGotoxy(x - 4, y - 1);
    printf(" __( )===:");

    // Terceira linha da matriz do boss
    screenGotoxy(x - 4, y);
    printf("/~~~X~~~\\");

    // Quarta linha da matriz do boss
    screenGotoxy(x - 4, y + 1);
    printf("\\O.O.O.O/");

    // Limpa o buffer para exibir imediatamente
    fflush(stdout);
}

void drawGun() {
    if (!player.hasWeapon && mapIndex == 0) {
        screenSetColor(WHITE, BLACK);
        screenGotoxy(53, 3);
        printf("G");
        fflush(stdout);
    }
    if (!player.hasShotgun && mapIndex == 1) {
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
    int cont;
    if (mapIndex == 2) {
        if (!drops[0].active) {
            drops[0].x = 9;
            drops[0].y = 4;
            drops[0].active = 1;
            drops[0].type = 1;
            drawDrops();
        }
        if (!drops[1].active) {
            drops[1].x = 46;
            drops[1].y = 17;
            drops[1].active = 1;
            drops[1].type = 2;
            drawDrops();
        }
        return;
    }
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
            player.currentWeapon = 0;
            player.ammo = MAX_AMMO;
            screenGotoxy(32, 4);
            printf(" ");
        }

        if (mapIndex==1 &&player.x == 28 && player.y == 10) { // Verificando a coleta da arma
            player.hasShotgun = 1;
            player.currentWeapon = 1;
            player.ammo = MAX_AMMO;
            screenGotoxy(28, 10);
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
    drawHUD();
}

void moveEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        if (enemies[i].cooldown > 0) {
            enemies[i].cooldown--;
            continue;
        }

        if (enemies[i].type == 2) {
            if (!enemies[i].playerDetected) {
                // Verificação Horizontal (esquerda e direita)
                for (int x = enemies[i].x + 1; x < MAP_WIDTH && maps[mapIndex][enemies[i].y][x] != '#'; x++) {
                    if (x == player.x && enemies[i].y == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (int x = enemies[i].x - 1; x >= 0 && maps[mapIndex][enemies[i].y][x] != '#'; x--) {
                    if (x == player.x && enemies[i].y == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }

                // Verificação Vertical (acima e abaixo)
                for (int y = enemies[i].y + 1; y < MAP_HEIGHT && maps[mapIndex][y][enemies[i].x] != '#'; y++) {
                    if (enemies[i].x == player.x && y == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (int y = enemies[i].y - 1; y >= 0 && maps[mapIndex][y][enemies[i].x] != '#'; y--) {
                    if (enemies[i].x == player.x && y == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }

                // Verificação Diagonal (cima-esquerda, cima-direita, baixo-esquerda, baixo-direita)
                int dx, dy;
                for (dx = 1, dy = 1; enemies[i].x + dx < MAP_WIDTH && enemies[i].y + dy < MAP_HEIGHT && maps[mapIndex][enemies[i].y + dy][enemies[i].x + dx] != '#'; dx++, dy++) {
                    if (enemies[i].x + dx == player.x && enemies[i].y + dy == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (dx = -1, dy = 1; enemies[i].x + dx >= 0 && enemies[i].y + dy < MAP_HEIGHT && maps[mapIndex][enemies[i].y + dy][enemies[i].x + dx] != '#'; dx--, dy++) {
                    if (enemies[i].x + dx == player.x && enemies[i].y + dy == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (dx = 1, dy = -1; enemies[i].x + dx < MAP_WIDTH && enemies[i].y + dy >= 0 && maps[mapIndex][enemies[i].y + dy][enemies[i].x + dx] != '#'; dx++, dy--) {
                    if (enemies[i].x + dx == player.x && enemies[i].y + dy == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (dx = -1, dy = -1; enemies[i].x + dx >= 0 && enemies[i].y + dy >= 0 && maps[mapIndex][enemies[i].y + dy][enemies[i].x + dx] != '#'; dx--, dy--) {
                    if (enemies[i].x + dx == player.x && enemies[i].y + dy == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
            }
            if (enemies[i].moves == 4) {
                enemyShoot(i, enemies[i].px, enemies[i].py);  // Chama a função para o tiro
                enemies[i].cooldown = 10;
                enemies[i].moves = 0;  // Reseta
                enemies[i].playerDetected = 0;    // Reseta
            }
            // Preparar o ataque se o jogador foi detectado
            if (enemies[i].playerDetected) {
                enemies[i].moves++;  // Inicia a contagem para o tiro
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
            handlePlayerHit();
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

void bossShoot(int direction) {
    int startX = tanque.x;
    int startY = tanque.y;
    int range = 30;
    char shotChar;

    // Configuração de cor para o tiro
    screenSetColor(RED, BLACK);

    // Disparo nas quatro direções conforme o tipo
    for (int i = 1; i <= range; i++) {
        // Tiros horizontais e verticais
        if (direction == 1) {
            // Direita
            int xRight = startX + i;
            if (xRight >= 0 && xRight < MAP_WIDTH && maps[mapIndex][startY][xRight] != '#') {
                screenGotoxy(xRight, startY);
                printf("-");
                fflush(stdout);
                if (xRight == player.x && startY == player.y) {
                    handlePlayerHit();
                    break;
                }
            }

            // Esquerda
            int xLeft = startX - i;
            if (xLeft >= 0 && xLeft < MAP_WIDTH && maps[mapIndex][startY][xLeft] != '#') {
                screenGotoxy(xLeft, startY);
                printf("-");
                fflush(stdout);
                if (xLeft == player.x && startY == player.y) {
                    handlePlayerHit();
                    break;
                }
            }

            // Cima
            int yUp = startY - i;
            if (yUp >= 0 && yUp < MAP_HEIGHT && maps[mapIndex][yUp][startX] != '#') {
                screenGotoxy(startX, yUp);
                printf("|");
                fflush(stdout);
                if (startX == player.x && yUp == player.y) {
                    handlePlayerHit();
                    break;
                }
            }

            // Baixo
            int yDown = startY + i;
            if (yDown >= 0 && yDown < MAP_HEIGHT && maps[mapIndex][yDown][startX] != '#') {
                screenGotoxy(startX, yDown);
                printf("|");
                fflush(stdout);
                if (startX == player.x && yDown == player.y) {
                    handlePlayerHit();
                    break;
                }
            }
        }

        // Tiros diagonais
        else if (direction == 2) {
            // Diagonal principal (\)
            int xRightDown = startX + i;
            int yRightDown = startY + i;
            if (xRightDown >= 0 && xRightDown < MAP_WIDTH && yRightDown >= 0 && yRightDown < MAP_HEIGHT && maps[mapIndex][yRightDown][xRightDown] != '#') {
                screenGotoxy(xRightDown, yRightDown);
                printf("\\");
                fflush(stdout);
                if (xRightDown == player.x && yRightDown == player.y) {
                    handlePlayerHit();
                    break;
                }
            }

            int xLeftUp = startX - i;
            int yLeftUp = startY - i;
            if (xLeftUp >= 0 && xLeftUp < MAP_WIDTH && yLeftUp >= 0 && yLeftUp < MAP_HEIGHT && maps[mapIndex][yLeftUp][xLeftUp] != '#') {
                screenGotoxy(xLeftUp, yLeftUp);
                printf("\\");
                fflush(stdout);
                if (xLeftUp == player.x && yLeftUp == player.y) {
                    handlePlayerHit();
                    break;
                }
            }

            // Diagonal secundária (/)
            int xRightUp = startX + i;
            int yRightUp = startY - i;
            if (xRightUp >= 0 && xRightUp < MAP_WIDTH && yRightUp >= 0 && yRightUp < MAP_HEIGHT && maps[mapIndex][yRightUp][xRightUp] != '#') {
                screenGotoxy(xRightUp, yRightUp);
                printf("/");
                fflush(stdout);
                if (xRightUp == player.x && yRightUp == player.y) {
                    handlePlayerHit();
                    break;
                }
            }

            int xLeftDown = startX - i;
            int yLeftDown = startY + i;
            if (xLeftDown >= 0 && xLeftDown < MAP_WIDTH && yLeftDown >= 0 && yLeftDown < MAP_HEIGHT && maps[mapIndex][yLeftDown][xLeftDown] != '#') {
                screenGotoxy(xLeftDown, yLeftDown);
                printf("/");
                fflush(stdout);
                if (xLeftDown == player.x && yLeftDown == player.y) {
                    handlePlayerHit();
                    break;
                }
            }
        }

        usleep(5000);  // Pequeno atraso entre cada tiro
    }

    // Redesenha o mapa após o disparo
    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawDrops();
    drawBoss(tanque.x, tanque.y);
}

void tripleBossShoot(int direction) {
    int startX = tanque.x;
    int startY = tanque.y;
    int range = 30;
    int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    char bulletChar;

    // Configuração das direções e caracteres dos tiros
    switch (direction) {
        case 1:  // Horizontal (esquerda e direita)
            dx1 = 1; dy1 = 0; bulletChar = '-';
            dx2 = -1; dy2 = 0;
            break;
        case 2:  // Vertical (cima e baixo)
            dx1 = 0; dy1 = 1; bulletChar = '|';
            dx2 = 0; dy2 = -1;
            break;
        case 3:  // Diagonal principal (\)
            dx1 = 1; dy1 = 1; bulletChar = '\\';
            dx2 = -1; dy2 = -1;
            break;
        case 4:  // Diagonal secundária (/)
            dx1 = 1; dy1 = -1; bulletChar = '/';
            dx2 = -1; dy2 = 1;
            break;
    }

    screenSetColor(RED, BLACK);  // Configuração de cor para o tiro

    if (direction == 2) {
        for (int i = 1; i <= range; i++) {
            int y1a = startY + i * dy1;  // Primeiro tiro na direção dy1 (baixo)
            int y1b = startY;            // Tiro central
            int y1c = startY - i * dy1;  // Primeiro tiro na direção oposta (cima)

            int x1 = startX - 1;         // Tiro na posição x da esquerda do tanque
            int x2 = startX + 1;         // Tiro na posição x da direita do tanque

            // Desenho e verificação de limites para tiros na primeira direção (baixo)
            if (y1a >= 0 && y1a < MAP_HEIGHT) {
                if (x1 >= 0 && x1 < MAP_WIDTH && maps[mapIndex][y1a][x1] != '#') {
                    screenGotoxy(x1, y1a);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x1 == player.x && y1a == player.y) { handlePlayerHit(); break; }
                }
                if (startX >= 0 && startX < MAP_WIDTH && maps[mapIndex][y1a][startX] != '#') {
                    screenGotoxy(startX, y1a);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (startX == player.x && y1a == player.y) { handlePlayerHit(); break; }
                }
                if (x2 >= 0 && x2 < MAP_WIDTH && maps[mapIndex][y1a][x2] != '#') {
                    screenGotoxy(x2, y1a);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x2 == player.x && y1a == player.y) { handlePlayerHit(); break; }
                }
            }

            // Desenho e verificação de limites para tiros na direção oposta (cima)
            if (y1c >= 0 && y1c < MAP_HEIGHT) {
                if (x1 >= 0 && x1 < MAP_WIDTH && maps[mapIndex][y1c][x1] != '#') {
                    screenGotoxy(x1, y1c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x1 == player.x && y1c == player.y) { handlePlayerHit(); break; }
                }
                if (startX >= 0 && startX < MAP_WIDTH && maps[mapIndex][y1c][startX] != '#') {
                    screenGotoxy(startX, y1c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (startX == player.x && y1c == player.y) { handlePlayerHit(); break; }
                }
                if (x2 >= 0 && x2 < MAP_WIDTH && maps[mapIndex][y1c][x2] != '#') {
                    screenGotoxy(x2, y1c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x2 == player.x && y1c == player.y) { handlePlayerHit(); break; }
                }
            }

            usleep(5000);  // Pequeno atraso entre cada tiro
        }
    } else {
          for (int i = 1; i <= range; i++) {
            int x1 = startX + i * dx1;
            int y1a = startY - 1 + i * dy1;
            int y1b = startY + i * dy1;
            int y1c = startY + 1 + i * dy1;

            int x2 = startX + i * dx2;
            int y2a = startY - 1 + i * dy2;
            int y2b = startY + i * dy2;
            int y2c = startY + 1 + i * dy2;

            // Desenho e verificação de limites para tiros na primeira direção
            if (x1 >= 0 && x1 < MAP_WIDTH) {
                if (y1a >= 0 && y1a < MAP_HEIGHT && maps[mapIndex][y1a][x1] != '#') {
                    screenGotoxy(x1, y1a);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x1 == player.x && y1a == player.y) { handlePlayerHit(); break; }
                }
                if (y1b >= 0 && y1b < MAP_HEIGHT && maps[mapIndex][y1b][x1] != '#') {
                    screenGotoxy(x1, y1b);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x1 == player.x && y1b == player.y) { handlePlayerHit(); break; }
                }
                if (y1c >= 0 && y1c < MAP_HEIGHT && maps[mapIndex][y1c][x1] != '#') {
                    screenGotoxy(x1, y1c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x1 == player.x && y1c == player.y) { handlePlayerHit(); break; }
                }
            }

            // Desenho e verificação de limites para tiros na direção oposta
            if (x2 >= 0 && x2 < MAP_WIDTH) {
                if (y2a >= 0 && y2a < MAP_HEIGHT && maps[mapIndex][y2a][x2] != '#') {
                    screenGotoxy(x2, y2a);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x2 == player.x && y2a == player.y) { handlePlayerHit(); break; }
                }
                if (y2b >= 0 && y2b < MAP_HEIGHT && maps[mapIndex][y2b][x2] != '#') {
                    screenGotoxy(x2, y2b);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x2 == player.x && y2b == player.y) { handlePlayerHit(); break; }
                }
                if (y2c >= 0 && y2c < MAP_HEIGHT && maps[mapIndex][y2c][x2] != '#') {
                    screenGotoxy(x2, y2c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if (x2 == player.x && y2c == player.y) { handlePlayerHit(); break; }
                }
            }

            usleep(5000);  // Pequeno atraso entre cada tiro
        }
    }


    // Redesenha o mapa após o disparo
    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawDrops();
    drawBoss(tanque.x, tanque.y);
}



// Função para gerenciar o impacto do tiro no jogador
void handlePlayerHit() {
    screenSetColor(RED, BLACK);
    screenGotoxy(player.x, player.y);
    printf("ඞ");
    fflush(stdout);
    usleep(100000);
    player.health--;
    drawHUD();
    if (player.health <= 0) {
        printf("Game Over!\n");
        exit(0);
    }
}

void moveBoss() {
    if (tanque.cooldown > 0) {
        tanque.cooldown--;
        return;
    }

    if (tanque.move == 1) {
        if (tanque.tick == 3 || tanque.tick == 5 || tanque.tick == 7) {
            bossShoot(1);  // Disparo nas horizontais e verticais
        } else if (tanque.tick == 4 || tanque.tick == 6 || tanque.tick == 8) {
            bossShoot(2);  // Disparo nas diagonais
        } else if (tanque.tick == 9) {
            tanque.cooldown = 5;
            tanque.tick = 0;
            tanque.move = 2;
        }
        tanque.tick += 1;

    } else if (tanque.move == 2) {
        tanque.cooldown = 5;
        // Lógica adicional para o move 2

    } else if (tanque.move == 3) {
        if (tanque.tick == 3 || tanque.tick == 7 || tanque.tick == 11) {
            tripleBossShoot(1);  // Disparo nas horizontais
        } else if (tanque.tick == 4 || tanque.tick == 8) {
            tripleBossShoot(3);  // Disparo nas diagonais
        } else if (tanque.tick == 5 || tanque.tick == 9) {
            tripleBossShoot(2);  // Disparo nas horizontais
        } else if (tanque.tick == 6 || tanque.tick == 10) {
            tripleBossShoot(4);  // Disparo nas diagonais
        } else if (tanque.tick == 12) {
            tanque.cooldown = 5;
            tanque.tick = 0;
            //tanque.move = 4;
        }
        tanque.tick += 1;

    } else if (tanque.move == 4) {

    }

    drawBoss(tanque.x, tanque.y);
}

void spawnEnemies() {
    if (difftime(time(NULL), lastEnemySpawn) < ENEMY_RESPAWN_INTERVAL || mapIndex == 2) return;
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
                } else {
                    enemies[i].type = 2;
                    enemies[i].moves = 0;
                    enemies[i].playerDetected = 0;
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
    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
    }
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
    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
    }
}

void playerShotgunShoot(int dx, int dy) {
    if (!player.hasShotgun || player.ammo <= 0) return;

    int x = player.x + dx;
    int y = player.y + dy;
    int range = 5;  // A shotgun tem um alcance de 5
    int spread[] = {1, 3, 5, 5, 5};  // Define o número de blocos atingidos por cada "passo" da shotgun

    player.ammo--;

    screenSetColor(CYAN, BLACK);

    for (int step = 0; step < range; step++) {
        int spreadRadius = spread[step] / 2;  // Define a "largura" do tiro para a shotgun
        
        for (int offset = -spreadRadius; offset <= spreadRadius; offset++) {
            int targetX = x + (dy == 0 ? 0 : offset);  // Espalha horizontalmente, se dx != 0
            int targetY = y + (dx == 0 ? 0 : offset);  // Espalha verticalmente, se dy != 0

            // Verifica se a posição está dentro dos limites do mapa
            if (targetX < 0 || targetX >= MAP_WIDTH || targetY < 0 || targetY >= MAP_HEIGHT) continue;

            // Verifica colisão com paredes
            if (maps[mapIndex][targetY][targetX] == '#') continue;

            // Desenha o tiro na posição atual
            screenGotoxy(targetX, targetY);
            printf("*");  // Caracter representando o tiro da shotgun
            fflush(stdout);
            usleep(10000);  // Tempo reduzido para o efeito de espalhamento

            // Checa se o tiro atingiu algum inimigo
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].alive && enemies[i].x == targetX && enemies[i].y == targetY) {
                    enemies[i].alive = 0;
                    enemies_dead++;
                    screenGotoxy(targetX, targetY);
                    printf(" ");
                    int dropChance = rand() % 100;
                    if (dropChance < DROP_CHANCE) {
                        spawnDrop(enemies[i].x, enemies[i].y);
                    }
                    pontosGanhos = 100 * combo;
                    updateScore(100, 1);
                }
            }

            // Limpa o tiro na posição atual após breve exibição
            screenGotoxy(targetX, targetY);

            fflush(stdout);
        }

        // Avança o tiro da shotgun
        x += dx;
        y += dy;
    }

    // Atualiza o mapa e o jogador
    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawDrops();
    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
    }
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
            screenSetColor(RED, BLACK);
            screenGotoxy(player.x, player.y);
            printf("ඞ");
            fflush(stdout);
            usleep(100000);  // Mantém o feedback vermelho por 100ms
            player.health--;
            drawHUD();
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
    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
    }
}

void reload() {
    if (player.clips > 0 && player.ammo < MAX_AMMO) {
        player.ammo = MAX_AMMO;
        player.clips--;
        drawHUD();
    }
}