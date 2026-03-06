#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>
#include "../include/config.h"
#include "../include/display.h"

// Global variables
String currentIP = "";
String lastIP = "";
unsigned long lastUpdateTime = 0;

// LED status tracking
bool wifiConnected = false;
bool hasPublicIP = false;
bool cloudflareUpdated = false;
static unsigned long lastLEDBlinkTime = 0;
static bool ledBlinkState = false;

// Function prototypes
void connectWiFi();
void updateSystemTime();
String getPublicIP();
void updateCloudflareIP(String ip);
void updateLEDStatus();
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    // Initialize display
    initDisplay();
    displayLog("Serial: Starting...");
    
    Serial.println("\n\n=== ESP32 DDNS Updater ===");
    Serial.println("Configuration:");
    Serial.print("  WiFi SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("  Domain: ");
    Serial.println(CF_DOMAIN);
    Serial.print("  Record ID: ");
    Serial.println(CF_RECORD_ID);
    Serial.println("");
    
    displayLog("SSID: " + String(WIFI_SSID));
    displayLog("Domain: " + String(CF_DOMAIN));
    
    // Setup LED D2
    pinMode(LED_D2_PIN, OUTPUT);
    digitalWrite(LED_D2_PIN, LOW);
    Serial.println("LED D2 initialized on GPIO 2");
    
    // Validate config
    if (String(CF_RECORD_ID) == "PUT_YOUR_RECORD_ID_HERE") {
        Serial.println("❌ ERROR: Please set CF_RECORD_ID in config.h");
        displayStatus("ERROR: Set Record ID");
        while(1) delay(1000);
    }
    
    Serial.println("Starting...\n");
}

void loop() {
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connecting to WiFi...");
        updateWiFiStatus("Connecting...");
        connectWiFi();
    }
    
    // Update system time on first connection and do initial IP check
    static bool timeUpdated = false;
    if (!timeUpdated && WiFi.status() == WL_CONNECTED) {
        updateSystemTime();
        timeUpdated = true;
        Serial.println("Ready for IP checks!");
        
        // Perform initial IP check immediately on startup
        Serial.println("Performing initial IP check...");
        displayStatus("Checking...");
        
        String newIP = getPublicIP();
        if (newIP.length() > 0) {
            displayLog("IP: " + newIP);
            updateCloudflareIP(newIP);
            currentIP = newIP;
            displayStatus("Cloudf: OK");
        }
        
        // Reset timer so first interval starts after this initial check
        lastUpdateTime = millis();
        displayStatus("Ready");
    }
    
    // Update countdown timer every 1 second
    static unsigned long lastCountdownUpdate = 0;
    if (millis() - lastCountdownUpdate >= 1000) {
        lastCountdownUpdate = millis();
        
        // Calculate remaining seconds until next IP check
        unsigned long timeSinceUpdate = millis() - lastUpdateTime;
        int secondsRemaining = (UPDATE_INTERVAL - timeSinceUpdate) / 1000;
        if (secondsRemaining < 0) secondsRemaining = 0;
        
        updateCountdown(secondsRemaining);
    }
    
    // Check IP every UPDATE_INTERVAL
    if (millis() - lastUpdateTime >= UPDATE_INTERVAL) {
        lastUpdateTime = millis();
        Serial.print("[");
        Serial.print(millis() / 1000);
        Serial.print("s] Checking IP...");
        
        displayStatus("Checking IP...");
        
        String newIP = getPublicIP();
        if (newIP.length() > 0) {
            if (newIP != currentIP) {
                Serial.println("⚡ IP changed! Updating Cloudflare...");
                displayStatus("IP Changed!");
                delay(500);
                
                displayStatus("Updating CF...");
                displayLog("IP: " + newIP);
                updateCloudflareIP(newIP);
                currentIP = newIP;
                
                displayStatus("Cloudf: OK");
            } else {
                Serial.println(" (no change)");
                displayStatus("Ready");
            }
        } else {
            Serial.println(" (no response)");
            displayStatus("Ready");
        }
    }
    
    // Update LED status based on conditions
    updateLEDStatus();
    
    delay(1000);
}

void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    updateWiFiStatus("Connecting...");
    Serial.println("Connecting to WiFi...");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi connected!");
        Serial.print("  LAN IP: ");
        Serial.println(WiFi.localIP());
        
        updateWiFiStatus("Connected");
        // Don't display LAN IP - will show WAN IP from API check later
    } else {
        Serial.println("\n✗ WiFi connection failed");
        updateWiFiStatus("Failed");
    }
}

void updateSystemTime() {
    Serial.print("Syncing time");
    // UTC+7 timezone for Vietnam: 7 * 3600 = 25200 seconds, no daylight saving
    configTime(25200, 0, "pool.ntp.org", "time.nist.gov");
    
    time_t now = time(nullptr);
    int attempts = 0;
    while (now < 24 * 3600 && attempts < 30) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        attempts++;
    }
    
    Serial.println(" Done!");
}

String getPublicIP() {
    Serial.print("  Checking IP from api.ipify.org...");
    displayStatus("Checking IP...");
    
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(IPCHECK_TIMEOUT / 1000);
    
    if (!client.connect("api.ipify.org", 443)) {
        Serial.println(" ✗ Connection failed");
        displayLog("IP check: Connection failed");
        return "";
    }
    
    Serial.print(" connected, sending request...");
    
    client.println("GET / HTTP/1.1");
    client.println("Host: api.ipify.org");
    client.println("Connection: close");
    client.println();
    
    unsigned long timeout = millis() + IPCHECK_TIMEOUT;
    String response = "";
    
    while (millis() < timeout && (client.connected() || client.available())) {
        if (client.available()) {
            char c = client.read();
            response += c;
            if (response.length() > 2000) break; // Safety limit
        }
    }
    
    client.stop();
    
    // Extract IP from response
    int bodyStart = response.indexOf("\r\n\r\n");
    if (bodyStart > 0) {
        String body = response.substring(bodyStart + 4);
        body.trim();
        Serial.print(" ✓ IP: ");
        Serial.println(body);
        displayIP(body);  // Update display with public IP
        return body;
    }
    
    Serial.println(" ✗ No valid response");
    return "";
}

void updateCloudflareIP(String ip) {
    Serial.println("\nUpdating Cloudflare...");
    displayStatus("Updating CF...");
    
    WiFiClientSecure client;
    client.setInsecure();
    
    if (!client.connect("api.cloudflare.com", 443)) {
        Serial.println("  ✗ Failed to connect Cloudflare API");
        displayLog("CF: Connection failed");
        return;
    }
    
    // Create JSON payload
    StaticJsonDocument<256> doc;
    doc["type"] = "A";
    doc["name"] = CF_DOMAIN;
    doc["content"] = ip;
    doc["ttl"] = 120;
    doc["proxied"] = false;
    
    String json;
    serializeJson(doc, json);
    
    // Build request
    String endpoint = String("/client/v4/zones/") + CF_ZONE_ID + "/dns_records/" + CF_RECORD_ID;
    String request = "PUT " + endpoint + " HTTP/1.1\r\n";
    request += "Host: api.cloudflare.com\r\n";
    request += "Authorization: Bearer " + String(CF_API_TOKEN) + "\r\n";
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + String(json.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += json;
    
    Serial.print("  Sending request: PUT /dns_records/");
    Serial.println(CF_RECORD_ID);
    
    client.print(request);
    
    // Read response
    unsigned long timeout = millis() + CF_UPDATE_TIMEOUT;
    String response = "";
    
    while (millis() < timeout && (client.connected() || client.available())) {
        if (client.available()) {
            response += char(client.read());
        }
    }
    
    client.stop();
    
    // Check response
    if (response.indexOf("200") > 0 || response.indexOf("\"success\":true") > 0) {
        Serial.println("  ✓ Cloudflare updated successfully!");
        displayStatus("Updated OK");
        displayLog("Cloudflare: OK");
    } else if (response.indexOf("405") > 0) {
        Serial.println("  ✗ 405 Error - Check Record ID");
        displayStatus("Error 405 - Record ID?");
        displayLog("CF: 405 Error");
        Serial.println(response.substring(0, 300));
    } else {
        Serial.println("  ✗ Update failed!");
        displayStatus("Update failed");
        displayLog("CF: Update failed");
        Serial.println(response.substring(0, 300));
    }
}
void updateLEDStatus() {
    // Check 3 conditions
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    hasPublicIP = (currentIP.length() > 0 && currentIP != "---");
    cloudflareUpdated = (lastUpdateTimestamp > 0);
    
    // All 3 conditions met: LED stays ON continuously
    if (wifiConnected && hasPublicIP && cloudflareUpdated) {
        digitalWrite(LED_D2_PIN, HIGH);
    } 
    // Missing at least 1 condition: Blink LED 50ms
    else {
        if (millis() - lastLEDBlinkTime >= 50) {
            lastLEDBlinkTime = millis();
            ledBlinkState = !ledBlinkState;
            digitalWrite(LED_D2_PIN, ledBlinkState ? HIGH : LOW);
        }
    }
}