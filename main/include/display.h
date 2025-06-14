#ifndef DISPLAY_H
#define DISPLAY_H

#include "ssd1306.h"
#include <stdint.h>
#include <stdbool.h>

#define DISPLAY_QUEUE_SIZE 10

typedef struct {
    int cmd;
    char data[16];
    int y;
} DisplayCommand;

void display_init(void);
void send_display_command(int cmd, int y, char *data);
void send_display_clear();
void draw_intro(void);

#endif