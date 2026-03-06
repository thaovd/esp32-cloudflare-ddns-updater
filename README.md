# ESP32 Cloudflare DDNS Updater

Automatic DNS updater for Cloudflare using ESP32. This firmware automatically detects public IP changes and updates your Cloudflare DNS records.

## Hardware Requirements

- **Microcontroller**: ESP32 (DOIT DevKit V1 or compatible)
- **Display**: SSD1306 OLED 128x64 (SPI interface)

### Pinout

| Component | Signal | GPIO |
|-----------|--------|------|
| OLED | SCLK (D0) | GPIO 15 |
| OLED | MOSI (D1) | GPIO 16 |
| OLED | RES | GPIO 4 |
| OLED | DC | GPIO 25 |
| OLED | CS | GPIO 17 |
| LED D2 | - | GPIO 2 |

## Installation

### 1. Hardware Setup

Connect OLED display to ESP32:
```
OLED Pin → ESP32 GPIO
-----------------------
VCC      → 3.3V
GND      → GND
D0       → GPIO 15 (SCLK)
D1       → GPIO 16 (MOSI)
RES      → GPIO 4
DC       → GPIO 25
CS       → GPIO 17
```

Optional: Connect LED to GPIO 2 and GND for status indication.

### 2. Clone Repository

```bash
git clone https://github.com/thaovd/esp32-cloudflare-ddns-updater.git
cd esp-ddns
```

### 3. PlatformIO Setup

```bash
# Install PlatformIO
pip install platformio

# Install dependencies
pio lib install

# Build project
pio run
```

### 4. Configuration

Edit `include/config.h`:

```cpp
// WiFi
#define WIFI_SSID "Your-WiFi-SSID"
#define WIFI_PASSWORD "Your-WiFi-Password"

// Cloudflare
#define CF_API_TOKEN "Your-API-Token"
#define CF_ZONE_ID "Your-Zone-ID"
#define CF_RECORD_ID "Your-Record-ID"
#define CF_DOMAIN "your-domain.com"

// Check interval (milliseconds)
#define UPDATE_INTERVAL 30000  // 30 seconds (testing)
// Change to 300000 for 5 minutes (production)
```

**How to get Cloudflare credentials:**
1. Go to https://dash.cloudflare.com
2. Select your domain → DNS
3. Find your A record, click "Edit"
4. Copy Record ID from URL: `/dns_records/[RECORD_ID]`
5. Create API Token: Account → API Tokens → Create Token
6. Get Zone ID: Domain Overview → right panel

### 5. Upload Firmware

```bash
pio run --target upload
```

## Configuration Details

### Timeout Settings

```cpp
#define WIFI_TIMEOUT 20000      // WiFi connection timeout
#define IPCHECK_TIMEOUT 10000   // Public IP check timeout
#define CF_UPDATE_TIMEOUT 15000 // Cloudflare API timeout
```

### Timezone

Default: UTC+7 (Vietnam). To change:

In `src/main.cpp`, modify `configTime()`:
```cpp
// UTC+7: 25200 seconds (7 * 3600)
// UTC+0: 0 seconds
// UTC-5: -18000 seconds
configTime(OFFSET_SECONDS, 0, "pool.ntp.org", "time.nist.gov");
```

## LED Status Indicator

**LED D2 (GPIO 2) behavior:**

| Status | Indication |
|--------|-----------|
| 🟢 **Solid ON** | All systems OK (WiFi ✓ IP ✓ Cloudflare ✓) |
| 🟡 **Blinking 50ms** | Missing one condition (WiFi, IP, or Cloudflare) |
| 🔴 **OFF** | System initializing |

## OLED Display Layout

```
=== DDNS UPDATER ===

IP: 14.177.XXX.XXX
WiFi: Connected
Status: Ready
Next: 120s
Sync: 06/03 14:30
```

**Display Lines:**
1. Title
2. Public IP (from api.ipify.org)
3. WiFi connection status
4. Current operation status
5. Countdown to next IP check
6. Last successful Cloudflare update time

## Operation Flow

### Startup
1. Initialize display and LED
2. Connect to WiFi
3. Sync system time (NTP)
4. **Perform initial IP check immediately**
5. Update Cloudflare DNS

### Normal Operation
1. Every second: Update countdown timer, check LED status
2. Every `UPDATE_INTERVAL`: Check IP, update if changed
3. LED status updated each loop based on conditions

### Display shows noise/corruption
- Check SPI pin connections
- Lower OLED refresh rate
- Verify 3.3V power supply is stable

### WiFi won't connect
- Check SSID and password in config.h
- Verify WiFi signal strength
- Check `WIFI_TIMEOUT` setting

### API errors (405, 401)
- Verify CF_RECORD_ID is correct
- Check API token has DNS:Write permission
- Confirm Zone ID is correct

### IP not updating
- Check api.ipify.org is accessible (HTTPS)
- Verify `IPCHECK_TIMEOUT` is sufficient
- Check Serial monitor for error messages

### Time sync fails
- Ensure NTP servers are reachable
- Check `configTime()` offset is correct for your timezone

## Serial Monitor Output

Monitor firmware status:
```bash
pio device monitor --port COM3 --baud 115200
```



## Libraries Used

- **Adafruit_SSD1306@2.5.10** - OLED display driver
- **Adafruit_GFX@1.11.9** - Graphics library
- **ArduinoJson@6.21.0** - JSON parsing for API
- **WiFiClientSecure** - HTTPS support (built-in)

## Technical Specifications

- **Update Interval**: 30 seconds (testing) / 300 seconds (production)
- **Display Refresh**: ~1 second
- **LED Blink Rate**: 50ms (when degraded)
- **HTTP Timeouts**: 10-20 seconds
- **Timezone**: UTC+7 (configurable)

## API Endpoints

- **IP Check**: `GET https://api.ipify.org/` → Returns current public IP
- **DNS Update**: `PUT https://api.cloudflare.com/client/v4/zones/{zone_id}/dns_records/{record_id}`

## Future Improvements

- [ ] Web-based configuration portal
- [ ] Multiple DNS records support
- [ ] DHCP hostname auto-detection
- [ ] Email notifications
- [ ] Syslog integration
- [ ] MQTT support

## License

MIT License - See LICENSE file for details

## Support

For issues, feature requests, or questions:
- Check Serial monitor output
- Review configuration in config.h
- Verify hardware connections
- Test with Serial Monitor enabled

## Author

Created for automatic Cloudflare DDNS management on ESP32

---

**Last Updated**: March 6, 2026  
**Firmware Version**: 1.0  
**Compatible with**: ESP32 DOIT DevKit V1+
