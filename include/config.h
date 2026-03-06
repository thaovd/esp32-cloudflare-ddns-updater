#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Cloudflare Configuration
#define CF_API_TOKEN ""
#define CF_ZONE_ID ""
#define CF_RECORD_ID ""  // Get this from Cloudflare Dashboard
#define CF_DOMAIN ""

// Update Interval (milliseconds)
#define UPDATE_INTERVAL 120000   // 120 seconds (for testing, change to 300000 for 5 minutes)

// Timeouts
#define WIFI_TIMEOUT 20000      // 20 seconds
#define IPCHECK_TIMEOUT 10000   // 10 seconds
#define CF_UPDATE_TIMEOUT 15000 // 15 seconds

// LED Configuration
#define LED_D2_PIN 2  // GPIO 2 for D2 LED

#endif
