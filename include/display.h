#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "config.h"

// Pin definitions for SPI OLED
// Physical OLED pins -> ESP32 GPIO mapping
#define OLED_CS    17  // CS pin (Chip Select)
#define OLED_DC    25  // DC pin (Data/Command) - use any free GPIO
#define OLED_RST   4   // RES pin (Reset)
#define OLED_MOSI  16  // D1 pin (MOSI/Data)
#define OLED_SCLK  15  // D0 pin (Clock)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create OLED display object (128x64, SPI)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_SCLK, OLED_DC, OLED_RST, OLED_CS);

// Display buffer for log lines
String displayLines[6];
int displayLineCount = 0;
String publicIP = "---";
String wifiStatus = "Disconnected";
String updateStatus = "Never";
String currentStatus = "Initializing...";
int countdownSeconds = 0;
time_t lastUpdateTimestamp = 0;

// Forward declarations
void refreshDisplay();
void displayLog(String msg);
void displayStatus(String status);
void displayIP(String ip);
void updateWiFiStatus(String status);
void updateCountdown(int seconds);

String formatDateTime(time_t t) {
    if (t == 0) return "Never";
    
    struct tm* timeinfo = localtime(&t);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m %H:%M", timeinfo);
    return String(buffer);
}

void initDisplay() {
    Serial.println("\n=== Initializing OLED Display ===");
    
    // Setup GPIO pins
    pinMode(OLED_CS, OUTPUT);
    pinMode(OLED_DC, OUTPUT);
    pinMode(OLED_RST, OUTPUT);
    pinMode(OLED_SCLK, OUTPUT);
    pinMode(OLED_MOSI, OUTPUT);
    
    digitalWrite(OLED_CS, HIGH);
    
    Serial.println("GPIO pins configured");
    
    // Reset display
    digitalWrite(OLED_RST, HIGH);
    delay(10);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);
    delay(100);
    
    Serial.println("Hardware reset done");
    
    // Initialize display with SPI
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("SSD1306 allocation failed!");
        while(1);
    }
    
    Serial.println("Display initialized!");
    
    // Clear and set text
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32 DDNS");
    display.println("Initializing...");
    display.display();
    
    Serial.println("Display ready!\n");
}

void displayLog(String msg) {
    // Trim message
    if (msg.length() > 20) {
        msg = msg.substring(0, 20);
    }
    
    Serial.println("Log: " + msg);
    
    // Add to buffer
    if (displayLineCount < 6) {
        displayLines[displayLineCount] = msg;
        displayLineCount++;
    } else {
        // Scroll up
        for (int i = 0; i < 5; i++) {
            displayLines[i] = displayLines[i + 1];
        }
        displayLines[5] = msg;
    }
    
    // Redraw display
    refreshDisplay();
}

void refreshDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    int yPos = 0;
    
    // Title
    display.setCursor(0, yPos);
    display.println("=== DDNS UPDATER ===");
    yPos += 10;
    
    // IP Public
    display.setCursor(0, yPos);
    display.print("IP: ");
    display.println(publicIP);
    yPos += 8;
    
    // WiFi Status
    display.setCursor(0, yPos);
    display.print("WiFi: ");
    display.println(wifiStatus);
    yPos += 8;
    
    // Current Status
    display.setCursor(0, yPos);
    display.print("Status: ");
    display.println(currentStatus);
    yPos += 8;
    
    // Countdown
    display.setCursor(0, yPos);
    display.print("Next: ");
    display.print(countdownSeconds);
    display.println("s");
    yPos += 8;
    
    display.setCursor(0, yPos);
    display.print("Sync: ");
    display.println(formatDateTime(lastUpdateTimestamp));
    
    display.display();
}

void displayStatus(String status) {
    Serial.println("Status: " + status);
    currentStatus = status;
    if (status.indexOf("OK") >= 0 || status.indexOf("Updated") >= 0) {
        lastUpdateTimestamp = time(nullptr);
    }
    refreshDisplay();
}

void displayIP(String ip) {
    if (ip.length() > 0) {
        publicIP = ip;
    }
    
    Serial.println("IP: " + ip);
    refreshDisplay();
}

void updateWiFiStatus(String status) {
    wifiStatus = status;
    refreshDisplay();
}

void updateCountdown(int seconds) {
    countdownSeconds = seconds;
    refreshDisplay();
}

#endif
