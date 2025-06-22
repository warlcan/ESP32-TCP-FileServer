#ifndef UPLOAD_H
#define UPLOAD_H

#include "config.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "string.h"
#include <sys/socket.h>

void start_upload(int main_sock, char *file_name);

#endif