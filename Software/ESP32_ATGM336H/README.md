# ESP32 + ATGM336H GPS-only Tracker

This is a minimal Arduino/PlatformIO firmware for an ESP32 connected to an ATGM336H GPS module.
It keeps only GPS-based tracking data:

- current latitude/longitude
- satellites
- current speed
- average speed
- max speed
- trip distance
- moving time

Calories and heart-rate features are intentionally not included.

## Wiring

| ATGM336H | ESP32 default pin |
| --- | --- |
| TX | GPIO16 / RX2 |
| RX | GPIO17 / TX2, optional |
| VCC | 3V3 or 5V according to your module board |
| GND | GND |

If you use different pins, edit `GPS_RX_PIN` and `GPS_TX_PIN` in `src/main.cpp`.

## Build and upload

```bash
cd Software/ESP32_ATGM336H
pio run
pio run -t upload
pio device monitor
```

The project reuses the bundled TinyGPS++ library from `../X-Track/Libraries`.
