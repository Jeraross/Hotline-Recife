#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scoreboard.h"


struct winners *head = NULL;

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