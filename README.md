# ESP32-C6 Touch LCD 1.47" Video Player

Touch-enabled video player for the ESP32-C6 Touch LCD 1.47" display.

## Features

- **Home Screen**: Displays image from SD card with touch buttons
- **Two Video Modes**: 
  - Alien Eye mode (sci-fi themed videos)
  - Cheetah mode (animal/nature themed videos)
- **Smart Video Progression**: Automatically selects related videos based on chosen mode
- **Touch Controls**: Intuitive touch interface for navigation
- **Return to Home**: Touch top area during video to return to home screen
- **MJPEG video playback** from SD card
- **Performance statistics** output

## Hardware Requirements

- ESP32-C6 Touch LCD 1.47" display
- MicroSD card with MJPEG videos and home screen image
- **Required SD card structure**:
  - `/mjpeg/` folder with MJPEG videos
  - `/images/image_1.jpg` for home screen background

## Touch Controls

### On Home Screen:
- **Tap TOP area (green)**: Start Alien Eye mode (sci-fi videos)
- **Tap BOTTOM area (yellow)**: Start Cheetah mode (animal/nature videos)

### During Video Playback:
- **Tap TOP area**: Return to home screen  
- **Tap anywhere else**: Skip to next related video
- Videos automatically progress through themed playlists

## Pin Configuration (Touch Version)

- **Display SPI**: SCK=1, MOSI=2, MISO=3, CS=14, DC=15, RST=22
- **Touch I2C**: SDA=18, SCL=19, RST=20, INT=21
- **SD Card**: CS=4
- **Backlight**: GPIO 23

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