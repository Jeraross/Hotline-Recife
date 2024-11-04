#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "screen.h"
#include "keyboard.h"
#include <unistd.h>

#define MAP_WIDTH 40
#define MAP_HEIGHT 20
#define MAX_ENEMIES 5  // Número de inimigos

// Cores para os elementos do mapa
#define COLOR_WALL RED
#define COLOR_DOOR YELLOW
#define COLOR_FLOOR LIGHTGRAY
#define COLOR_PLAYER GREEN
#define COLOR_ENEMY MAGENTA
#define COLOR_ATTACK CYAN  // Cor para o feedback de ataque

// Definição do mapa ampliado
char **map;

void initMap() {
    map = (char**) malloc(MAP_HEIGHT * sizeof(char*));
    for (int i = 0; i < MAP_HEIGHT; i++) {
        map[i] = (char*) malloc(MAP_WIDTH * sizeof(char));
    }

    const char *mapData[MAP_HEIGHT] = {
        "########################################",
        "#    #     D      #              D    #",
        "#    #            #                   #",
        "#    ####  ###############  ########  #",
        "#         D            #              #",
        "#    #######           #              #",
        "#       D              #              #",
        "###  #######################  #########",
        "#                 #                   #",
        "#      ########   #       D           #",
        "###    #      #            #          #",
        "#      #      ##########   #   ####   #",
        "#   D          #           #      #   #",
        "######         #           #      #   #",
        "#              ####################### #",
        "#                      D              #",
        "#  ####   ####    ####   ####    #### #",
        "#                 #      #             #",
        "#                 #      #             #",
        "########################################"
    };
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            map[i][j] = mapData[i][j];
        }
    }
}

void freeMap() {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        free(map[i]);
    }
    free(map);
}

struct Player {
    int x, y;
    int hasWeapon;
    int alive;
};


struct Player player;

int weaponX = 20;
int weaponY = 10;
int hasWeapon = 0;

typedef struct {
    int x, y;
    int alive;
} Enemy;

Enemy enemies[MAX_ENEMIES] = {
    {5, 5, 1}, {8, 2, 1}, {15, 7, 1}, {30, 15, 1}, {35, 10, 1}
};

// Função para desenhar o mapa
void screenDrawMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = map[y][x];

            // Escolhe a cor para cada tipo de célula
            switch(cell) {
                case '#':
                    screenSetColor(COLOR_WALL, BLACK);
                screenGotoxy(x, y);
                printf("▣");
                break;
                case 'D':
                    screenSetColor(COLOR_DOOR, BLACK);
                screenGotoxy(x, y);
                printf("D");
                break;
                default:
                    screenSetColor(COLOR_FLOOR, BLACK);
                screenGotoxy(x, y);
                printf(" ");
                break;
            }
        }
    }
    screenSetColor(WHITE, BLACK);
    fflush(stdout);
}


void drawPlayer() {
    screenSetColor(COLOR_PLAYER, BLACK);
    screenGotoxy(player.x, player.y);
    printf("★");
    fflush(stdout);
}

// Função para desenhar os inimigos
void drawEnemies() {
    screenSetColor(COLOR_ENEMY, BLACK);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            screenGotoxy(enemies[i].x, enemies[i].y);
            printf("👾");
        }
    }
    fflush(stdout);
}

void drawWeapon() {
    if (!hasWeapon) {
        screenSetColor(YELLOW, BLACK);
        screenGotoxy(weaponX, weaponY);
        printf("🔫");  // W representa a arma
        fflush(stdout);
    }
}


// Função para mover o jogador
void movePlayer(int dx, int dy) {
    int newX = player.x + dx;
    int newY = player.y + dy;

    if (map[newY][newX] != '#') {
        screenGotoxy(player.x, player.y);
        printf(" ");

        player.x = newX;
        player.y = newY;

        // Verifica se o jogador pegou a arma
        if (player.x == weaponX && player.y == weaponY) {
            hasWeapon = 1;
            screenGotoxy(weaponX, weaponY);
            printf(" ");  // Remove a arma do mapa
        }

        drawPlayer();
    }
}




// Função para verificar se uma posição está ocupada por outro inimigo
int isOccupiedByEnemy(int x, int y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

// Função para mover inimigos em direção ao jogador
void moveEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        screenGotoxy(enemies[i].x, enemies[i].y);
        printf(" ");

        int newX = enemies[i].x;
        int newY = enemies[i].y;

        if (enemies[i].x < player.x && map[enemies[i].y][enemies[i].x + 1] != '#' && !isOccupiedByEnemy(enemies[i].x + 1, enemies[i].y)) {
            newX++;
        } else if (enemies[i].x > player.x && map[enemies[i].y][enemies[i].x - 1] != '#' && !isOccupiedByEnemy(enemies[i].x - 1, enemies[i].y)) {
            newX--;
        }

        if (enemies[i].y < player.y && map[enemies[i].y + 1][enemies[i].x] != '#' && !isOccupiedByEnemy(enemies[i].x, enemies[i].y + 1)) {
            newY++;
        } else if (enemies[i].y > player.y && map[enemies[i].y - 1][enemies[i].x] != '#' && !isOccupiedByEnemy(enemies[i].x, enemies[i].y - 1)) {
            newY--;
        }

        enemies[i].x = newX;
        enemies[i].y = newY;
    }
    drawEnemies();
}

// Função para exibir feedback visual do ataque ao redor do jogador
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

// Função para ataque com feedback visual
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

    usleep(200000);  // Aguarda para mostrar o feedback visual
    screenDrawMap();  // Redesenha o mapa para remover o feedback
    drawPlayer();
    drawEnemies();
    drawWeapon();
}

void playerShoot(int dx, int dy) {
    if (!hasWeapon) return;  // Só pode atirar se tiver a arma

    int x = player.x + dx;
    int y = player.y + dy;
    int range = 5;  // Alcance máximo do tiro

    // Define o caractere do tiro com base na direção
    char shotChar = (dx == 0) ? '|' : '-';  // '|' para tiro vertical, '-' para horizontal

    screenSetColor(CYAN, BLACK);  // Cor do tiro

    for (int step = 0; step < range; step++) {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT || map[y][x] == '#') {
            break;  // Sai do loop se o tiro atingir um obstáculo ou o limite do mapa
        }

        screenGotoxy(x, y);
        printf("%c", shotChar);  // Exibe o tiro
        fflush(stdout);
        usleep(50000);  // Pausa para a animação do tiro

        // Verifica se atingiu algum inimigo
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
                enemies[i].alive = 0;
                screenGotoxy(x, y);
                printf(" ");  // Remove o inimigo atingido
                return;  // Termina o disparo após atingir um inimigo
            }
        }

        // Apaga o caractere de tiro da célula anterior
        screenGotoxy(x, y);
        printf(" ");
        fflush(stdout);

        // Move o tiro para a próxima posição
        x += dx;
        y += dy;
    }

    screenDrawMap();  // Redesenha o mapa para remover o último tiro
    drawPlayer();
    drawEnemies();
}



int main() {
    keyboardInit();
    screenInit(0);

    initMap();

    screenDrawMap();
    player.x = 1;
    player.y = 1;
    drawPlayer();
    drawEnemies();
    drawWeapon();

    time_t lastEnemyMove = time(NULL);

    while (1) {
        if (keyhit()) {
            char key = readch();

            switch (key) {
                case 'w': movePlayer(0, -1); break;
                case 's': movePlayer(0, 1); break;
                case 'a': movePlayer(-1, 0); break;
                case 'd': movePlayer(1, 0); break;
                case ' ': playerAttack(); break;
                case 'i': playerShoot(0, -1); break;  // Tiro para cima
                case 'k': playerShoot(0, 1); break;   // Tiro para baixo
                case 'j': playerShoot(-1, 0); break;  // Tiro para a esquerda
                case 'l': playerShoot(1, 0); break;   // Tiro para a direita
                case 'q':
                    keyboardDestroy();
                screenDestroy();
                return 0;
            }
        }

        // Movimento dos inimigos e atualização da tela
        if (difftime(time(NULL), lastEnemyMove) >= 1) {
            moveEnemies();
            lastEnemyMove = time(NULL);
        }

        screenGotoxy(0, MAP_HEIGHT);
        fflush(stdout);
    }

    free(map);
}
