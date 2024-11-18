#ifndef __SCOREBOARD_H__
#define __SCOREBOARD_H__

#include "menus.h"

struct winners {
    char nome[20];
    int score;
    struct winners *next;
};


extern struct winners *head; 

void loadScoreboard();

void add_score(struct winners **head, char *nome, int score);

void printScoreboard(struct winners *head);

void writeScoreboard();

#endif /* __SCOREBOARD_H__ */