#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"


#define MAP_WIDTH 20
#define MAP_HEIGHT 10

// Cores para os elementos do mapa
#define COLOR_WALL RED
#define COLOR_DOOR YELLOW
#define COLOR_FLOOR LIGHTGRAY
#define COLOR_PLAYER GREEN

// Definição do mapa
char map[MAP_HEIGHT][MAP_WIDTH] = {
    "####################",
    "#    #     D      #",
    "#    #            #",
    "#    ####  #######",
    "#         D       #",
    "#    #######      #",
    "#       D         #",
    "###  ##############",
    "#                 #",
    "####################"
};

// Posição inicial do jogador
int playerX = 1;
int playerY = 1;

// Função para desenhar o mapa
void screenDrawMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = map[y][x];

            // Escolhe a cor para cada tipo de célula
            switch(cell) {
                case '#':
                    screenSetColor(COLOR_WALL, BLACK); break;
                case 'D':
                    screenSetColor(COLOR_DOOR, BLACK); break;
                default:
                    screenSetColor(COLOR_FLOOR, BLACK); break;
            }

            // Posiciona o cursor e desenha a célula
            screenGotoxy(x, y);
            printf("%c", cell);
        }
    }
    screenSetColor(WHITE, BLACK);  // Reset de cor
    fflush(stdout); // Garante que a tela seja atualizada
}

// Função para desenhar o jogador
void drawPlayer() {
    screenSetColor(COLOR_PLAYER, BLACK);
    screenGotoxy(playerX, playerY);
    printf("@");  // Caractere do jogador
    fflush(stdout); // Atualiza a tela imediatamente
}

// Função para mover o jogador
void movePlayer(int dx, int dy) {
    // Nova posição do jogador
    int newX = playerX + dx;
    int newY = playerY + dy;

    // Checa se a nova posição é válida (não é parede)
    if (map[newY][newX] != '#') {
        // Apaga a posição antiga
        screenGotoxy(playerX, playerY);
        printf(" ");

        // Atualiza a posição do jogador
        playerX = newX;
        playerY = newY;

        // Desenha o jogador na nova posição
        drawPlayer();
    }
}

int main() {
    keyboardInit();     // Inicializa o teclado
    screenInit(0);      // Inicializa a tela sem bordas

    screenDrawMap();    // Desenha o mapa na tela
    drawPlayer();       // Desenha o jogador na posição inicial

    while (1) {
        if (keyhit()) {
            char key = readch();

            // Move o jogador com base na tecla pressionada
            switch (key) {
                case 'w': movePlayer(0, -1); break;  // Cima
                case 's': movePlayer(0, 1); break;   // Baixo
                case 'a': movePlayer(-1, 0); break;  // Esquerda
                case 'd': movePlayer(1, 0); break;   // Direita
                case 'q':  // Tecla para sair do jogo
                    keyboardDestroy();
                    screenDestroy();
                    return 0;
            }
        }

        // Move o cursor para uma posição fixa para evitar "sobras" na tela
        screenGotoxy(0, MAP_HEIGHT);
        fflush(stdout); // Atualiza o conteúdo da tela
    }
}

