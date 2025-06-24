# File Server for ESP32

## Table of Contents
- [Project Description](#project-description)
- [Features](#features)
- [Hardware Configuration](#hardware-configuration)
- [Network Protocol](#network-protocol)
- [Installation and Building](#installation-and-building)
- [Configuration](#configuration)
- [Launch and Usage](#launch-and-usage)

## Project Description
This project implements a TCP file server for ESP32 with file upload/download capabilities over Wi-Fi. Files are stored on an SD card using SPI interface, and system messages are displayed on a 128×64 OLED screen via I2C bus.

## Features
- File upload/download over TCP
- 8.3 filename format restriction (8 characters + "." + 3-character extension)
- Data storage on SPI-connected SD card
- Status display on 128×64 I2C OLED screen
- Configurable SSID, password, and peripheral pins in `config.h`

## Hardware Configuration
| Component      | Interface | Default Pins                     |
|----------------|-----------|----------------------------------|
| SD Card        | SPI       | Configured in `main/include/config.h` |
| OLED 128×64    | I2C       | Configured in `main/include/config.h` |
| ESP32 Wi-Fi    | UART/TCP  | SSID/PASSWORD in `main/include/config.h` |

## Network Protocol
The server expects special control packets to initiate data transfers.

### 1. File Upload
1. Client sends header:
   ```
   | 1B FLAG | 4B FILESIZE | 8B + 1B + 3B FileName |
   ```
   - FLAG: Control code for transfer start
   - FILESIZE: File size in bytes (little-endian)
   - FileName: Up to 12 characters (FAT32 limitation)
2. After receiving header, server expects data packet:
   ```
   | 1B FLAG (0xDD) | up to 31B DATA |
   ```
3. Server writes content to SD card with specified filename

### 2. File Download
1. Client sends request:
   ```
   | 1B FLAG | 8B + 1B + 3B FileName |
   ```
2. Server opens file and sends data through established TCP socket
3. If file not found, server returns error message and waits for next control packet

## Installation and Building
1. Clone repository:
   ```bash
   git clone https://github.com/warlcan/ESP32-TCP-FileServer.git
   ```
2. Navigate to project directory:
   ```bash
   cd ESP32-TCP-FileServer
   ```
3. Configure parameters in `main/include/config.h`:
   - SD card and OLED pins
   - Wi-Fi SSID and PASSWORD
4. Build and flash firmware:
   ```bash
   idf.py build
   idf.py -p <PORT> flash monitor
   ```

## Configuration
All customizable parameters are located in `main/include/config.h`:

**Wi-Fi Settings**
```c
#define WIFI_SSID      "your-ssid"
#define WIFI_PASSWORD  "your-password"
```

**SD Card (SPI)**
```c
#define PIN_SPI_MISO 19
#define PIN_SPI_MOSI 23
#define PIN_SPI_CLK 18
#define PIN_SPI_CS 5
```

**OLED Display (I2C)**
```c
#define PIN_SDA_GPIO 21
#define PIN_SCL_GPIO 22
#define PIN_RESET_GPIO -1
```

## Launch and Usage
1. After flashing, the module will connect to Wi-Fi and listen on TCP port (default: 12035)
2. ESP32's IP address will be displayed on OLED screen (line 2)
3. Use TCP client (e.g., `netcat` or custom script) to send control packets and data

Test scripts in `scripts/` directory:
```bash
# Upload file to server
python3 UPLOAD_test_script.py <IP> <PORT> archive.zip

# Download file from server
python3 DOWNLOAD_test_script.py <IP> <PORT> archive.zip
```
> Note: IP address is shown in `monitor` console and on OLED display