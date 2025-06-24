#include "include/sd_card.h"

static sdmmc_card_t *card = NULL;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();
static sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
static esp_vfs_fat_sdmmc_mount_config_t mount_config;
static char mount_point[32] = {0};

#define TAG "SD"

void sd_card_init(const char *_mount_point)
{
    strlcpy(mount_point, _mount_point, sizeof(mount_point));
    // SPI
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_SPI_MOSI,
        .miso_io_num = PIN_SPI_MISO,
        .sclk_io_num = PIN_SPI_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error: spi bus: %d", ret);
        return;
    }
    host.max_freq_khz = SD_CARD_MAX_FEQ_KHZ;

    slot_config.gpio_cs = PIN_SPI_CS;
    slot_config.host_id = SPI2_HOST;

    mount_config = (esp_vfs_fat_sdmmc_mount_config_t){
        .format_if_mount_failed = false,
        .max_files = 2,
        .allocation_unit_size = 16 * 1024};
}

esp_err_t mount_sd()
{
    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret == ESP_OK) ESP_LOGI(TAG, "Mount OK");
    else ESP_LOGE(TAG, "Error: 0x%X", ret);
    return ret;
}

esp_err_t umount_sd()
{
    esp_err_t ret = esp_vfs_fat_sdcard_unmount(mount_point, card);
    if (ret == ESP_OK) ESP_LOGI(TAG, "Umount OK");
    else ESP_LOGE(TAG, "Error: 0x%X", ret);
    //spi_bus_free(SPI2_HOST);
    return ret;
}