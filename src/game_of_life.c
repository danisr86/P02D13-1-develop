#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEIGHT 25
#define LENGTH 80

#define MAXRATE 2
#define MINRATE 0.1
#define STEP 0.1
#define STARTRATE 100000

void startState();
void initState(int mode);
void processState(int mode);
void fillState(char **array);
void outputState(char **array, WINDOW *win);
int updateState(char ***array, char ***buffer);

int countCellIsAlive(char **array, int i, int j);
char updateCellState(char cell, int value, int *flag);

void scan(int *command);
void changeRate(char button, float *rate);

int allocMemory(char ***array);
void freeMemory(char **array);

int main() {
    startState();

    return 0;
}

void startState() {
    int command = -1;
    scan(&command);
    processState(command);
}

void scan(int *command) {
    int check = 0;

    while (!check) {
        check = scanf("%d", command);
        if (!check) {
            printf("n/a\n");
            getchar();
        } else if (*command < 1 || *command >= 6) {
            printf("n/a\n");
            check = 0;
        }
    }
}

void processState(int mode) {
    char **array;
    char **buffer;
    float rate = 1.0f;
    char button = '\0';

    allocMemory(&array);
    allocMemory(&buffer);

    initState(mode);
    fillState(array);

    stdin = freopen("/dev/tty", "r", stdin);
    initscr();
    noecho();
    WINDOW *win = newwin(HEIGHT, LENGTH, 0, 0);
    wrefresh(win);
    outputState(array, win);
    wrefresh(win);

    while (updateState(&array, &buffer) && !(button == 'q' || button == 'Q')) {
        outputState(array, win);
        mvwprintw(win, HEIGHT - 1, 5, "rate: x%.1f", 2.0 - rate);
        halfdelay(1);
        button = wgetch(win);
        changeRate(button, &rate);
        usleep(STARTRATE * rate);
        wrefresh(win);
    }

    freeMemory(array);
    freeMemory(buffer);

    endwin();
}

void initState(int mode) {
    switch (mode) {
        case 1:
            stdin = freopen("./maps/map1.txt", "r", stdin);
            break;
        case 2:
            stdin = freopen("./maps/map2.txt", "r", stdin);
            break;
        case 3:
            stdin = freopen("./maps/map3.txt", "r", stdin);
            break;
        case 4:
            stdin = freopen("./maps/map4.txt", "r", stdin);
            break;
        case 5:
            stdin = freopen("./maps/map5.txt", "r", stdin);
            break;
    }
}

int allocMemory(char ***array) {
    int check = 1;
    (*array) = malloc(HEIGHT * sizeof(char *));

    if (*array != NULL) {
        for (int i = 0; i < HEIGHT; i++) {
            (*array)[i] = malloc(LENGTH * sizeof(char));
            if ((*array)[i] == NULL) {
                check = 0;
                for (int j = 0; j < i; j++) free((*array)[i]);
                free(array);
                break;
            }
        }
    } else {
        check = 0;
    }

    return check;
}

void freeMemory(char **array) {
    for (int i = 0; i < HEIGHT; i++) free(array[i]);
    free(array);
}

void fillState(char **array) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < LENGTH; j++) {
            char c;
            scanf("%c ", &c);
            if (c == '-') c = '0';
            if (c == 'o') c = '1';
            array[i][j] = c;
        }
    }
}

int updateState(char ***array, char ***buffer) {
    int livCells = 0;
    int flag = 0;
    int value;
    int check = 1;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < LENGTH; j++) {
            value = countCellIsAlive(*array, i, j);
            livCells += value;
            (*buffer)[i][j] = updateCellState((*array)[i][j], value, &flag);
        }
    }
    char **temp = *array;
    *array = *buffer;
    *buffer = temp;
    if (livCells == 0 || flag == 0) check = 0;

    return check;
}

int countCellIsAlive(char **array, int i, int j) {
    int value = 0;
    for (int istep = -1; istep <= 1; istep++) {
        for (int jstep = -1; jstep <= 1; jstep++) {
            if ((array[(HEIGHT + i + istep) % HEIGHT][(LENGTH + j + jstep) % LENGTH] == '1') &&
                !(istep == 0 && jstep == 0))
                value++;
        }
    }
    return value;
}

char updateCellState(char cell, int value, int *flag) {
    char newCage;

    if (cell == '1') {
        if (value != 2 && value != 3) {
            newCage = '0';
            *flag = 1;
        } else {
            newCage = '1';
        }
    } else {
        if (value == 3) {
            newCage = '1';
            *flag = 1;
        } else {
            newCage = '0';
        }
    }

    return newCage;
}

void outputState(char **array, WINDOW *win) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < LENGTH; j++) {
            if (array[i][j] == '1') {
                mvwprintw(win, i, j, "*");
                printw("*");
            } else {
                mvwprintw(win, i, j, " ");
            }
        }
    }
}

void changeRate(char button, float *rate) {
    if (*rate > MINRATE && button == 'i') {
        *rate -= STEP;
    }
    if (*rate < MAXRATE && button == 'o') {
        *rate += STEP;
    }
}