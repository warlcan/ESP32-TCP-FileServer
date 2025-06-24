#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "ssd1306.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int cmd;
    char data[16];
    int y;
} DisplayCommand;

typedef enum {
    STATUS_DOWNLOAD = 0,
    STATUS_UPLOAD = 1
} StatusLoading;

enum {
    DISPLAY_YES = 0,
    DISPLAY_NO = 1,
    DFISPLAY_ERROR = 2
};

void display_init(void);
void send_display_command(int cmd, int y, char *data);
void show_intro(bool);
void show_mnt_status(int mnt_status);
void show_cnt_status(int cnt_status);
void show_file_size(size_t file_size);
void show_progress_status(uint8_t progress);
void show_file_name(char *file_name);

#endif