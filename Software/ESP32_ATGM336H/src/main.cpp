#include <Arduino.h>
#include <TinyGPS++.h>

/*
 * ESP32 + ATGM336H GPS-only tracker example.
 *
 * Wiring (default):
 *   ATGM336H TX -> ESP32 GPIO16 (RX2)
 *   ATGM336H RX -> ESP32 GPIO17 (TX2), optional
 *   ATGM336H VCC -> 3V3 or 5V according to your module board
 *   ATGM336H GND -> ESP32 GND
 *
 * This firmware intentionally has no calorie and no heart-rate features.
 */

static constexpr uint32_t DEBUG_BAUD = 115200;
static constexpr uint32_t GPS_BAUD = 9600;
static constexpr int GPS_RX_PIN = 16;
static constexpr int GPS_TX_PIN = 17;
static constexpr uint32_t PRINT_PERIOD_MS = 1000;
static constexpr double MIN_DISTANCE_STEP_M = 1.0;

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(2);

static bool hasLastPoint = false;
static double lastLat = 0.0;
static double lastLng = 0.0;
static double tripMeters = 0.0;
static uint32_t movingMillis = 0;
static uint32_t lastUpdateMs = 0;
static uint32_t lastPrintMs = 0;
static float maxSpeedKmph = 0.0f;

static void updateTrip()
{
    if (!gps.location.isUpdated() || !gps.location.isValid())
    {
        return;
    }

    const double lat = gps.location.lat();
    const double lng = gps.location.lng();

    if (hasLastPoint)
    {
        const double step = TinyGPSPlus::distanceBetween(lastLat, lastLng, lat, lng);
        if (step >= MIN_DISTANCE_STEP_M)
        {
            tripMeters += step;
        }
    }
    else
    {
        hasLastPoint = true;
    }

    lastLat = lat;
    lastLng = lng;
}

static void updateMovingTime(uint32_t now)
{
    if (lastUpdateMs == 0)
    {
        lastUpdateMs = now;
        return;
    }

    const uint32_t elapsed = now - lastUpdateMs;
    lastUpdateMs = now;

    if (gps.speed.isValid() && gps.speed.kmph() > 1.0)
    {
        movingMillis += elapsed;
    }
}

static void printStatus()
{
    Serial.println(F("---- GPS tracker ----"));

    if (gps.location.isValid())
    {
        Serial.print(F("Lat: "));
        Serial.println(gps.location.lat(), 6);
        Serial.print(F("Lng: "));
        Serial.println(gps.location.lng(), 6);
    }
    else
    {
        Serial.println(F("Location: invalid"));
    }

    Serial.print(F("Satellites: "));
    Serial.println(gps.satellites.isValid() ? gps.satellites.value() : 0);

    const float speedKmph = gps.speed.isValid() ? gps.speed.kmph() : 0.0f;
    if (speedKmph > maxSpeedKmph)
    {
        maxSpeedKmph = speedKmph;
    }

    const float avgSpeedKmph = movingMillis > 0
        ? static_cast<float>((tripMeters / 1000.0) / (movingMillis / 3600000.0))
        : 0.0f;

    Serial.print(F("Speed: "));
    Serial.print(speedKmph, 1);
    Serial.println(F(" km/h"));

    Serial.print(F("Avg speed: "));
    Serial.print(avgSpeedKmph, 1);
    Serial.println(F(" km/h"));

    Serial.print(F("Max speed: "));
    Serial.print(maxSpeedKmph, 1);
    Serial.println(F(" km/h"));

    Serial.print(F("Trip: "));
    Serial.print(tripMeters / 1000.0, 3);
    Serial.println(F(" km"));

    Serial.print(F("Moving time: "));
    Serial.print(movingMillis / 1000);
    Serial.println(F(" s"));
}

void setup()
{
    Serial.begin(DEBUG_BAUD);
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    Serial.println(F("ESP32 + ATGM336H GPS-only tracker"));
    Serial.println(F("No calorie or heart-rate functions are included."));
}

void loop()
{
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());
    }

    const uint32_t now = millis();
    updateTrip();
    updateMovingTime(now);

    if (now - lastPrintMs >= PRINT_PERIOD_MS)
    {
        lastPrintMs = now;
        printStatus();
    }
}
