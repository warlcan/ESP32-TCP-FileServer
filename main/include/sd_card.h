#include "esp_err.h"
#include <stdbool.h>
#include <string.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_log.h"

#ifndef SD_CARD_H
#define SD_CARD_H

void sd_card_init(const char *_mount_point);
esp_err_t mount_sd();
esp_err_t umount_sd();

#endif