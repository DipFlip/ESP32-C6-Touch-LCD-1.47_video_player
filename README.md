# ESP32-C6 Touch LCD 1.47" Phone Interface

Touch-enabled phone interface mockup for the ESP32-C6 Touch LCD 1.47" display.

## Features

- **Phone-Style Home Screen**: 2x2 grid with 4 app icons
- **Multiple App Types**: 
  - Static Image Apps (Facebook)
  - Video Player Apps (Sci-Fi, Animals, Action)
  - Extensible for future apps (games, etc.)
- **Pre-designed Home Screen**: Uses composite image with background and icons
- **Smart Video Progression**: Each video app has themed playlists
- **Touch Navigation**: Tap apps to launch, touch top to return home
- **Professional UI**: Clean interface with app names and return indicators

## Hardware Requirements

- ESP32-C6 Touch LCD 1.47" display
- MicroSD card with videos and images
- **Required SD card structure**:
  - `/mjpeg/` - MJPEG video files
  - `/images/app-select-screen.jpg` - Complete home screen with background and app icons
  - `/images/image_1.jpg` - Fallback wallpaper (if app-select-screen.jpg not found)
  - `/facebook-app.jpg` - Facebook app content image
  - `/logos/` - Individual app icons (fallback only)

## Touch Controls

### On Home Screen:
- **Tap Top-Left**: Launch Facebook app (static image)
- **Tap Top-Right**: Launch Sci-Fi video app (alien, space themes)
- **Tap Bottom-Left**: Launch Animals video app (catmeow, nature themes)
- **Tap Bottom-Right**: Launch Action video app (gundam, action themes)

### In Apps:
- **Facebook App**: Touch anywhere to return to home screen
- **Video Apps**: 
  - Touch top area to return to home screen
  - Touch anywhere else to skip to next themed video
  - Videos automatically progress through app's playlist

## App System

### Static Image Apps
- Display a single image from SD card
- Simple touch-to-return interface
- Perfect for mockup content like social media feeds

### Video Player Apps  
- Each app has its own themed video playlist
- Smart progression through related videos
- Touch controls for navigation and return

### Future App Types (Extensible)
- **Game Apps**: Add game logic with touch controls
- **Settings Apps**: Configuration screens
- **Utility Apps**: Calculator, clock, etc.
- Easy to add new apps by modifying the `apps[]` array

## Code Structure

### App Definition
Each app is defined in the `apps[]` array with:
```cpp
{"App Name", "icon_path", APP_STATE, APP_TYPE, "static_image", "start_video", {...keywords}, keyword_count}
```

### Adding New Apps
1. Add new AppState enum value
2. Add app definition to apps[] array  
3. Add app icon to /logos/ folder
4. Handle new state in main loop
5. Implement app-specific logic

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