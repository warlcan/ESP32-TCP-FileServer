#include "include/sd_card.h"

#define PIN_SPI_MISO  19
#define PIN_SPI_MOSI  23
#define PIN_SPI_CLK   18
#define PIN_SPI_CS    5


esp_err_t sd_card_controller(const bool mount, const char *mount_point) {
    static bool is_init = false;
    static sdmmc_card_t *card = NULL;
    static sdmmc_host_t host;
    static sdspi_device_config_t slot_config;
    static esp_vfs_fat_sdmmc_mount_config_t mount_config;

    if (!is_init) {
        // SPI conf
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = PIN_SPI_MOSI,
            .miso_io_num = PIN_SPI_MISO,
            .sclk_io_num = PIN_SPI_CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4000,
        };
        
        esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
        if (ret != ESP_OK) {
            return ret;
        }

        // Host conf (temp) 
        sdmmc_host_t temp_host = SDSPI_HOST_DEFAULT();
        temp_host.max_freq_khz = 400;
        host = temp_host;

        // Slot conf
        sdspi_device_config_t temp_slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        temp_slot_config.gpio_cs = PIN_SPI_CS;
        temp_slot_config.host_id = SPI2_HOST;
        slot_config = temp_slot_config;

        // Mount conf
        mount_config = (esp_vfs_fat_sdmmc_mount_config_t) {
            .format_if_mount_failed = false,
            .max_files = 1,
            .allocation_unit_size = 16 * 1024
        };

        is_init = true;
    }

    if (mount) {
        esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
        return ret;
    } else {
        esp_err_t ret = esp_vfs_fat_sdcard_unmount(mount_point, card);
        spi_bus_free(SPI2_HOST);
        is_init = false;
        return ret;
    }
}