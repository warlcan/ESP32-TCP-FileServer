#include "esp_err.h"
#include <stdbool.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"

#ifndef SD_CARD_H
#define SD_CARD_H

esp_err_t sd_card_controller(const bool mount, const char *mount_point);

#endif