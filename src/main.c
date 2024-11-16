#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"
#include "map.h"
#include "menus.h"
#include "minigame.h"
#include "characters.h"

#define MAP_WIDTH 55
#define MAP_HEIGHT 21
#define NUM_MAPS 3
#define MAX_ENEMIES 8
#define MAX_AMMO 5
#define PLAYER_MAX_HEALTH 5
#define ENEMY_RESPAWN_INTERVAL 3
#define ENEMY_COOLDOWN_PERIOD 5  // Tempo que o inimigo fica parado ao colidir com o jogador
#define MAX_CLIPS 1
#define DROP_CHANCE 30
#define COMBO_HUD_X 0
#define COMBO_HUD_Y MAP_HEIGHT + 1
#define BOSS_HEALTH_BAR_LENGTH 50  // Comprimento da barra de vida do chefe (ajuste conforme o design desejado)
#define BOSS_HEALTH_BAR_X 0
#define BOSS_HEALTH_BAR_Y MAP_HEIGHT + 2

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
    int mask; //0 = Galo, 1 = Leao, 2 = Timbu
    char name[20];
} Player;


Player player = {2, 2, PLAYER_MAX_HEALTH, 0, 0, 5, 1}; // Inicializando o jogador

typedef struct {
    int x, y;
    int alive;
    int cooldown;
    int type;
    int moves; // atirador
    int playerDetected;
    int px, py;
} Enemy;

Enemy enemies[MAX_ENEMIES] = {{0}};

typedef struct {
    int x, y; //core matriz
    int health;
    int cooldown;
    int move;
    int tick;
} Boss;

Boss tanque = {MAP_WIDTH / 2, MAP_HEIGHT / 2, 50, 0, 1, 0};

struct winners {
    char nome[20];
    int score;
    struct winners *next;
};

struct winners *head = NULL;

time_t lastEnemySpawn;
time_t lastDropSpawn;
time_t comboStartTime;
clock_t lastEnemyMove;

int score = 0, combo = 1, pontosGanhos = 0, px = 0, py = 0, enemies_dead;

int comboColors[] = {COLOR_COMBO1, COLOR_COMBO2, COLOR_COMBO3};
int comboColorIndex = 0;

int bossWidth = 9; // Largura do tanque
int bossHeight = 3; // Altura do tanque

int lastdx = 0, lastdy = 0;

int porta_x, porta_y;
int mapIndex;

int powerCooldown = 20000;           // Cooldown de 20 segundos em milissegundos
int powerActivatedTime = -1;         // Tempo em que o poder foi ativado
int printActivatedTime = -1;         // Tempo em que a mensagem foi exibida

int ghostMode = 0;               // Indica se o jogador está em modo fantasma
long ghostModeActivatedTime = -1; // Tempo de ativação do modo fantasma
int ghostModeDuration = 5000;    // Duração do modo fantasma em milissegundos

// Array de músicas, cada mapa terá sua música correspondente
const char* mapMusicFiles[NUM_MAPS] = {
    "music/LAPERV.mp3",  // Música para o mapa 0
    "music/LAPERV.mp3",  // Música para o mapa 1
    "music/ROLLER.mp3"   // Música para o mapa 2
};

// Música do Menu e Minigame
const char* menuMusicFile = "music/MENU.mp3";  // Música do menu
const char* minigameMusicFile = "music/MINIGAME.mp3";  // Música do minigame
const char* GAMEOVERR = "music/SADGAME.mp3";  // Música do menu

int initSDL();
bool switchMusic(Mix_Music** currentMusic, const char* newMusicFile, int loops);

void drawHUD();

void drawPlayer();
void movePlayer(int dx, int dy);
void showAttackFeedback();
void playerAttack();
void throwWeapon(int lastdx, int lastdy);
void playerShoot(int dx, int dy);
void playerShotgunShoot(int dx, int dy);
void reload();

void drawEnemies();
void moveEnemies();
void spawnEnemies();
void enemyShoot(int enemyIndex, int plx, int ply);
int isOccupiedByEnemy(int x, int y);

void drawBoss(int x, int y);
void moveBoss();
void bossShoot(int direction);
void tripleBossShoot(int direction);
void bossShockwave();
void drawBossHealthBar();

void handlePlayerHit();

void drawDrops();
void spawnDrop(int x, int y);

void drawComboHUD();
void updateScore(int points, int isEnemyKill);

void drawDoor();
int doorVerify();

void add_score(struct winners **head, char *nome, int score);
void loadScoreboard();
void printScoreboard();
void writeScoreboard();
void activePower();


int main() {
    if (initSDL() != 0) {
        return -1;
    }

    Mix_Music* currentMusic = NULL;
    keyboardInit();
    screenInit(0);
    srand(time(NULL));

    if (!switchMusic(&currentMusic, menuMusicFile, -1)) {
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    displayMenu();

    player.mask = navigate_masks();

    displayOpeningArt();

    mapIndex = 0;
    player.ammo = MAX_AMMO;
    player.hasWeapon = 0;
    player.hasShotgun = 0;
    player.currentWeapon = -1;

    if (player.mask == 1) {
        player.health = PLAYER_MAX_HEALTH;
    }
    else player.health = 3;

    if (player.mask == 0) player.clips = 3;
    else player.clips = 1;

    if (!switchMusic(&currentMusic, mapMusicFiles[mapIndex], -1)) {
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawHUD();
    drawDrops();

    lastEnemyMove = clock();
    lastEnemySpawn = time(NULL);
    lastDropSpawn = time(NULL);
    porta_x = MAP_WIDTH - 2;
    porta_y = 10;
    enemies_dead = 0;

    while (1) {
        if (player.health <= 0) {
            if (!switchMusic(&currentMusic, GAMEOVERR, 0)) {
                Mix_CloseAudio();
                SDL_Quit();
                return 1;
            }
            char final;
            gameover();
            scanf("%c", &final);
            if (final == '\n') {
                screenDestroy();
                keyboardDestroy();
                exit(0);
            }
        }
        if (keyhit()) {
            char key = readch();

            switch (key) {
                case 'w': movePlayer(0, -1); break;
                case 's': movePlayer(0, 1); break;
                case 'a': movePlayer(-1, 0); break;
                case 'd': movePlayer(1, 0); break;
                case ' ': playerAttack(); break;
                case 'h':
                  if (player.hasWeapon || player.hasShotgun) {
                    if (player.currentWeapon == 0) {
                      throwWeapon(lastdx, lastdy);
                      player.hasWeapon = 0;
                    } else if (player.currentWeapon == 1) {
                      throwWeapon(lastdx, lastdy);
                      player.hasShotgun = 0;
                    }
                  }
                  break;
                case 'i': if (player.currentWeapon == 1) playerShotgunShoot(0, -1); else playerShoot(0, -1); break;
                case 'k': if (player.currentWeapon == 1) playerShotgunShoot(0, 1); else playerShoot(0, 1); break;
                case 'j': if (player.currentWeapon == 1) playerShotgunShoot(-1, 0); else playerShoot(-1, 0); break;
                case 'l': if (player.currentWeapon == 1) playerShotgunShoot(1, 0); else playerShoot(1, 0); break;
                case 'u': if (player.mask == 2) playerShoot(-1, -1); break;   // Diagonal superior esquerda
                case 'o': if (player.mask == 2) playerShoot(1, -1); break;   // Diagonal superior direita
                case 'm': if (player.mask == 2) playerShoot(-1, 1); break;   // Diagonal inferior esquerda
                case ',': if (player.mask == 2) playerShoot(1, 1); break;    // Diagonal inferior direita
                case 'r': reload(); break;
                case 't':
                    if (player.hasWeapon && player.hasShotgun) {
                        player.currentWeapon = 1 - player.currentWeapon; // Alterna entre 0 e 1
                    }
                    break;
                case 'q': activePower(); break;
                case '.':
                    keyboardDestroy();
                    screenDestroy();
                    return 0;
            }
        }
        if (ghostMode && getTimeDiff() - ghostModeActivatedTime >= ghostModeDuration) {
            ghostMode = 0; // Desativa o modo fantasma
            ghostModeActivatedTime = -1; // Reseta o tempo de ativação
            screenDrawMap(mapIndex);
            drawPlayer();
            drawEnemies();
            drawDrops();
            if (mapIndex == 2) {
                drawBoss(tanque.x, tanque.y);
                drawBossHealthBar();
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
            if (difftime(time(NULL), lastDropSpawn) >= 15) {
                  spawnDrop(-1, -1);
                  lastDropSpawn = time(NULL);
            }
        }

        spawnEnemies();
        drawComboHUD();  // Atualiza o HUD do combo com cor e tempo
        drawHUD();
        long tempo_atual = getTimeDiff();
        drawDrops();

        // Verificar se o tempo do combo acabou
        if (combo >= 2 && difftime(time(NULL), comboStartTime) >= 5) {
            combo = 1;  // Reseta o combo se o tempo acabou
        }

        drawDoor();

        int verifyDoor = doorVerify();

        if (verifyDoor == 1){
            // Reinicia o array de inimigos
            memset(enemies, 0, sizeof(enemies)); // Zera todos os inimigos

            // Reinicia o array de drops
            memset(drops, 0, sizeof(drops)); // Zera todos os drops

            if (!switchMusic(&currentMusic, minigameMusicFile, 0)) {
                Mix_CloseAudio();
                SDL_Quit();
                return 1;
            }
            startMinigame();

            powerActivatedTime = -1; // Reseta o tempo de ativação do poder
            printActivatedTime = -1; // Reseta o tempo de exibição da mensagem

            if (!switchMusic(&currentMusic, mapMusicFiles[mapIndex], -1)) {
                Mix_CloseAudio();
                SDL_Quit();
                return 1;
            }

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
            drawDrops();
            if (mapIndex == 2) {
                drawBoss(tanque.x, tanque.y);
                drawBossHealthBar();
            }
        }
        else if (verifyDoor == 2){
            if (!switchMusic(&currentMusic, menuMusicFile, 0)) {
                Mix_CloseAudio();
                SDL_Quit();
                return 1;
            }
            screenClear();
            displayEndGame(player.name, sizeof(player.name));
            printf("Parabéns, %s!\n", player.name);

            loadScoreboard();
            add_score(&head, player.name, score);
            printScoreboard(head);
            writeScoreboard();

            printf("Pressione Enter para continuar...\n");
            getchar();

            printCreditsArt();
            printf("Pressione Enter para sair...\n");
            getchar();

            keyboardDestroy();
            screenDestroy();
            return 0;
        }
    }
}

int initSDL() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        return -1;
    }

    if (Mix_Init(MIX_INIT_MP3) != MIX_INIT_MP3) {
        return -1;
    }

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        return -1;
    }

    // Configura volume para um valor mais baixo
    Mix_VolumeMusic(MIX_MAX_VOLUME / 6);

    return 0;
}

// Função para trocar a música
bool switchMusic(Mix_Music** currentMusic, const char* newMusicFile, int loops) {
    // Se uma música está tocando, para e libera a atual
    if (*currentMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(*currentMusic);
        *currentMusic = NULL;
    }

    // Carrega a nova música
    *currentMusic = Mix_LoadMUS(newMusicFile);
    if (!*currentMusic) {
        return false;
    }

    // Reproduz a nova música
    if (Mix_PlayMusic(*currentMusic, loops) == -1) {
        Mix_FreeMusic(*currentMusic);
        *currentMusic = NULL;
        return false;
    }

    return true;
}

void drawHUD() {
    screenGotoxy(0, MAP_HEIGHT);
    screenSetColor(WHITE, BLACK);

    // Define o nome da arma atual do jogador
    const char *currentWeaponName = (!player.hasWeapon && !player.hasShotgun) ? "       " :
                                    (player.currentWeapon == 0) ? "Pistola" :
                                    (player.currentWeapon == 1) ? "Shotgun" : "       ";

    // Calcula o tempo restante do cooldown
    int cooldownRemaining = 0;
    if (powerActivatedTime != -1) {
        long currentTime = getTimeDiff();
        cooldownRemaining = (powerCooldown - (currentTime - powerActivatedTime)) / 1000;
        if (cooldownRemaining < 0) cooldownRemaining = 0;  // Evita valores negativos
    }

    printf("Vida: %d  Munição: %d/%d  Score: %d  Arma: %s  Poder: %d seg ",
           player.health, player.ammo, player.clips, score, currentWeaponName, cooldownRemaining);
}



void drawComboHUD() {
    if (combo < 2) {
        // Limpa o HUD do combo quando o combo termina
        screenGotoxy(COMBO_HUD_X, COMBO_HUD_Y);
        printf("                      "); // Apaga o texto do combo
        fflush(stdout);
        return;
    }

    // Alternar cor
    comboColorIndex = (comboColorIndex + 1) % 3;  // Cíclico entre as três cores
    screenSetColor(comboColors[comboColorIndex], BLACK);

    screenGotoxy(COMBO_HUD_X, COMBO_HUD_Y);
    int remainingTime = 5 - (int)difftime(time(NULL), comboStartTime);
    printf("COMBO X%d   TIMER %d ", combo, remainingTime);

    fflush(stdout);
}

void drawPlayer() {
    screenSetColor(COLOR_PLAYER, BLACK);
    screenGotoxy(player.x, player.y);
   if (player.mask == 0) 
    printf("🐔");  // Representação do  com "C"
else if (player.mask == 1) 
    printf("🦁");  // Representação do  com "L"
else if (player.mask == 2) 
    printf("🐭");  // Representação do  com "F"
    fflush(stdout);
}

void drawEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            screenGotoxy(enemies[i].x, enemies[i].y);

            if (enemies[i].cooldown > 0) {
                screenSetColor(COLOR_ENEMY_HIT, BLACK);
            }
            else if (enemies[i].moves <= 4 && enemies[i].moves != 0 && (enemies[i].type == 2 || enemies[i].type == 1)) {
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

    if (tanque.health <= 0) {
        // Segunda linha da matriz do tanque destruído
        screenGotoxy(x - 4, y - 1);
        printf("   ( )=_ ");

        // Terceira linha da matriz do tanque destruído
        screenGotoxy(x - 4, y);
        printf("/\\/~-~-~\\");

        // Quarta linha da matriz do tanque destruído
        screenGotoxy(x - 4, y + 1);
        printf("\\0.0.0.==:");
    } else {
        // Segunda linha da matriz do boss
        screenGotoxy(x - 4, y - 1);
        printf(" __( )===:");

        // Terceira linha da matriz do boss
        screenGotoxy(x - 4, y);
        printf("/~-~-~-~\\");

        // Quarta linha da matriz do boss
        screenGotoxy(x - 4, y + 1);
        printf("\\O.O.O.O/");
    }


    // Limpa o buffer para exibir imediatamente
    fflush(stdout);
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
            } else if (drops[i].type == 3) {
                screenSetColor(YELLOW, BLACK);
                printf("S"); // Drop de shotgun
            } else if (drops[i].type == 4) {
                screenSetColor(YELLOW, BLACK);
                printf("P"); // Drop de pistola
            }

            fflush(stdout);
        }
    }
}

void drawDoor() {
  	if (mapIndex == 2) {
	if (tanque.health > 0) return;
    screenGotoxy(porta_x, porta_y);
    screenSetColor(COLOR_DOOR, BLACK);
    printf("🚪");
    fflush(stdout);
  	} else if (mapIndex == 1 || mapIndex == 0) {
    if (enemies_dead < 10) return;
    screenGotoxy(porta_x, porta_y);
    screenSetColor(COLOR_DOOR, BLACK);
    printf("🚪");
    fflush(stdout);
	}
}

int doorVerify() {
    // Verifica se as condições para avançar de fase ou finalizar o jogo foram atingidas
    if (((enemies_dead >= 10 && (player.x == porta_x || player.x + 1 == porta_x) && player.y == porta_y)) || 
        ((tanque.health <= 0 && (player.x == porta_x || player.x + 1 == porta_x) && player.y == porta_y))) {
        
        mapIndex++;
        enemies_dead = 0;

        if (mapIndex >= NUM_MAPS) {
            return 2;
        } else {
            return 1;
        }
    }
    return 0;
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

            // Define o tipo de drop com base no tipo do inimigo
            if (enemies[i].type == 1) {
                drops[i].type = 3;  // Tipo 3 representa a shotgun
            } else if (enemies[i].type == 2) {
                drops[i].type = 4;  // Tipo 4 representa a pistola
            } else {
                drops[i].type = (rand() % 2) + 1;  // Randomiza munição ou vida
            }

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

    lastdx = dx;
    lastdy = dy;

    // Verifica colisão com o tanque somente no mapa 2
    int colideComTanque = 0;
    if (mapIndex == 2) {
        int tanqueX = tanque.x - bossWidth / 2;
        int tanqueY = tanque.y - bossHeight / 2;

        colideComTanque = (newX >= tanqueX && newX < tanqueX + bossWidth &&
                           newY >= tanqueY && newY < tanqueY + bossHeight) ||
                          (newX + 1 >= tanqueX && newX + 1 < tanqueX + bossWidth &&
                           newY >= tanqueY && newY < tanqueY + bossHeight);
    }

    if (ghostMode == 1) {
        if ((maps[mapIndex][newY][newX] == '#') ||
        (maps[mapIndex][newY][newX + 1] == '#') &&
        !(maps[mapIndex][newY][newX] == maps[mapIndex][MAP_HEIGHT][MAP_WIDTH]) &&
        !(maps[mapIndex][newY][newX + 1] == maps[mapIndex][MAP_HEIGHT][MAP_WIDTH]) &&
        !(maps[mapIndex][newY][newX] == maps[mapIndex][1][1]) &&
        !(maps[mapIndex][newY][newX + 1] == maps[mapIndex][1][1])) {
            if (dx == -1) {
                dx = -2;
            }
            newX += dx;
            newY += dy;
        } else if (maps[mapIndex][newY][newX + 1] == '#' && dx == 1) {
            newX += 2;
        }
    }

    if ((maps[mapIndex][newY][newX] != '#') &&
        !isOccupiedByEnemy(newX, newY) &&
        (maps[mapIndex][newY][newX + 1] != '#') &&
        !isOccupiedByEnemy(newX + 1, newY) &&
        !colideComTanque) {

        screenGotoxy(player.x, player.y);
        printf(" ");

        player.x = newX;
        player.y = newY;

        // Verifica se o jogador coletou um drop
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (drops[i].active &&
               (drops[i].x == player.x || drops[i].x == player.x + 1) &&
                drops[i].y == player.y) {

                if (player.mask == 0) {
                    if (drops[i].type == 1 && player.clips < 3) {
                        player.clips++;
                    } else if (drops[i].type == 2 && player.health < 3) {
                        player.health++;
                    }
                } else if (player.mask == 1) {
                    if (drops[i].type == 1 && player.clips < MAX_CLIPS) {
                        player.clips++;
                    } else if (drops[i].type == 2 && player.health < PLAYER_MAX_HEALTH) {
                        player.health++;
                    }
                } else if (player.mask == 2) {
                    if (drops[i].type == 1 && player.clips < MAX_CLIPS) {
                        player.clips++;
                    } else if (drops[i].type == 2 && player.health < 3) {
                        player.health++;
                    }
                }

                if (drops[i].type == 3) {
                  player.hasShotgun = 1;
                  player.currentWeapon = 1;
                  if (player.ammo == MAX_AMMO && ((player.mask == 0) ? player.clips < 3 : player.clips > MAX_CLIPS)) {
                    player.clips++;
                  } else if (player.ammo < MAX_AMMO) {
                    player.ammo = MAX_AMMO;
                  }
                } else if (drops[i].type == 4) {
                  player.hasWeapon = 1;
                  player.currentWeapon = 0;
                  if (player.ammo == MAX_AMMO && ((player.mask == 0) ? player.clips < 3 : player.clips > MAX_CLIPS)) {
                    player.clips++;
                  } else if (player.ammo < MAX_AMMO) {
                    player.ammo = MAX_AMMO;
                  }
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
                    if ((x == player.x || x == player.x + 1) && enemies[i].y == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (int x = enemies[i].x - 1; x >= 0 && maps[mapIndex][enemies[i].y][x] != '#'; x--) {
                    if ((x == player.x || x == player.x + 1) && enemies[i].y == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }

                // Verificação Vertical (acima e abaixo)
                for (int y = enemies[i].y + 1; y < MAP_HEIGHT && maps[mapIndex][y][enemies[i].x] != '#'; y++) {
                    if ((enemies[i].x == player.x || enemies[i].x == player.x + 1) && y == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (int y = enemies[i].y - 1; y >= 0 && maps[mapIndex][y][enemies[i].x] != '#'; y--) {
                    if ((enemies[i].x == player.x || enemies[i].x == player.x + 1) && y == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }

                // Verificação Diagonal (cima-esquerda, cima-direita, baixo-esquerda, baixo-direita)
                int dx, dy;
                for (dx = 1, dy = 1; enemies[i].x + dx < MAP_WIDTH && enemies[i].y + dy < MAP_HEIGHT && maps[mapIndex][enemies[i].y + dy][enemies[i].x + dx] != '#'; dx++, dy++) {
                    if ((enemies[i].x + dx == player.x || enemies[i].x + dx == player.x + 1) && enemies[i].y + dy == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (dx = -1, dy = 1; enemies[i].x + dx >= 0 && enemies[i].y + dy < MAP_HEIGHT && maps[mapIndex][enemies[i].y + dy][enemies[i].x + dx] != '#'; dx--, dy++) {
                    if ((enemies[i].x + dx == player.x || enemies[i].x + dx == player.x + 1) && enemies[i].y + dy == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (dx = 1, dy = -1; enemies[i].x + dx < MAP_WIDTH && enemies[i].y + dy >= 0 && maps[mapIndex][enemies[i].y + dy][enemies[i].x + dx] != '#'; dx++, dy--) {
                    if ((enemies[i].x + dx == player.x || enemies[i].x + dx == player.x + 1) && enemies[i].y + dy == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
                for (dx = -1, dy = -1; enemies[i].x + dx >= 0 && enemies[i].y + dy >= 0 && maps[mapIndex][enemies[i].y + dy][enemies[i].x + dx] != '#'; dx--, dy--) {
                    if ((enemies[i].x + dx == player.x || enemies[i].x + dx == player.x + 1) && enemies[i].y + dy == player.y) {
                        enemies[i].px = player.x;
                        enemies[i].py = player.y;
                        enemies[i].playerDetected = 1;
                        break;
                    }
                }
            }

            if (enemies[i].moves == 3) {
                enemyShoot(i, enemies[i].px, enemies[i].py);  // Chama a função para o tiro
                enemies[i].cooldown = 5;
                enemies[i].moves = 0;  // Reseta
                enemies[i].playerDetected = 0;    // Reseta
            }
            // Preparar o ataque se o jogador foi detectado
            if (enemies[i].playerDetected) {
                enemies[i].moves++;  // Inicia a contagem para o tiro
            }
            if (enemies[i].playerDetected != 0) continue;

        }
        if (enemies[i].type == 1) {
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
            }

            if (enemies[i].moves == 3) {
                enemyShoot(i, enemies[i].px, enemies[i].py);  // Chama a função para o tiro
                enemies[i].cooldown = 5;
                enemies[i].moves = 0;  // Reseta
                enemies[i].playerDetected = 0;    // Reseta
            }
            // Preparar o ataque se o jogador foi detectado
            if (enemies[i].playerDetected) {
                enemies[i].moves++;  // Inicia a contagem para o tiro
            }
            if (enemies[i].playerDetected != 0) continue;

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

        if ((nextX == player.x || nextX == player.x + 1) && nextY == player.y) {
            handlePlayerHit();
            enemies[i].cooldown = ENEMY_COOLDOWN_PERIOD;
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
        case 5:  // Spread em quatro direções ortogonais (direita, esquerda, cima e baixo)
            screenSetColor(RED, BLACK);
            for (int i = 1; i <= range; i++) {
                // Tiros na direção direita (um central e dois laterais)
                int xDirCentral = startX + i;
                int yDirLateral1 = startY - 1;
                int yDirLateral2 = startY + 1;

                if (xDirCentral >= 0 && xDirCentral < MAP_WIDTH) {
                    if (maps[mapIndex][startY][xDirCentral] != '#') {
                        screenGotoxy(xDirCentral, startY);
                        printf("%c", '-');
                        fflush(stdout);
                        if (xDirCentral == player.x && startY == player.y) { handlePlayerHit(); break; }
                    }
                    if (yDirLateral1 >= 0 && yDirLateral1 < MAP_HEIGHT && maps[mapIndex][yDirLateral1][xDirCentral] != '#') {
                        screenGotoxy(xDirCentral, yDirLateral1);
                        printf("%c", '-');
                        fflush(stdout);
                        if (xDirCentral == player.x && yDirLateral1 == player.y) { handlePlayerHit(); break; }
                    }
                    if (yDirLateral2 >= 0 && yDirLateral2 < MAP_HEIGHT && maps[mapIndex][yDirLateral2][xDirCentral] != '#') {
                        screenGotoxy(xDirCentral, yDirLateral2);
                        printf("%c", '-');
                        fflush(stdout);
                        if (xDirCentral == player.x && yDirLateral2 == player.y) { handlePlayerHit(); break; }
                    }
                }

                // Tiros na direção esquerda (um central e dois laterais)
                int xEsqCentral = startX - i;

                if (xEsqCentral >= 0 && xEsqCentral < MAP_WIDTH) {
                    if (maps[mapIndex][startY][xEsqCentral] != '#') {
                        screenGotoxy(xEsqCentral, startY);
                        printf("%c", '-');
                        fflush(stdout);
                        if (xEsqCentral == player.x && startY == player.y) { handlePlayerHit(); break; }
                    }
                    if (yDirLateral1 >= 0 && yDirLateral1 < MAP_HEIGHT && maps[mapIndex][yDirLateral1][xEsqCentral] != '#') {
                        screenGotoxy(xEsqCentral, yDirLateral1);
                        printf("%c", '-');
                        fflush(stdout);
                        if (xEsqCentral == player.x && yDirLateral1 == player.y) { handlePlayerHit(); break; }
                    }
                    if (yDirLateral2 >= 0 && yDirLateral2 < MAP_HEIGHT && maps[mapIndex][yDirLateral2][xEsqCentral] != '#') {
                        screenGotoxy(xEsqCentral, yDirLateral2);
                        printf("%c", '-');
                        fflush(stdout);
                        if (xEsqCentral == player.x && yDirLateral2 == player.y) { handlePlayerHit(); break; }
                    }
                }

                // Tiros na direção baixo (um central e dois laterais)
                int yBaixoCentral = startY + i;
                int xLateral1 = startX - 1;
                int xLateral2 = startX + 1;

                if (yBaixoCentral >= 0 && yBaixoCentral < MAP_HEIGHT) {
                    if (maps[mapIndex][yBaixoCentral][startX] != '#') {
                        screenGotoxy(startX, yBaixoCentral);
                        printf("%c", '|');
                        fflush(stdout);
                        if (startX == player.x && yBaixoCentral == player.y) { handlePlayerHit(); break; }
                    }
                    if (xLateral1 >= 0 && xLateral1 < MAP_WIDTH && maps[mapIndex][yBaixoCentral][xLateral1] != '#') {
                        screenGotoxy(xLateral1, yBaixoCentral);
                        printf("%c", '|');
                        fflush(stdout);
                        if (xLateral1 == player.x && yBaixoCentral == player.y) { handlePlayerHit(); break; }
                    }
                    if (xLateral2 >= 0 && xLateral2 < MAP_WIDTH && maps[mapIndex][yBaixoCentral][xLateral2] != '#') {
                        screenGotoxy(xLateral2, yBaixoCentral);
                        printf("%c", '|');
                        fflush(stdout);
                        if (xLateral2 == player.x && yBaixoCentral == player.y) { handlePlayerHit(); break; }
                    }
                }

                // Tiros na direção cima (um central e dois laterais)
                int yCimaCentral = startY - i;

                if (yCimaCentral >= 0 && yCimaCentral < MAP_HEIGHT) {
                    if (maps[mapIndex][yCimaCentral][startX] != '#') {
                        screenGotoxy(startX, yCimaCentral);
                        printf("%c", '|');
                        fflush(stdout);
                        if (startX == player.x && yCimaCentral == player.y) { handlePlayerHit(); break; }
                    }
                    if (xLateral1 >= 0 && xLateral1 < MAP_WIDTH && maps[mapIndex][yCimaCentral][xLateral1] != '#') {
                        screenGotoxy(xLateral1, yCimaCentral);
                        printf("%c", '|');
                        fflush(stdout);
                        if (xLateral1 == player.x && yCimaCentral == player.y) { handlePlayerHit(); break; }
                    }
                    if (xLateral2 >= 0 && xLateral2 < MAP_WIDTH && maps[mapIndex][yCimaCentral][xLateral2] != '#') {
                        screenGotoxy(xLateral2, yCimaCentral);
                        printf("%c", '|');
                        fflush(stdout);
                        if (xLateral2 == player.x && yCimaCentral == player.y) { handlePlayerHit(); break; }
                    }
                }

                usleep(5000);  // Pequeno atraso entre cada rajada
            }
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
                    if ((x1 == player.x || x1 == player.x + 1) && y1a == player.y) { handlePlayerHit(); break; }
                }
                if (startX >= 0 && startX < MAP_WIDTH && maps[mapIndex][y1a][startX] != '#') {
                    screenGotoxy(startX, y1a);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((startX == player.x || startX == player.x + 1) && y1a == player.y) { handlePlayerHit(); break; }
                }
                if (x2 >= 0 && x2 < MAP_WIDTH && maps[mapIndex][y1a][x2] != '#') {
                    screenGotoxy(x2, y1a);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((x2 == player.x || x2 == player.x + 1) && y1a == player.y) { handlePlayerHit(); break; }
                }
            }

            // Desenho e verificação de limites para tiros na direção oposta (cima)
            if (y1c >= 0 && y1c < MAP_HEIGHT) {
                if (x1 >= 0 && x1 < MAP_WIDTH && maps[mapIndex][y1c][x1] != '#') {
                    screenGotoxy(x1, y1c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((x1 == player.x || x1 == player.x + 1) && y1c == player.y) { handlePlayerHit(); break; }
                }
                if (startX >= 0 && startX < MAP_WIDTH && maps[mapIndex][y1c][startX] != '#') {
                    screenGotoxy(startX, y1c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((startX == player.x || startX == player.x + 1) && y1c == player.y) { handlePlayerHit(); break; }
                }
                if (x2 >= 0 && x2 < MAP_WIDTH && maps[mapIndex][y1c][x2] != '#') {
                    screenGotoxy(x2, y1c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((x2 == player.x || x2 == player.x + 1) && y1c == player.y) { handlePlayerHit(); break; }
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
                    if ((x1 == player.x || x1 == player.x + 1) && y1a == player.y) { handlePlayerHit(); break; }
                }
                if (y1b >= 0 && y1b < MAP_HEIGHT && maps[mapIndex][y1b][x1] != '#') {
                    screenGotoxy(x1, y1b);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((x1 == player.x || x1 == player.x + 1) && y1b == player.y) { handlePlayerHit(); break; }
                }
                if (y1c >= 0 && y1c < MAP_HEIGHT && maps[mapIndex][y1c][x1] != '#') {
                    screenGotoxy(x1, y1c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((x1 == player.x || x1 == player.x + 1) && y1c == player.y) { handlePlayerHit(); break; }
                }
            }

            // Desenho e verificação de limites para tiros na direção oposta
            if (x2 >= 0 && x2 < MAP_WIDTH) {
                if (y2a >= 0 && y2a < MAP_HEIGHT && maps[mapIndex][y2a][x2] != '#') {
                    screenGotoxy(x2, y2a);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((x2 == player.x || x2 == player.x + 1) && y2a == player.y) { handlePlayerHit(); break; }
                }
                if (y2b >= 0 && y2b < MAP_HEIGHT && maps[mapIndex][y2b][x2] != '#') {
                    screenGotoxy(x2, y2b);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((x2 == player.x || x2 == player.x + 1) && y2b == player.y) { handlePlayerHit(); break; }
                }
                if (y2c >= 0 && y2c < MAP_HEIGHT && maps[mapIndex][y2c][x2] != '#') {
                    screenGotoxy(x2, y2c);
                    printf("%c", bulletChar);
                    fflush(stdout);
                    if ((x2 == player.x || x2 == player.x + 1) && y2c == player.y) { handlePlayerHit(); break; }
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
    drawBossHealthBar();
}

// Função para gerenciar o impacto do tiro no jogador
void handlePlayerHit() {
    screenSetColor(RED, BLACK);
    screenGotoxy(player.x, player.y);
    printf("░░");
    fflush(stdout);
    usleep(150000);
    drawPlayer();
    player.health--;
    drawHUD();
}

void drawBossHealthBar() {
    int healthSegments = (tanque.health * BOSS_HEALTH_BAR_LENGTH) / 50; // 50 é a nova vida máxima do tanque
    screenGotoxy(BOSS_HEALTH_BAR_X, BOSS_HEALTH_BAR_Y);

    // Escolhe a cor da barra com base na saúde do tanque
    if (tanque.health > 35) {
        screenSetColor(GREEN, BLACK);
    } else if (tanque.health > 15) {
        screenSetColor(YELLOW, BLACK);
    } else {
        screenSetColor(RED, BLACK);
    }

    // Desenha a barra de vida
    printf("[");
    for (int i = 0; i < BOSS_HEALTH_BAR_LENGTH; i++) {
        if (i < healthSegments) {
            printf("=");
        } else {
            printf(" ");
        }
    }
    printf("] %d/50", tanque.health);  // Exibe a saúde atual do chefe
    fflush(stdout);
}

void moveBoss() {
    if (tanque.cooldown > 0) {
        tanque.cooldown--;
        return;
    }

    if (tanque.health <= 0) {
        return;
    }

    if (tanque.move == 1) {
        if (tanque.tick == 3 || tanque.tick == 5 || tanque.tick == 7) {
            tripleBossShoot(5);
        } else if (tanque.tick == 9) {
            tanque.cooldown = 3;
            tanque.tick = 0;
            tanque.move = 2;
        }
        tanque.tick += 1;

    } else if (tanque.move == 2) {
        if (tanque.tick == 3 ) {
            bossShockwave();
        } else if (tanque.tick == 4) {
            tanque.cooldown = 3;
            tanque.tick = 0;
            tanque.move = 3;
        }
        tanque.tick += 1;

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
            tanque.cooldown = 3;
            tanque.tick = 0;
            tanque.move = 1;
        }
        tanque.tick += 1;

    }

    drawBoss(tanque.x, tanque.y);
    drawBossHealthBar();
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
            } while (isOccupiedByEnemy(spawnX, spawnY) || abs(spawnX - player.x) < 5 || abs(spawnY - player.y) < 5 || maps[mapIndex][spawnY][spawnX] == '#' || spawnX == porta_x && spawnY == porta_y);

            enemies[i].x = spawnX;
            enemies[i].y = spawnY;

            if (mapIndex == 0) {
                if (randomEnemy < 50) {
                    enemies[i].type = 0;
                } else {
                    enemies[i].type = 2;
                    enemies[i].moves = 0;
                    enemies[i].playerDetected = 0;
                }
            } else if (mapIndex == 1) {
                if (randomEnemy < 30) {
                    enemies[i].type = 0;
                } else if (randomEnemy < 60) {
                    enemies[i].type = 2;
                    enemies[i].moves = 0;
                    enemies[i].playerDetected = 0;
                } else {
                    enemies[i].type = 1;
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
        for (int dx = -1; dx <= 2; dx++) {
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

    if (mapIndex == 2) {
        // Verifica se o jogador está dentro da área de colisão do Boss Tanque
        if ((((player.x == tanque.x - 5 || player.x + 1 == tanque.x - 5) || player.x == tanque.x + 5) &&
     player.y >= tanque.y - 2 && player.y <= tanque.y + 2) ||
    ((player.y == tanque.y - 2 || player.y == tanque.y + 2) &&
     player.x >= tanque.x - 5 && player.x <= tanque.x + 5)) {
            // Reduz a vida do tanque
            tanque.health -= 1;  // Ajuste o dano conforme necessário
            if (tanque.health <= 0) {
                tanque.health = 0;
                // Ação quando o Boss Tanque morre
            }
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        // Verifica se o ataque atinge algum inimigo próximo
        if (abs(enemies[i].x - player.x) <= 2 && abs(enemies[i].y - player.y) <= 1) {
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
    drawDrops();

    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
        drawBossHealthBar();
    }
}

void throwWeapon(int lastdx, int lastdy) {
    if (!player.hasWeapon && !player.hasShotgun) return; // Não há arma para jogar

    // Define a arma atual que será jogada
    int currentWeaponToThrow = player.currentWeapon;

    // Coordenadas iniciais de onde a arma será lançada
    int x = (lastdx == 1) ? (player.x + lastdx + 1) : (player.x + lastdx);
    int y = player.y + lastdy;
    int range = 10; // Alcance máximo do lançamento da arma

    // Atualiza o estado do jogador para a arma jogada
    if (currentWeaponToThrow == 0) {
        player.hasWeapon = 0; // Remove a pistola
    } else {
        player.hasShotgun = 0; // Remove a shotgun
    }

    // Alterna para a outra arma se o jogador ainda tiver uma
    if (player.hasWeapon || player.hasShotgun) {
        player.currentWeapon = player.hasWeapon ? 0 : 1; // Troca para a arma que restou
    }

    // Array de caracteres para simular a rotação
    char rotation[] = {'|', '/', '-', '\\'};
    int rotationIndex = 0;

    screenSetColor(YELLOW, BLACK); // Cor para a animação do lançamento

    for (int step = 0; step < range; step++) {
        // Verifica se o lançamento ultrapassa os limites do mapa ou atinge uma parede
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT || maps[mapIndex][y][x] == '#') {
            break;
        }

        // Exibe o caractere de rotação da arma
        screenGotoxy(x, y);
        printf("%c", rotation[rotationIndex]);
        fflush(stdout);
        usleep(50000); // Controle da velocidade de rotação

        if (mapIndex == 2) {
            // Verifica se o tiro atingiu a área do tanque
            if (x >= tanque.x - bossWidth / 2 && x <= tanque.x + bossWidth / 2 &&
                y >= tanque.y - bossHeight / 2 && y <= tanque.y + bossHeight / 2) {
                // Dano ao tanque
                tanque.health -= 1;  // Ajuste conforme necessário
                if (tanque.health <= 0) {
                    tanque.health = 0;
                    // Se o Boss Tanque morrer, você pode fazer alguma ação, como ele desaparecer
                }
                break;
                }
        }

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

        // Limpa o caractere anterior e avança na direção de lançamento
        screenGotoxy(x, y);
        printf(" ");
        fflush(stdout);

        // Atualiza a rotação e a posição da arma
        rotationIndex = (rotationIndex + 1) % 4; // Alterna entre '|', '/', '-', '\\'
        x += lastdx;
        y += lastdy;
    }

    // Atualiza o mapa e as entidades
    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawDrops();
    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
        drawBossHealthBar();
    }
}

void playerShoot(int dx, int dy) {
    if (!player.hasWeapon || player.ammo <= 0) return;

    int x = (dx == 1 && dy == 0) ? 1 + player.x + dx : player.x + dx;
    int y = player.y + dy;
    int range = 30;

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
        usleep(12500);

        if (mapIndex == 2) {
            // Verifica se o tiro atingiu a área do tanque
            if (x >= tanque.x - bossWidth / 2 && x <= tanque.x + bossWidth / 2 &&
                y >= tanque.y - bossHeight / 2 && y <= tanque.y + bossHeight / 2) {
                // Dano ao tanque
                tanque.health -= 1;  // Ajuste conforme necessário
                if (tanque.health <= 0) {
                    tanque.health = 0;

                }
                break;
                }
        }

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
        drawBossHealthBar();
    }
}

void playerShotgunShoot(int dx, int dy) {
    if (!player.hasShotgun || player.ammo <= 0) return;

    int startX = (dx == 1 && dy == 0) ? 1 + player.x + dx : player.x + dx;
    int startY = player.y + dy;
    int range = 5;  // Alcance fixo da shotgun

    player.ammo--;

    screenSetColor(CYAN, BLACK);

    for (int i = 0; i < range; i++) {
        // Desenha os três tiros da linha atual
        for (int offset = -1; offset <= 1; offset++) {
            int x = startX + dx * i + (dy == 0 ? 0 : offset);  // Ajusta para tiros horizontais
            int y = startY + dy * i + (dx == 0 ? 0 : offset);  // Ajusta para tiros verticais

            // Verifica se a posição está dentro dos limites do mapa
            if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) continue;

            // Verifica colisão com paredes
            if (maps[mapIndex][y][x] == '#') continue;

            // Desenha o tiro na posição atual
            screenGotoxy(x, y);
            printf("*");
            fflush(stdout);

            // Checa se o tiro atingiu o Boss Tanque
            if (mapIndex == 2 &&
                x >= tanque.x - bossWidth / 2 && x <= tanque.x + bossWidth / 2 &&
                y >= tanque.y - bossHeight / 2 && y <= tanque.y + bossHeight / 2) {
                tanque.health -= 1;
                if (tanque.health <= 0) {
                    tanque.health = 0;
                    // Ação quando o Boss Tanque morre
                }
                break;
            }

            // Checa se o tiro atingiu algum inimigo
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].alive && enemies[j].x == x && enemies[j].y == y) {
                    enemies[j].alive = 0;
                    enemies_dead++;
                    screenGotoxy(x, y);
                    printf(" ");
                    int dropChance = rand() % 100;
                    if (dropChance < DROP_CHANCE) {
                        spawnDrop(enemies[j].x, enemies[j].y);
                    }
                    pontosGanhos = 100 * combo;
                    updateScore(100, 1);
                }
            }
        }

        // Pausa para exibir os tiros na linha atual antes de apagar
        usleep(25000);

        // Apaga a linha anterior para criar o efeito de movimento
        if (i > 0) {
            for (int offset = -1; offset <= 1; offset++) {
                int x = startX + dx * (i - 1) + (dy == 0 ? 0 : offset);
                int y = startY + dy * (i - 1) + (dx == 0 ? 0 : offset);

                // Verifica se a posição está dentro dos limites do mapa
                if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) continue;

                screenGotoxy(x, y);
                printf(" ");
            }
        }
        fflush(stdout);
    }

    // Atualiza o mapa e o jogador
    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawDrops();
    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
        drawBossHealthBar();
    }
}

void enemyShoot(int enemyIndex, int plx, int ply) {
    // Se o inimigo for do tipo shotgun (type 1), aplique a lógica de tiro da shotgun
    if (enemies[enemyIndex].type == 1) {
        int dx = (plx > enemies[enemyIndex].x) - (plx < enemies[enemyIndex].x);
        int dy = (ply > enemies[enemyIndex].y) - (ply < enemies[enemyIndex].y);

        int startX = (dx == 1 && dy == 0) ? 1 + enemies[enemyIndex].x + dx : enemies[enemyIndex].x + dx;
        int startY = enemies[enemyIndex].y + dy;
        int range = 5;  // Alcance fixo da espingarda do inimigo

        screenSetColor(RED, BLACK);

        for (int i = 0; i < range; i++) {
        // Desenha os três tiros da linha atual
            for (int offset = -1; offset <= 1; offset++) {
                int x = startX + dx * i + (dy == 0 ? 0 : offset);  // Ajusta para tiros horizontais
                int y = startY + dy * i + (dx == 0 ? 0 : offset);  // Ajusta para tiros verticais

            // Verifica se a posição está dentro dos limites do mapa
                if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) continue;

            // Verifica colisão com paredes
                if (maps[mapIndex][y][x] == '#') continue;

            // Desenha o tiro na posição atual
                screenGotoxy(x, y);
                printf("*");
                fflush(stdout);

            // Checa se o tiro atingiu o jogador
                if ((x == player.x || x == player.x + 1) && y == player.y) {
                    handlePlayerHit();
                    break;
                }
            }

        // Pausa para exibir os tiros na linha atual antes de apagar
            usleep(25000);

        // Apaga a linha anterior para criar o efeito de movimento
            if (i > 0) {
                for (int offset = -1; offset <= 1; offset++) {
                    int x = startX + dx * (i - 1) + (dy == 0 ? 0 : offset);
                    int y = startY + dy * (i - 1) + (dx == 0 ? 0 : offset);

                // Verifica se a posição está dentro dos limites do mapa
                    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) continue;

                    screenGotoxy(x, y);
                    printf(" ");
                }
            }
            fflush(stdout);
        }
    } else if (enemies[enemyIndex].type == 2) {
        // Caso seja um inimigo do tipo atirador normal, aplique a lógica original
        int dx = plx - enemies[enemyIndex].x;
        int dy = ply - enemies[enemyIndex].y;

        int range = 10;

        int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
        int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

        int x = enemies[enemyIndex].x + stepX;
        int y = enemies[enemyIndex].y + stepY;

        char shotChar;
        if (stepX == 0 || stepY == 0) {
            shotChar = (stepX == 0) ? '|' : '-';
        } else {
            shotChar = (stepX == stepY) ? '\\' : '/';
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

            if ((x == player.x || x == player.x + 1) && y == player.y) {
                handlePlayerHit();
                break;
            }

            screenGotoxy(x, y);
            printf(" ");
            fflush(stdout);

            x += stepX;
            y += stepY;
        }
    }

    // Atualiza o mapa e os elementos
    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawDrops();
    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
        drawBossHealthBar();
    }
}

void reload() {
    if (player.clips > 0 && player.ammo < MAX_AMMO) {
        player.ammo = MAX_AMMO;
        player.clips--;
        drawHUD();
    }
}

void bossShockwave() {
    int range = (MAP_WIDTH > MAP_HEIGHT) ? MAP_WIDTH : MAP_HEIGHT; // Define o alcance da onda
    int startX = tanque.x;
    int startY = tanque.y;



    for (int r = 1; r < range; r++) {
        // Expande ao redor do boss
        for (int dx = -r; dx <= r; dx++) {
            int x1 = startX + dx;
            int y1 = startY + (r - abs(dx));
            int y2 = startY - (r - abs(dx));

            // Ignorar as coordenadas especificadas
            if ((x1 >= 2 && x1 <= 10 || x1 >= 43 && x1 <= 54) && (y1 >= 4 && y1 <= 7 || y1 >= 14 && y1 <= 20)) {
                continue;
            }
            if ((x1 >= 2 && x1 <= 10|| x1>=43 && x1<=54 )  && (y2 >= 4 && y2 <= 7 || y2 >= 14 && y2 <= 20)) {
                continue;
            }

            // Verifica os limites do mapa antes de desenhar o shockwave
            if (x1 >= 0 && x1 < MAP_WIDTH && y1 >= 0 && y1 < MAP_HEIGHT && maps[mapIndex][y1][x1] != '#') {
                screenGotoxy(x1, y1);
                screenSetColor(RED, BLACK);

                printf("*"); // Símbolo do shockwave
                fflush(stdout);
                if ((x1 == player.x || x1 == player.x) && y1 == player.y) {
                    handlePlayerHit();
                }
            }
            if (x1 >= 0 && x1 < MAP_WIDTH && y2 >= 0 && y2 < MAP_HEIGHT && maps[mapIndex][y2][x1] != '#') {
                screenGotoxy(x1, y2);
                screenSetColor(RED, BLACK);
                printf("*");
                fflush(stdout);
                if ((x1 == player.x || x1 == player.x) && y2 == player.y) {
                    handlePlayerHit();
                }
            }
        }
        usleep(25000); // Atraso para o efeito de expansão
        screenDrawMap(mapIndex);
        drawPlayer();
        drawEnemies();
        drawDrops();
        drawBoss(tanque.x, tanque.y);
        drawBossHealthBar();
    }
}


void add_score(struct winners **head, char *nome, int score) {
    struct winners *n = *head;
    struct winners *novo = (struct winners*) malloc(sizeof(struct winners));
    struct winners *anterior = NULL;

    strcpy(novo->nome, nome);
    novo->score = score;
    novo->next = NULL;

    if (*head == NULL || (*head)->score < novo->score) {
        novo->next = *head;
        *head = novo;
        return;
    }

    while (n != NULL && n->score > novo->score) {
        anterior = n;
        n = n->next;
    }

    anterior->next = novo;
    novo->next = n;
}

void loadScoreboard() {
    FILE *list = fopen("scoreboard.txt", "r");
    if (!list) {
        printf("Erro ao abrir o arquivo scoreboard.\n");
        return;
    }

    char nome[20];
    int score;

    while (fscanf(list, "%s %d", nome, &score) == 2) {
        add_score(&head, nome, score);
    }

    fclose(list);
}

void printScoreboard(struct winners *head) {
    struct winners *n = head;
    int i = 1;

    printScoreboardArt();
    while (n != NULL && i <= 10) {
        printf("%dº - %s: %d pontos\n", i, n->nome, n->score);
        n = n->next;
        i++;
    }
    printf("\n");
}

void writeScoreboard() {
    FILE *list = fopen("scoreboard.txt", "w");
    if (!list) {
        printf("Não existe scoreboard.\n");
        return;
    }

    struct winners *n = head;

    while (n != NULL) {
        fprintf(list, "%s %d\n", n->nome, n->score);
        n = n->next;
    }

    fclose(list);
}

void activePower() {
    long currentTime = getTimeDiff(); // Obtém o tempo atual em milissegundos

    // Verifica se o poder está em cooldown
    if (powerActivatedTime != -1 && currentTime - powerActivatedTime < powerCooldown) {
        return; // Poder está em cooldown, então não faz nada
    }

    // Se o poder não está em cooldown, ativa-o e configura o tempo de ativação
    if (player.mask == 1) {
        int range = 5;
        int startX = player.x;
        int startY = player.y;

        for (int r = 1; r < range; r++) {
        // Expande ao redor do boss
            for (int dx = -r; dx <= r; dx++) {
                int x1 = startX + dx;
                int y1 = startY + (r - abs(dx));
                int y2 = startY - (r - abs(dx));

            // Verifica os limites do mapa antes de desenhar o shockwave
                if (x1 >= 0 && x1 < MAP_WIDTH && y1 >= 0 && y1 < MAP_HEIGHT && maps[mapIndex][y1][x1] != '#') {
                    screenGotoxy(x1, y1);
                    screenSetColor(WHITE, BLACK);
                    printf("*"); // Símbolo do shockwave
                    fflush(stdout);
                }
                if (x1 >= 0 && x1 < MAP_WIDTH && y2 >= 0 && y2 < MAP_HEIGHT && maps[mapIndex][y2][x1] != '#') {
                    screenGotoxy(x1, y2);
                    screenSetColor(WHITE, BLACK);
                    printf("*");
                    fflush(stdout);
                }
            }
            usleep(12500); // Atraso para o efeito de expansão
            screenDrawMap(mapIndex);
            drawPlayer();
            drawEnemies();
            drawDrops();
        }
        tanque.cooldown = ENEMY_COOLDOWN_PERIOD * 2;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].alive) {
                enemies[i].cooldown = ENEMY_COOLDOWN_PERIOD * 2;
            }
        }
        // Define o momento exato em que o poder foi ativado
        powerActivatedTime = currentTime;

} else if (player.mask == 0) {
    int dashRange = 20; // Distância máxima da investida
    int dashColors[] = {RED, GREEN, BLUE}; // Gradiente de cores
    int colorCount = sizeof(dashColors) / sizeof(dashColors[0]);
    int currentX = player.x;
    int currentY = player.y;

    char dashChar; // Inicializa como seta para direita

    // Define o símbolo com base na direção
    if (lastdx > 0 && lastdy == 0) dashChar = '>'; // Direita
    else if (lastdx < 0 && lastdy == 0) dashChar = '<'; // Esquerda
    else if (lastdx == 0 && lastdy > 0) dashChar = 'v'; // Baixo
    else if (lastdx == 0 && lastdy < 0) dashChar = '^'; // Cima

    // Executa a investida
    for (int step = 1; step <= dashRange; step++) {
        int nextX = currentX + lastdx * step;
        int nextY = currentY + lastdy * step;

        // Verifica se colidiu com uma parede ou está fora dos limites do mapa
        if (nextX < 0 || nextX >= MAP_WIDTH || nextY < 0 || nextY >= MAP_HEIGHT || maps[mapIndex][nextY][nextX] == '#' || maps[mapIndex][nextY][nextX + 1] == '#') {
            break; // Encerra o dash ao colidir
        }

        // Atualiza a posição do jogador
        player.x = nextX;
        player.y = nextY;

        // Deixa um rastro colorido no caminho
        int colorIndex = step % colorCount; // Alterna entre as cores
        screenGotoxy(nextX, nextY);
        screenSetColor(dashColors[colorIndex], BLACK);
        printf("%c", dashChar);
        fflush(stdout);

        // Verifica se atinge o Boss (mapIndex == 2)
        if (mapIndex == 2) {
            if ((((player.x == tanque.x - 5 || player.x + 1 == tanque.x - 5) || player.x == tanque.x + 5) &&
                 player.y >= tanque.y - 2 && player.y <= tanque.y + 2) ||
                ((player.y == tanque.y - 2 || player.y == tanque.y + 2) &&
                 player.x >= tanque.x - 5 && player.x <= tanque.x + 5)) {
                tanque.health -= 1; // Reduz a vida do tanque
                if (tanque.health <= 0) {
                    tanque.health = 0;
                }
                break;
            }
        }

        // Verifica se atinge algum inimigo
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].alive) continue;

            if (abs(enemies[i].x - player.x) <= 2 && abs(enemies[i].y - player.y) <= 1) {
                screenGotoxy(enemies[i].x, enemies[i].y);
                printf(" ");
                enemies[i].alive = 0;
                enemies_dead++;

                // Gera drop
                int dropChance = rand() % 100;
                if (dropChance < DROP_CHANCE) {
                    spawnDrop(enemies[i].x, enemies[i].y);
                }

                // Atualiza pontuação
                int pontosGanhos = 100 * combo;
                updateScore(pontosGanhos, 1);
            }
        }

        usleep(12500); // Atraso para suavizar o efeito do dash
    }

    // Desenha o jogador na posição final
    screenGotoxy(player.x, player.y);
    drawPlayer();

    // Atualiza o mapa após o dash
    screenDrawMap(mapIndex);
    drawPlayer();
    drawEnemies();
    drawDrops();

    // Atualiza o Boss se no mapa correto
    if (mapIndex == 2) {
        drawBoss(tanque.x, tanque.y);
        drawBossHealthBar();
    }

    // Define o tempo de ativação
    powerActivatedTime = currentTime;

    } else if (player.mask == 2) {
        ghostMode = 1;
        screenDrawMap(mapIndex);
        drawPlayer();
        drawEnemies();
        drawDrops();
        if (mapIndex == 2) {
            drawBoss(tanque.x, tanque.y);
            drawBossHealthBar();
        }
        ghostModeActivatedTime = currentTime; // Registra o tempo de ativação
        powerActivatedTime = currentTime;
    }
}