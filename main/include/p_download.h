#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "config.h"

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "string.h"
#include <sys/socket.h>

void start_download(int main_sock, char* file_name, uint32_t file_size);

#endif