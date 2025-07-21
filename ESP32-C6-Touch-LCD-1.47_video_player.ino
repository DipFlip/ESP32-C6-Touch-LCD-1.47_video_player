// Touch-enabled video player for ESP32-C6 Touch LCD 1.47"
// Based on ESP32-C6-LCD-1.47_video_player adapted for touch version
// Features: Home screen with image and touch buttons for video selection
// Use board "ESP32C6 Dev Module"

#include <Arduino_GFX_Library.h>
#include "esp_lcd_touch_axs5106l.h"
#include "MjpegClass.h"
#include "SD.h"
#include "Arduino.h"
#include "JpegFunc.h"

#define GFX_BRIGHTNESS 255

// Touch pins
#define Touch_I2C_SDA 18
#define Touch_I2C_SCL 19
#define Touch_RST     20
#define Touch_INT     21

// Display and hardware pins (updated for touch version)
#define GFX_BL 23
#define SD_CS 4

// Display setup for touch version (using correct pins from examples)
Arduino_DataBus *bus = new Arduino_HWSPI(15 /* DC */, 14 /* CS */, 1 /* SCK */, 2 /* MOSI */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 22 /* RST */, 0 /* rotation */, false /* IPS */, 172 /* width */, 320 /* height */, 34 /* col offset 1 */, 0 /* row offset 1 */, 34 /* col offset 2 */, 0 /* row offset 2 */);

const char *MJPEG_FOLDER = "/mjpeg";

// Storage for files to read on the SD card
#define MAX_FILES 20
String mjpegFileList[MAX_FILES];
uint32_t mjpegFileSizes[MAX_FILES] = {0};
int mjpegCount = 0;
static int currentMjpegIndex = 0;
static File mjpegFile;

// Global variables for mjpeg
MjpegClass mjpeg;
int total_frames;
unsigned long total_read_video;
unsigned long total_decode_video;
unsigned long total_show_video;
unsigned long start_ms, curr_ms;
long output_buf_size, estimateBufferSize;
uint8_t *mjpeg_buf;
uint16_t *output_buf;

// Touch control variables
volatile bool skipRequested = false;
volatile bool pauseRequested = false;
volatile uint32_t lastTouch = 0;
bool isPaused = false;

// Touch data
touch_data_t touch_data;

// Phone Interface App System
enum AppState {
  HOME_SCREEN,
  FACEBOOK_APP,
  SCIFI_VIDEO_APP,
  ANIMAL_VIDEO_APP,
  ACTION_VIDEO_APP
};

enum AppType {
  STATIC_IMAGE_APP,
  VIDEO_PLAYER_APP
};

struct App {
  const char* name;
  const char* iconPath;
  AppState appState;
  AppType appType;
  const char* staticImagePath;  // For static image apps
  const char* startVideoKeyword; // For video apps
  const char* videoKeywords[8]; // For video app playlists
  int videoKeywordCount;
};

// App definitions (2x2 grid layout)
App apps[4] = {
  // Top-left: Facebook
  {"Facebook", "/logos/facebook.jpg", FACEBOOK_APP, STATIC_IMAGE_APP, "/facebook-app.jpg", nullptr, {}, 0},
  
  // Top-right: Sci-Fi Videos  
  {"Sci-Fi", "/logos/scifi.jpg", SCIFI_VIDEO_APP, VIDEO_PLAYER_APP, nullptr, "alien_eye", 
   {"alien", "death_star", "starfighter", "nova", "universe", "human_scan", "human_xray", "dna_helix"}, 8},
  
  // Bottom-left: Animal Videos
  {"Animals", "/logos/animal.jpg", ANIMAL_VIDEO_APP, VIDEO_PLAYER_APP, nullptr, "catmeow",
   {"catmeow", "cheetah", "turtle", "human_eye", "falls", "river", "earth_spin"}, 7},
   
  // Bottom-right: Action Videos (or future game)
  {"Action", "/logos/action.jpg", ACTION_VIDEO_APP, VIDEO_PLAYER_APP, nullptr, "gundam",
   {"gundam", "winston", "circuit", "control_room", "rebel_base"}, 5}
};

AppState currentState = HOME_SCREEN;
int currentAppIndex = -1;

// JPEG drawing callback for home screen image

// Device initialization for touch version
void DEV_DEVICE_INIT() {
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
}

void lcd_reg_init(void) {
  static const uint8_t init_operations[] = {
    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x11,  // 2: Out of sleep mode, no args, w/delay
    END_WRITE,
    DELAY, 120,

    BEGIN_WRITE,
    WRITE_C8_D16, 0xDF, 0x98, 0x53,
    WRITE_C8_D8, 0xB2, 0x23, 

    WRITE_COMMAND_8, 0xB7,
    WRITE_BYTES, 4,
    0x00, 0x47, 0x00, 0x6F,

    WRITE_COMMAND_8, 0xBB,
    WRITE_BYTES, 6,
    0x1C, 0x1A, 0x55, 0x73, 0x63, 0xF0,

    WRITE_C8_D16, 0xC0, 0x44, 0xA4,
    WRITE_C8_D8, 0xC1, 0x16, 

    WRITE_COMMAND_8, 0xC3,
    WRITE_BYTES, 8,
    0x7D, 0x07, 0x14, 0x06, 0xCF, 0x71, 0x72, 0x77,

    WRITE_COMMAND_8, 0xC4,
    WRITE_BYTES, 12,
    0x00, 0x00, 0xA0, 0x79, 0x0B, 0x0A, 0x16, 0x79, 0x0B, 0x0A, 0x16, 0x82,

    WRITE_COMMAND_8, 0xC8,
    WRITE_BYTES, 32,
    0x3F, 0x32, 0x29, 0x29, 0x27, 0x2B, 0x27, 0x28, 0x28, 0x26, 0x25, 0x17, 0x12, 0x0D, 0x04, 0x00, 0x3F, 0x32, 0x29, 0x29, 0x27, 0x2B, 0x27, 0x28, 0x28, 0x26, 0x25, 0x17, 0x12, 0x0D, 0x04, 0x00,

    WRITE_COMMAND_8, 0xD0,
    WRITE_BYTES, 5,
    0x04, 0x06, 0x6B, 0x0F, 0x00,

    WRITE_C8_D16, 0xD7, 0x00, 0x30,
    WRITE_C8_D8, 0xE6, 0x14, 
    WRITE_C8_D8, 0xDE, 0x01, 

    WRITE_COMMAND_8, 0xB7,
    WRITE_BYTES, 5,
    0x03, 0x13, 0xEF, 0x35, 0x35,

    WRITE_COMMAND_8, 0xC1,
    WRITE_BYTES, 3,
    0x14, 0x15, 0xC0,

    WRITE_C8_D16, 0xC2, 0x06, 0x3A,
    WRITE_C8_D16, 0xC4, 0x72, 0x12,
    WRITE_C8_D8, 0xBE, 0x00, 
    WRITE_C8_D8, 0xDE, 0x02, 

    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3,
    0x00, 0x02, 0x00,

    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3,
    0x01, 0x02, 0x00,

    WRITE_C8_D8, 0xDE, 0x00, 
    WRITE_C8_D8, 0x35, 0x00, 
    WRITE_C8_D8, 0x3A, 0x05, 

    WRITE_COMMAND_8, 0x2A,
    WRITE_BYTES, 4,
    0x00, 0x22, 0x00, 0xCD,

    WRITE_COMMAND_8, 0x2B,
    WRITE_BYTES, 4,
    0x00, 0x00, 0x01, 0x3F,

    WRITE_C8_D8, 0xDE, 0x02, 

    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3,
    0x00, 0x02, 0x00,
    
    WRITE_C8_D8, 0xDE, 0x00, 
    WRITE_C8_D8, 0x36, 0x00,
    WRITE_COMMAND_8, 0x21,
    END_WRITE,
    
    DELAY, 10,

    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x29,  // 5: Main screen turn on, no args, w/delay
    END_WRITE
  };
  bus->batchOperation(init_operations, sizeof(init_operations));
}

void setup()
{
    Serial.begin(115200);
    DEV_DEVICE_INIT();
    delay(2000);

    // Initialize SPI with touch version pins (from SD card example)
    SPI.begin(1 /* SCK */, 3 /* MISO */, 2 /* MOSI */);

    // Display initialization
    if (!gfx->begin())
    {
        Serial.println("Display initialization failed!");
        while (true) {}
    }
    lcd_reg_init();
    gfx->setRotation(0);
    gfx->fillScreen(RGB565_BLACK);
    setDisplayBrightness();

    // Initialize touch
    Wire.begin(Touch_I2C_SDA, Touch_I2C_SCL);
    bsp_touch_init(&Wire, Touch_RST, Touch_INT, gfx->getRotation(), gfx->width(), gfx->height());
    Serial.println("Touch initialized");

    // SD card initialization (using correct CS pin and SPI setup)
    if (!SD.begin(SD_CS))
    {
        Serial.println("ERROR: File system mount failed!");
        while (true) {}
    }

    // Buffer allocation for mjpeg playing
    output_buf_size = gfx->width() * 4 * 2;
    output_buf = (uint16_t *)heap_caps_aligned_alloc(16, output_buf_size * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!output_buf)
    {
        Serial.println("output_buf aligned_alloc failed!");
        while (true) {}
    }
    estimateBufferSize = gfx->width() * gfx->height() * 2 / 5;
    mjpeg_buf = (uint8_t *)heap_caps_malloc(estimateBufferSize, MALLOC_CAP_8BIT);

    loadMjpegFilesList();

    // Show the home screen
    Serial.println("Showing home screen");
    showHomeScreen();
}

void setDisplayBrightness()
{
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
}

// JPEG draw callback for home screen image
int jpegDrawCallbackHomeScreen(JPEGDRAW *pDraw)
{
    gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
    return 1;
}

// Phone-style home screen using pre-designed composite image
void showHomeScreen()
{
    gfx->fillScreen(RGB565_BLACK);
    
    // Use the pre-designed app select screen with background and icons
    File appSelectFile = SD.open("/images/app-select-screen.jpg", "r");
    if (appSelectFile) {
        appSelectFile.close();
        Serial.println("Loading app select screen...");
        // Display the complete app select screen
        jpegDraw("/images/app-select-screen.jpg", jpegDrawCallbackHomeScreen, true,
                0, 0, gfx->width(), gfx->height());
    } else {
        Serial.println("App select screen not found, using fallback");
        // Fallback to generated interface
        drawAppIconsFallback();
    }
}

// Fallback interface if app-select-screen.jpg is not found
void drawAppIconsFallback()
{
    gfx->fillScreen(RGB565_BLACK);
    
    // Try to show background image first  
    File wallpaperFile = SD.open("/images/image_1.jpg", "r");
    if (wallpaperFile) {
        wallpaperFile.close();
        Serial.println("Loading background wallpaper...");
        jpegDraw("/images/image_1.jpg", jpegDrawCallbackHomeScreen, true,
                0, 0, gfx->width(), gfx->height());
        // Add semi-transparent overlay for better icon visibility
        for (int y = 0; y < gfx->height(); y += 4) {
            for (int x = 0; x < gfx->width(); x += 4) {
                gfx->drawPixel(x, y, RGB565_BLACK);
            }
        }
    }
    
    // 2x2 Grid layout for 4 apps
    // Screen: 172x320, Grid: 86x160 per quadrant
    
    int iconSize = 60;
    int gridWidth = gfx->width() / 2;
    int gridHeight = (gfx->height() - 40) / 2; // Leave space at top and bottom
    
    for (int i = 0; i < 4; i++) {
        int gridX = i % 2;  // 0 or 1 (left/right)
        int gridY = i / 2;  // 0 or 1 (top/bottom)
        
        int iconX = gridX * gridWidth + (gridWidth - iconSize) / 2;
        int iconY = 40 + gridY * gridHeight + (gridHeight - iconSize - 20) / 2; // 20 for text
        
        // Try to load individual app icon
        File iconFile = SD.open(apps[i].iconPath, "r");
        if (iconFile) {
            iconFile.close();
            Serial.printf("Loading icon: %s\n", apps[i].iconPath);
            jpegDraw(apps[i].iconPath, jpegDrawCallbackHomeScreen, true,
                    iconX, iconY, iconSize, iconSize);
        } else {
            // Fallback: Draw colored rectangle with app initial
            uint16_t appColors[] = {RGB565_BLUE, RGB565_RED, RGB565_GREEN, RGB565_ORANGE};
            gfx->fillRoundRect(iconX, iconY, iconSize, iconSize, 8, appColors[i]);
            gfx->setTextColor(RGB565_WHITE);
            gfx->setTextSize(3);
            gfx->setCursor(iconX + iconSize/2 - 9, iconY + iconSize/2 - 12);
            gfx->print(apps[i].name[0]); // First letter of app name
        }
        
        // Draw app name below icon
        gfx->setTextColor(RGB565_WHITE);
        gfx->setTextSize(1);
        int textWidth = strlen(apps[i].name) * 6; // Approximate text width
        int textX = gridX * gridWidth + (gridWidth - textWidth) / 2;
        int textY = iconY + iconSize + 5;
        gfx->setCursor(textX, textY);
        gfx->println(apps[i].name);
    }
    
    // Title at top
    gfx->setTextColor(RGB565_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(10, 10);
    gfx->println("ESP32 Phone Interface");
}

void showStaticImageApp(int appIndex)
{
    gfx->fillScreen(RGB565_BLACK);
    
    // Show return instruction at top
    gfx->setTextColor(RGB565_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(5, 5);
    gfx->println("< Touch top to return");
    
    // Display the static image
    File imageFile = SD.open(apps[appIndex].staticImagePath, "r");
    if (imageFile) {
        imageFile.close();
        Serial.printf("Loading static image: %s\n", apps[appIndex].staticImagePath);
        jpegDraw(apps[appIndex].staticImagePath, jpegDrawCallbackHomeScreen, true,
                0, 25, gfx->width(), gfx->height() - 25);
    } else {
        // Fallback display
        gfx->fillRect(10, 30, gfx->width() - 20, gfx->height() - 60, RGB565_BLUE);
        gfx->setTextColor(RGB565_WHITE);
        gfx->setTextSize(2);
        int textX = (gfx->width() - strlen(apps[appIndex].name) * 12) / 2;
        gfx->setCursor(textX, gfx->height()/2 - 20);
        gfx->println(apps[appIndex].name);
        
        gfx->setTextSize(1);
        gfx->setCursor(20, gfx->height()/2 + 10);
        gfx->println("Image not found:");
        gfx->setCursor(20, gfx->height()/2 + 25);
        gfx->println(apps[appIndex].staticImagePath);
        
        gfx->setCursor(20, gfx->height() - 40);
        gfx->println("Touch anywhere to return");
    }
}

void displayTouchControls()
{
    gfx->fillScreen(RGB565_BLACK);
    gfx->setTextColor(RGB565_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(10, 10);
    gfx->println("ESP32-C6 Touch Video Player");
    gfx->setCursor(10, 30);
    gfx->println("Touch Controls:");
    gfx->setCursor(10, 50);
    gfx->println("- Tap anywhere: Next video");
    gfx->setCursor(10, 70);
    gfx->println("- Hold: Pause/Resume");
    gfx->setCursor(10, 100);
    gfx->println("Starting playback...");
}

void checkTouch()
{
    bsp_touch_read();
    bool touchPressed = bsp_touch_get_coordinates(&touch_data);
    
    if (touchPressed)
    {
        uint32_t now = millis();
        if (now - lastTouch > 300) // 300ms debounce
        {
            int touchX = touch_data.coords[0].x;
            int touchY = touch_data.coords[0].y;
            Serial.printf("Touch detected at: x=%d, y=%d\n", touchX, touchY);
            
            if (currentState == HOME_SCREEN) {
                // Detect which app was touched (2x2 grid)
                int gridX = (touchX < gfx->width() / 2) ? 0 : 1;
                int gridY = (touchY < gfx->height() / 2) ? 0 : 1;
                int appIndex = gridY * 2 + gridX;
                
                if (appIndex >= 0 && appIndex < 4) {
                    launchApp(appIndex);
                }
            } else {
                // In an app - check for return to home or app-specific actions
                if (touchY <= 30) {
                    // Top area - return to home screen
                    Serial.println("Returning to home screen");
                    returnToHome();
                } else {
                    // App-specific touch handling
                    handleAppTouch(touchX, touchY);
                }
            }
            lastTouch = now;
        }
    }
}

void launchApp(int appIndex)
{
    Serial.printf("Launching app: %s\n", apps[appIndex].name);
    currentAppIndex = appIndex;
    currentState = apps[appIndex].appState;
    
    if (apps[appIndex].appType == STATIC_IMAGE_APP) {
        // Launch static image app (like Facebook)
        showStaticImageApp(appIndex);
    } else if (apps[appIndex].appType == VIDEO_PLAYER_APP) {
        // Launch video player app
        currentMjpegIndex = findVideoIndex(apps[appIndex].startVideoKeyword);
        Serial.printf("Starting video app with: %s\n", apps[appIndex].startVideoKeyword);
    }
}

void returnToHome()
{
    currentState = HOME_SCREEN;
    currentAppIndex = -1;
    skipRequested = true; // Stop any video playback
    showHomeScreen();
}

void handleAppTouch(int touchX, int touchY)
{
    if (currentAppIndex >= 0) {
        if (apps[currentAppIndex].appType == STATIC_IMAGE_APP) {
            // For static image apps, any touch returns to home
            returnToHome();
        } else if (apps[currentAppIndex].appType == VIDEO_PLAYER_APP) {
            // For video apps, skip to next video
            skipRequested = true;
        }
    }
}

// Find index of specific video in the list
int findVideoIndex(const char* filename)
{
    for (int i = 0; i < mjpegCount; i++) {
        if (mjpegFileList[i].indexOf(filename) >= 0) {
            return i;
        }
    }
    return 0; // Default to first video if not found
}

// Get next video index based on current app
int getNextVideoIndex(int currentIndex)
{
    Serial.printf("Getting next video from index %d (current: %s)\n", currentIndex, mjpegFileList[currentIndex].c_str());
    
    // Find current app
    if (currentAppIndex < 0 || currentAppIndex >= 4) {
        Serial.println("Invalid app index, using default progression");
        return (currentIndex + 1) % mjpegCount;
    }
    
    App* currentApp = &apps[currentAppIndex];
    if (currentApp->appType != VIDEO_PLAYER_APP) {
        Serial.println("Current app is not a video player");
        return currentIndex;
    }
    
    // Search for next video matching this app's keywords
    // First, search from current position + 1 to end
    for (int i = currentIndex + 1; i < mjpegCount; i++) {
        for (int j = 0; j < currentApp->videoKeywordCount; j++) {
            if (mjpegFileList[i].indexOf(currentApp->videoKeywords[j]) >= 0) {
                Serial.printf("Found next themed video: %s at index %d\n", mjpegFileList[i].c_str(), i);
                return i;
            }
        }
    }
    
    // If not found, search from beginning to current position (wrap around)
    for (int i = 0; i < currentIndex; i++) {
        for (int j = 0; j < currentApp->videoKeywordCount; j++) {
            if (mjpegFileList[i].indexOf(currentApp->videoKeywords[j]) >= 0) {
                Serial.printf("Found wrap-around themed video: %s at index %d\n", mjpegFileList[i].c_str(), i);
                return i;
            }
        }
    }
    
    // Fallback to app's start video
    Serial.printf("No other themed videos found, returning to start video: %s\n", currentApp->startVideoKeyword);
    return findVideoIndex(currentApp->startVideoKeyword);
}

void loop()
{
    if (currentState == HOME_SCREEN) {
        // On home screen - just check for touch input
        checkTouch();
        delay(50);
    } else if (currentState == FACEBOOK_APP) {
        // In static image app - just check for touch input
        checkTouch();
        delay(50);
    } else if (currentState == SCIFI_VIDEO_APP || currentState == ANIMAL_VIDEO_APP || currentState == ACTION_VIDEO_APP) {
        // In video player app - play videos
        playSelectedMjpeg(currentMjpegIndex);
        
        if (currentState != HOME_SCREEN) { // Don't advance if we returned to home
            currentMjpegIndex = getNextVideoIndex(currentMjpegIndex);
        }
    } else {
        // Unknown state - return to home
        Serial.println("Unknown app state, returning to home");
        returnToHome();
    }
}

void playSelectedMjpeg(int mjpegIndex)
{
    // Build the full path for the selected mjpeg
    String fullPath = String(MJPEG_FOLDER) + "/" + mjpegFileList[mjpegIndex];
    char mjpegFilename[128];
    fullPath.toCharArray(mjpegFilename, sizeof(mjpegFilename));

    Serial.printf("Playing %s\n", mjpegFilename);
    mjpegPlayFromSDCard(mjpegFilename);
}

// Callback function to draw a JPEG
int jpegDrawCallback(JPEGDRAW *pDraw)
{
    unsigned long s = millis();
    gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
    total_show_video += millis() - s;
    return 1;
}

void mjpegPlayFromSDCard(char *mjpegFilename)
{
    File mjpegFile = SD.open(mjpegFilename, "r");

    if (!mjpegFile || mjpegFile.isDirectory())
    {
        Serial.printf("ERROR: Failed to open %s file for reading\n", mjpegFilename);
    }
    else
    {
        Serial.println(F("MJPEG start"));
        gfx->fillScreen(RGB565_BLACK);

        start_ms = millis();
        curr_ms = millis();
        total_frames = 0;
        total_read_video = 0;
        total_decode_video = 0;
        total_show_video = 0;

        mjpeg.setup(
            &mjpegFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
            0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);

        while (!skipRequested && mjpegFile.available() && mjpeg.readMjpegBuf())
        {
            // Check for touch input
            checkTouch();
            
            if (skipRequested) break;

            // Read video
            total_read_video += millis() - curr_ms;
            curr_ms = millis();

            // Play video
            mjpeg.drawJpg();
            total_decode_video += millis() - curr_ms;

            curr_ms = millis();
            total_frames++;
        }
        
        int time_used = millis() - start_ms;
        Serial.println(F("MJPEG end"));
        mjpegFile.close();
        skipRequested = false; // ready for next video
        
        // Check if we should return to home screen
        if (currentState == HOME_SCREEN) {
            showHomeScreen();
            return;
        }
        
        float fps = 1000.0 * total_frames / time_used;
        total_decode_video -= total_show_video;
        Serial.printf("Total frames: %d\n", total_frames);
        Serial.printf("Time used: %d ms\n", time_used);
        Serial.printf("Average FPS: %0.1f\n", fps);
        Serial.printf("Read MJPEG: %lu ms (%0.1f %%)\n", total_read_video, 100.0 * total_read_video / time_used);
        Serial.printf("Decode video: %lu ms (%0.1f %%)\n", total_decode_video, 100.0 * total_decode_video / time_used);
        Serial.printf("Show video: %lu ms (%0.1f %%)\n", total_show_video, 100.0 * total_show_video / time_used);
    }
}

void loadMjpegFilesList()
{
    File mjpegDir = SD.open(MJPEG_FOLDER);
    if (!mjpegDir)
    {
        Serial.printf("Failed to open %s folder\n", MJPEG_FOLDER);
        while (true) {}
    }
    mjpegCount = 0;
    while (true)
    {
        File file = mjpegDir.openNextFile();
        if (!file) break;
        if (!file.isDirectory())
        {
            String name = file.name();
            if (name.endsWith(".mjpeg"))
            {
                mjpegFileList[mjpegCount] = name;
                mjpegFileSizes[mjpegCount] = file.size();
                mjpegCount++;
                if (mjpegCount >= MAX_FILES) break;
            }
        }
        file.close();
    }
    mjpegDir.close();
    Serial.printf("%d mjpeg files found\n", mjpegCount);
    for (int i = 0; i < mjpegCount; i++)
    {
        Serial.printf("File %d: %s, Size: %lu bytes\n", i, mjpegFileList[i].c_str(), mjpegFileSizes[i]);
    }
}