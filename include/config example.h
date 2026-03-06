#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "BVDK HN Xin Chao"
#define WIFI_PASSWORD "29hanthuyen"

// Cloudflare Configuration
#define CF_API_TOKEN "98H1PlgCP4sDRNBv3GKb4QCz1MIm2EttKTeKIzcf"
#define CF_ZONE_ID "26b6b59db864b9a55fc411be31cd6f4a"
#define CF_RECORD_ID "0bca17fd455c5d402b27e12d8cc8068f"  // Get this from Cloudflare Dashboard
#define CF_DOMAIN "ddns.vuthao.id.vn"

// Update Interval (milliseconds)
#define UPDATE_INTERVAL 120000   // 120 seconds (for testing, change to 300000 for 5 minutes)

// Timeouts
#define WIFI_TIMEOUT 20000      // 20 seconds
#define IPCHECK_TIMEOUT 10000   // 10 seconds
#define CF_UPDATE_TIMEOUT 15000 // 15 seconds

// LED Configuration
#define LED_D2_PIN 2  // GPIO 2 for D2 LED

#endif
