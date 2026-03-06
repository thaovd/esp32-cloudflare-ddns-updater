// User_Setup.h for ST7735S IPS 80x160 display

#define ST7735_DRIVER

// Display dimensions
#define TFT_WIDTH  80
#define TFT_HEIGHT 160

// Pin assignments - match your schematic
#define TFT_CS    17  // CS
#define TFT_RST   4   // RES
#define TFT_DC    -1  // Not used for ST7735 in some configs
#define TFT_MOSI  16  // SDA (MOSI)
#define TFT_SCLK  15  // SCL

// ST7735 specific
#define ST7735_TABCOLOR INITR_BLACKTAB  // or INITR_REDTAB or INITR_GREENTAB

// SPI frequency
#define SPI_FREQUENCY  27000000

// Font loads
#define LOAD_GLCD
#define LOAD_FONT2

// No touch
#define TOUCH_CS -1

