#pragma once
#include <stdint.h>
namespace Config {
constexpr uint8_t UP_PIN=14,DOWN_PIN=13,ENTER_PIN=12,BACK_PIN=27;
constexpr uint8_t SDA_PIN=21,SCL_PIN=22,OLED_ADDRESS=0x3C;
constexpr uint32_t I2C_HZ=400000,FRAME_MS=83,DISPLAY_MS=20000;
constexpr uint32_t SCAN_TIMEOUT_MS=15000,SCAN_COOLDOWN_MS=10000,TRACK_PERIOD_MS=1000;
constexpr uint32_t POWER_HOLD_MS=2000;
constexpr char VERSION[]="4.2.4-HW2";
constexpr bool REDUCED_MOTION=false;
constexpr char AP_SSID[]="ESP32-LAB";
constexpr char AP_PASSWORD[]="Lab32-2026";
}
// Production always uses real radio/system readings. The separate Wokwi
// validation build explicitly opts into demo data and is labelled DEMO.
#ifndef DECK_DEMO
#define DECK_DEMO 0
#endif
#ifndef DECK_SELF_TEST
#define DECK_SELF_TEST 0
#endif
