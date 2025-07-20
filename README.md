# ESP32-C6 Touch LCD 1.47" Video Player

Touch-enabled video player for the ESP32-C6 Touch LCD 1.47" display.

## Features

- Touch control support
- MJPEG video playback from SD card
- Touch to skip to next video
- Automatic video loop
- Performance statistics output

## Hardware Requirements

- ESP32-C6 Touch LCD 1.47" display
- MicroSD card with MJPEG videos
- MJPEG videos should be placed in `/mjpeg/` folder on the SD card

## Touch Controls

- **Tap anywhere**: Skip to next video
- Videos automatically loop when reaching the end of the playlist

## Pin Configuration (Touch Version)

- **Display SPI**: SCK=7, MOSI=6, MISO=5, CS=14, DC=15, RST=21
- **Touch I2C**: SDA=18, SCL=19, RST=20, INT=21
- **SD Card**: CS=4
- **Backlight**: GPIO 22

## Dependencies

- Arduino_GFX_Library
- esp_lcd_touch_axs5106l (included in libraries folder)
- JPEGDEC (for MJPEG decoding)
- SD library (included with ESP32 core)

## Installation

1. Use Arduino IDE with ESP32 board support
2. Select "ESP32C6 Dev Module" as the board
3. Install "GFX Library for Arduino" from Library Manager
4. Copy MJPEG files to SD card in `/mjpeg/` folder
5. Upload the sketch

## Video Format

- Format: MJPEG
- Recommended resolution: 172x320 or smaller
- File extension: .mjpeg
- Place files in `/mjpeg/` folder on SD card

## Differences from Non-Touch Version

1. **Updated pin configuration** for touch LCD version
2. **Touch controller initialization** using AXS5106L driver
3. **Touch input handling** for video control
4. **Enhanced user interface** with touch control instructions

## Performance

The video player outputs performance statistics including:
- Total frames played
- Average FPS
- Time breakdown (read, decode, display)

## License

Based on the original ESP32-C6-LCD-1.47_video_player, adapted for touch functionality.