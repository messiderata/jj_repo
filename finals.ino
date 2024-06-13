#include <Arduino.h>
#include <Wire.h>
#include <Button.h>
#include "U8g2lib.h"
#include "RTClib.h"
#include <Adafruit_INA219.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

RTC_DS3231 rtc;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
Adafruit_INA219 ina219;

// ' icon_manual', 16x16px
const unsigned char epd_bitmap__icon_ina[] PROGMEM = {
  0xff, 0xff, 0x80, 0x01, 0x83, 0xc1, 0x8d, 0xb1, 0x98, 0x19, 0x90, 0x09, 0xb0, 0x0d, 0xb0, 0x05,
  0x80, 0x11, 0x80, 0x21, 0x80, 0x41, 0x83, 0x81, 0x85, 0x41, 0x83, 0x81, 0x80, 0x01, 0xff, 0xff
};
// ' icon_monitoring', 16x16px
const unsigned char epd_bitmap__icon_monitoring[] PROGMEM = {
  0xff, 0xff, 0x80, 0x01, 0x80, 0x09, 0x80, 0x31, 0x80, 0xc1, 0x83, 0x01, 0x8c, 0x01, 0x90, 0x01,
  0xa0, 0x01, 0x80, 0x01, 0xff, 0xff, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x7f, 0xfe, 0xff, 0xff
};
// ' icon_timer', 16x16px
const unsigned char epd_bitmap__icon_timer[] PROGMEM = {
  0x0f, 0xf0, 0x10, 0x08, 0x20, 0x04, 0x41, 0x02, 0x81, 0x01, 0x81, 0x01, 0x83, 0x81, 0x82, 0xfd,
  0x81, 0x81, 0x80, 0x01, 0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x0f, 0xf0, 0x00, 0x00
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 144)

// ' icon_scrollbar', 8x64px
const unsigned char epd_bitmap__icon_scrollbar[] PROGMEM = {
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02
};
// ' icon_selector', 128x21px
const unsigned char epd_bitmap__icon_selector[] PROGMEM = {
  0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
  0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
  0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80
};

// ' item_sel_bg', 128x20px
const unsigned char epd_bitmap__item_sel_bg[] PROGMEM = {
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0,
  0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0,
  0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80
};

const unsigned char epd_bitmap_border[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0xf7, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char epd_bitmap_off[] PROGMEM = {
  0xf8, 0xf8, 0xf8, 0xf8, 0xf8
};

const unsigned char epd_bitmap_on[] PROGMEM = {
  0xf8, 0xf8, 0xf8, 0xf8, 0xf8
};


const unsigned char epd_bitmap__person[] PROGMEM = {
  0x3f, 0xfc, 0x40, 0x02, 0x82, 0x11, 0x82, 0x11, 0x80, 0x01, 0x82, 0x09, 0x81, 0xf1, 0x40, 0x02,
  0x3f, 0xfc, 0x10, 0x84, 0x10, 0x84, 0x10, 0x84, 0x10, 0x84, 0x10, 0x84, 0x10, 0xc4, 0x1f, 0xbc
};

const unsigned char *bitmap_icons[3] = {
  epd_bitmap__icon_ina,
  epd_bitmap__icon_monitoring,
  epd_bitmap__person

};

const unsigned char *bitmap_lamp_icons[5] = {
  epd_bitmap__person,
  epd_bitmap__person,
  epd_bitmap__person,
  epd_bitmap__person,
  epd_bitmap__person,
};

const int NUM_ITEMS = 3;
const int LAMP_NUM_ITEMS = 4;
const int PERSON_NUM_ITEMS = 5;


const char *menu_items[NUM_ITEMS] = {
  "INA 219",
  "Monitoring",
  "Member"
};

const char *person_menu_items[PERSON_NUM_ITEMS] = {
  "Prudencio",
  "Amista",
  "Alvdendia",
  "Manginsay",
  "Rivera",
};

const char *lamp_menu_items[LAMP_NUM_ITEMS] = {
  "LAMP 1",
  "LAMP 2",
  "LAMP 3",
  "LAMP 4",

};



#define WIFI_SSID "Meralco"
#define WIFI_PASSWORD "Amistacompany"

S
#define API_KEY "AIzaSyDPoMBEE6yq2v3qroCGrjZlyC5uUwDmtbM"
#define DATABASE_URL "https://jowsep-ce721-default-rtdb.firebaseio.com/"


const int relayPin[] = { 26, 25, 33, 32 };
const int switchPin[] = { 13, 12, 23, 27 };
const int numberOfButtons = sizeof(switchPin) / sizeof(switchPin[0]);

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

int mainMenuSelection = 0;
int submenuSelection = 0;


int lastState = LOW;
int count = 0;
int controlSwitch;
int scrollOffset = 0;       // Offset for scrolling
const int scrollStep = 10;  // Number of pixels to scroll per step

bool inSubMenu = false;
bool inLampDisplay = false;
bool scheduleDisplay = false;
bool value;

#define UP_BUTTON_PIN 17
#define DOWN_BUTTON_PIN 4
#define ENTER_BUTTON_PIN 18
#define BACK_BUTTON_PIN 19
#define SCREEN_BUTTON 5
#define CONTROL_PIN 16

#define EEPROM_SIZE 512

#define EEPROM_ADDR_LAMP1_ON_HOUR 8
#define EEPROM_ADDR_LAMP1_ON_MINUTE 9
#define EEPROM_ADDR_LAMP1_ON_SECOND 10
#define EEPROM_ADDR_LAMP1_ON_PERIOD 11

#define EEPROM_ADDR_LAMP1_OFF_HOUR 12
#define EEPROM_ADDR_LAMP1_OFF_MINUTE 13
#define EEPROM_ADDR_LAMP1_OFF_SECOND 14
#define EEPROM_ADDR_LAMP1_OFF_PERIOD 15

#define EEPROM_ADDR_LAMP2_ON_HOUR 16
#define EEPROM_ADDR_LAMP2_ON_MINUTE 17
#define EEPROM_ADDR_LAMP2_ON_SECOND 18
#define EEPROM_ADDR_LAMP2_ON_PERIOD 19


int lampState1 = HIGH;
int lampState2 = HIGH;
int lampState3 = HIGH;
int lampState4 = HIGH;

int selectedComponent = 0;

int time_s = 0;
int time_m = 0;
int time_h = 0;  // Initialize to a valid 12-hour format
int startHour;
int startMinute;
int startSecond;

int endHour;
int endMinute;
int endSecond;
int lampSelected = 0;
bool endPeriod;  // 0 for AM, 1 for PM

bool manualMode = true;
bool manualControlActive = false;
bool startPeriod;  // 0 for AM, 1 for PM
bool isPM = false;
bool isPhone;

Button upButton(UP_BUTTON_PIN);
Button downButton(DOWN_BUTTON_PIN);
Button enterButton(ENTER_BUTTON_PIN);
Button backButton(BACK_BUTTON_PIN);
Button screenButton(SCREEN_BUTTON);
Button controlSwitching(CONTROL_PIN);

float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;
float shuntvoltage = 0;
float busvoltage = 0;

Button *buttons[numberOfButtons];


unsigned long previousMillis = 0;
const unsigned long interval = 10;

unsigned long previousSensorMillis = 0;
unsigned long sensorInterval = 2000;



bool isTimerIsOn;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData lampControl;
FirebaseData modeControl;

bool signupOK = false;
bool wifiConnected = false;

TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  u8g2.setColorIndex(1);  // set the color to white
  u8g2.begin();
  u8g2.setBitmapMode(1);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 1, 42, 0));
  }

  //   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  Wire.beginTransmission(0x68);  // address DS3231
  Wire.write(0x0E);              // select register
  Wire.write(0b00011100);        // write register bitmap, bit 7 is /EOSC
  Wire.endTransmission();

  while (!Serial) {
    delay(1);
  }

  upButton.begin();
  downButton.begin();
  enterButton.begin();
  backButton.begin();
  screenButton.begin();
  controlSwitching.begin();


  // // Initialize the INA219.
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }
  ina219.setCalibration_16V_400mA();
  Serial.println("Measuring voltage, current, and power with INA219 ...");
  ina219.begin();


  for (int i = 0; i < numberOfButtons; i++) {
    buttons[i] = new Button(switchPin[i]);
    buttons[i]->begin();
    pinMode(relayPin[i], OUTPUT);  // Initialize the relay pins as outputs
  }

  EEPROM.begin(EEPROM_SIZE);

  // Load values from EEPROM
  loadLampOffTimeFromEEPROM(lampSelected);
  loadLampOnTimeFromEEPROM(lampSelected);

  Serial.begin(115200);
  connectWiFi();

  // Initialize Firebase (if required)
  if (wifiConnected) {
    initializeFirebase();
  }
  xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, &Task1Handle, 0);
  xTaskCreatePinnedToCore(Task2, "Task2", 10000, NULL, 1, &Task2Handle, 1);
}

void loop() {
  DateTime now = rtc.now();
  unsigned long currentMillis = millis();

  if (currentMillis - previousSensorMillis >= sensorInterval) {
    previousSensorMillis = currentMillis;
    sensorData();
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Increment count or perform any other necessary actions based on button presses or other triggers
    if (screenButton.pressed()) {
      count++;
    }
    // Switch between different modes based on the value of count
    switch (count) {
      case 0:
        // Perform actions related to mode 0 (e.g., clock display, manual control)
        clockDisplay(now);
        break;
      case 1:

        // Perform actions related to mode 1 (e.g., timer function, menu display)
        if (inLampDisplay) {
          displaySelectedLamp(submenuSelection);
        } else if (inSubMenu) {
          displaySubMenu(now);
        } else {
          displayMainMenu(NUM_ITEMS, bitmap_icons, menu_items);
        }
        handleButtons();
        break;
      case 2:
        // Perform actions related to mode 2 (e.g., timer section, button press handling)
        timerSection();

        ButtonPress();

        break;
      default:
        // Reset count if it exceeds the maximum value
        count = 0;
        break;
    }
  }


  //  if (controlSwitching.pressed()) {
  //    controlSwitch ++;
  //
  //  }
  if (!isPhone) {  // Same as if (isPhone == false)
    switch (controlSwitch) {
      case 0:
        manualRelayControl();
        Serial.println("Manual Lamp Control");
        isTimerIsOn = false;
        break;
      case 1:
        timerFunction(now);
        Serial.println("Timer Lamp Control");
        isTimerIsOn = true;
        break;
      default:
        controlSwitch = 0;  // Default case to reset controlSwitch to 0
        break;
    }
  }

  loadLampOffTimeFromEEPROM(lampSelected);
  loadLampOnTimeFromEEPROM(lampSelected);
  handleScrollInput();
}


void handleButtons() {
  if (inLampDisplay) {
    // Only handle back button when in lamp display mode
    if (backButton.pressed()) {
      inLampDisplay = false;
    }
    return;  // Do not handle other buttons when in lamp display mode
  }

  // Handle button inputs and update menu state accordingly
  if (upButton.pressed()) {
    if (inSubMenu) {
      submenuSelection = (submenuSelection - 1 + PERSON_NUM_ITEMS) % PERSON_NUM_ITEMS;
    } else {
      mainMenuSelection = (mainMenuSelection - 1 + NUM_ITEMS) % NUM_ITEMS;
    }
  }
  if (downButton.pressed()) {
    if (inSubMenu) {
      submenuSelection = (submenuSelection + 1) % PERSON_NUM_ITEMS;
    } else {
      mainMenuSelection = (mainMenuSelection + 1) % NUM_ITEMS;
    }
  }
  if (enterButton.pressed()) {
    if (!inSubMenu) {
      inSubMenu = true;
    } else {
      if (mainMenuSelection == 1) {  // Ensure correct submenu
        inLampDisplay = true;
      }
    }
  }
  if (backButton.pressed()) {
    if (inSubMenu) {
      inSubMenu = false;
      submenuSelection = 0;
    }
  }
}

void ButtonPress() {
  if (enterButton.pressed()) {
    selectedComponent = (selectedComponent + 1) % 7;  // Cycle through components
  }

  if (upButton.pressed()) {
    if (selectedComponent == 0) {
      time_h = (time_h - 1 + 12) % 12;
      if (time_h == 0) time_h = 12;  // Handle 12-hour wrap around
    } else if (selectedComponent == 1) {
      time_m = (time_m + 1) % 60;  // Increment minutes
    } else if (selectedComponent == 2) {
      time_s = (time_s + 1) % 60;  // Increment seconds
    } else if (selectedComponent == 3) {
      isPM = !isPM;  // Toggle AM/PM
    } else if (selectedComponent == 4) {
      lampSelected = (lampSelected + 1) % 4;  // Cycle through lamps
    }
  }

  if (downButton.pressed()) {
    if (selectedComponent == 0) {
      time_h = (time_h + 1) % 12;
      if (time_h == 0) time_h = 12;  // Handle 12-hour wrap around
    } else if (selectedComponent == 1) {
      time_m = (time_m - 1 + 60) % 60;  // Decrement minutes
    } else if (selectedComponent == 2) {
      time_s = (time_s - 1 + 60) % 60;  // Decrement seconds
    } else if (selectedComponent == 3) {
      isPM = !isPM;  // Toggle AM/PM
    } else if (selectedComponent == 4) {
      lampSelected = (lampSelected - 1 + 4) % 4;  // Cycle through lamps
    }
  }
}

void displaySelectedLamp(int lampIndex) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g_font_5x8r);

  switch (lampIndex) {
    case 0:
      u8g2.drawStr(8, 8, "Philip Joshua F. Amista");
      u8g2.drawStr(8, 16, "Male");
      u8g2.drawStr(8, 24, "21");
      u8g2.drawStr(8, 32, "Marikina City");
      break;

    case 1:
      u8g2.drawStr(8, 8, "Kim Alvendia");
      u8g2.drawStr(8, 16, "Female");
      u8g2.drawStr(8, 24, "22");
      u8g2.drawStr(8, 32, "Marikina City");
      break;
    case 2:
      u8g2.drawStr(8, 8, "Mars Laurenz Manginsay");
      u8g2.drawStr(8, 16, "Male");
      u8g2.drawStr(8, 24, "21");
      u8g2.drawStr(8, 32, "Caloocan");

      break;
    case 3:
      u8g2.drawStr(8, 8, "James Rivera");
      u8g2.drawStr(8, 16, "Male");
      u8g2.drawStr(8, 24, "21");
      u8g2.drawStr(8, 32, "Laguna");

      break;
    case 4:
      u8g2.drawStr(8, 8, "Kimberly Prudencio");
      u8g2.drawStr(8, 16, "Female");
      u8g2.drawStr(8, 24, "23");
      u8g2.drawStr(8, 32, "Cavite");

      break;
  }

  u8g2.sendBuffer();
}

void displaySubMenu(DateTime now) {
  u8g2.clearBuffer();
  const char **menu = nullptr;  // Initialize menu pointer to nullptr

  switch (mainMenuSelection) {
    case 0:
      monitoringDisplay(now);
      handleScrollInput();

      break;  // Exit the function early
    case 1:
      menu = person_menu_items;
      break;
    case 2:
      sensorDisplay();
      return;  // Exit the function early
  }
  int itemHeight = 20;
  int y = 17;
  int x = 6;
  // Display submenu options only if menu is not nullptr
  if (menu != nullptr) {
    u8g2.drawBitmap(0, 22, 128 / 8, 21, epd_bitmap__icon_selector);
    u8g2.drawBitmap(120, 1, 8 / 8, 63, epd_bitmap__icon_scrollbar);
    u8g2.drawBox(125, 64 / PERSON_NUM_ITEMS * submenuSelection, PERSON_NUM_ITEMS, 64 / PERSON_NUM_ITEMS);

    for (int i = 0; i < PERSON_NUM_ITEMS; i++) {
      if (i == 1) {
        u8g2.setFont(u8g_font_7x14B);
      } else {
        u8g2.setFont(u8g_font_7x14);
      }
      u8g2.drawStr(25, y, person_menu_items[(submenuSelection + i) % PERSON_NUM_ITEMS]);
      u8g2.drawBitmap(x, y - 13, 16 / 8, 16, bitmap_lamp_icons[(submenuSelection + i) % PERSON_NUM_ITEMS]);
      y += itemHeight;
    }
  }

  u8g2.sendBuffer();
}

void displayMainMenu(int numItems, const unsigned char *Icons[], const char *Items[]) {
  u8g2.clearBuffer();
  int itemHeight = 20;
  int y = 17;
  int x = 6;

  u8g2.setFont(u8g_font_7x14);
  u8g2.drawBitmap(0, 22, 128 / 8, 21, epd_bitmap__icon_selector);
  u8g2.drawBitmap(120, 1, 8 / 8, 63, epd_bitmap__icon_scrollbar);
  u8g2.drawBox(125, 64 / numItems * mainMenuSelection, numItems, 64 / numItems);
  for (int i = 0; i < numItems; i++) {
    if (i == 1) {
      u8g2.setFont(u8g_font_7x14B);
    } else {
      u8g2.setFont(u8g_font_7x14);
    }
    u8g2.drawStr(25, y, Items[(mainMenuSelection + i) % numItems]);
    u8g2.drawBitmap(x, y - 13, 16 / 8, 16, Icons[(mainMenuSelection + i) % numItems]);
    y += itemHeight;
  }
  u8g2.sendBuffer();
}


void clockDisplay(DateTime now) {
  u8g2.clearBuffer();

  char timeString[20];
  char secondString[5];
  char dateString[50];
  char tempString[10];
  char periodString[6];

  float temperature = rtc.getTemperature();


  // formatDateTime(now, timeString, sizeof(timeString));
  dateDisplay(now, dateString, sizeof(dateString));
  periodDisplay(now, periodString, sizeof(periodString));

  // sprintf(timeString, "%02d:%02d", now.hour(), now.minute());
  formatDateTime(now, timeString, sizeof(timeString));
  sprintf(secondString, "%02d", now.second());
  dtostrf(temperature, 4, 1, tempString);

  u8g2.setFont(u8g_font_profont29r);
  u8g2.drawStr(20, 33, timeString);
  u8g2.setFont(u8g_font_profont15r);
  u8g2.drawStr(100, 23, secondString);
  u8g2.drawStr(100, 33, periodString);
  u8g2.setFont(u8g_font_profont12r);
  u8g2.drawStr(8, 8, dateString);

  u8g2.setFont(u8g_font_7x14);
  u8g2.drawStr(45, 48, tempString);

  if (isTimerIsOn == false) {
    u8g2.drawStr(8, 48, "M");
  } else {
    u8g2.drawStr(8, 48, "T");
  }
  //
  u8g2.drawGlyph(75, 48, 0xB0);  // Display the degree Celsius symbol

  u8g2.drawStr(83, 48, "C");  // Display the degree Celsius symbol
  // u8g2.setFont(u8g2_font_open_iconic_all_2x_t);
  if (lampState1 == HIGH) {
    u8g2.drawStr(40, 63, "-");

  } else {
    u8g2.drawGlyph(40, 63, 0xB0);
  }

  if (lampState2 == HIGH) {
    u8g2.drawStr(55, 63, "-");

  } else {
    u8g2.drawGlyph(55, 63, 0xB0);
  }

  if (lampState3 == HIGH) {
    u8g2.drawStr(70, 63, "-");

  } else {
    u8g2.drawGlyph(70, 63, 0xB0);
  }

  if (lampState4 == HIGH) {
    u8g2.drawStr(85, 63, "-");

  } else {
    u8g2.drawGlyph(85, 63, 0xB0);
  }

  u8g2.sendBuffer();
}

void formatDateTime(DateTime now, char *buffer, size_t bufferSize) {
  int hour12 = now.hour() % 12;
  if (hour12 == 0) {
    hour12 = 12;  // Special case for 12 AM/PM
  }
  snprintf(buffer, bufferSize, "%02d:%02d", hour12, now.minute());
}

void periodDisplay(DateTime now, char *buffer, size_t bufferSize) {
  int hour12 = now.hour() % 12;
  if (hour12 == 0) {
    hour12 == 0;
  }
  const char *period = (now.hour() < 12) ? "AM" : "PM";
  snprintf(buffer, bufferSize, "%s", period);
}


void dateDisplay(DateTime now, char *buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "%s,%02d-%02d-%04d",
           daysOfTheWeek[now.dayOfTheWeek()],
           now.day(), now.month(), now.year());
}

void manualRelayControl() {
  // Check if the timer is active
  bool timerActive = (count == 1 || count == 2);  // Assuming count 1 and 2 represent timer modes

  // Only allow manual control if the timer is not active
  if (!timerActive) {
    for (int i = 0; i < numberOfButtons; i++) {
      if (buttons[i]->pressed()) {
        digitalWrite(relayPin[i], HIGH);  // Turn on the corresponding relay
        // Update the corresponding lampState
        switch (i) {
          case 0: lampState1 = HIGH; break;
          case 1: lampState2 = HIGH; break;
          case 2: lampState3 = HIGH; break;
          case 3: lampState4 = HIGH; break;
        }
      }
      if (buttons[i]->released()) {
        digitalWrite(relayPin[i], LOW);  // Turn off the corresponding relay
        // Update the corresponding lampState
        switch (i) {
          case 0: lampState1 = LOW; break;
          case 1: lampState2 = LOW; break;
          case 2: lampState3 = LOW; break;
          case 3: lampState4 = LOW; break;
        }
      }
    }
  }
}


void saveLampOffTimeToEEPROM(int lampIndex) {
  int addr_hour = EEPROM_ADDR_LAMP1_OFF_HOUR + lampIndex * 8;
  int addr_minute = EEPROM_ADDR_LAMP1_OFF_MINUTE + lampIndex * 8;
  int addr_second = EEPROM_ADDR_LAMP1_OFF_SECOND + lampIndex * 8;
  int addr_period = EEPROM_ADDR_LAMP1_OFF_PERIOD + lampIndex * 8;

  EEPROM.write(addr_hour, time_h);
  EEPROM.write(addr_minute, time_m);
  EEPROM.write(addr_second, time_s);
  EEPROM.write(addr_period, isPM);

  EEPROM.commit();
}

void saveLampOnTimeToEEPROM(int lampIndex) {
  int addr_hour = EEPROM_ADDR_LAMP1_ON_HOUR + lampIndex * 8;
  int addr_minute = EEPROM_ADDR_LAMP1_ON_MINUTE + lampIndex * 8;
  int addr_second = EEPROM_ADDR_LAMP1_ON_SECOND + lampIndex * 8;
  int addr_period = EEPROM_ADDR_LAMP1_ON_PERIOD + lampIndex * 8;

  EEPROM.write(addr_hour, time_h);
  EEPROM.write(addr_minute, time_m);
  EEPROM.write(addr_second, time_s);
  EEPROM.write(addr_period, isPM);

  EEPROM.commit();
}

void loadLampOffTimeFromEEPROM(int lampIndex) {
  int addr_hour = EEPROM_ADDR_LAMP1_OFF_HOUR + lampIndex * 8;
  int addr_minute = EEPROM_ADDR_LAMP1_OFF_MINUTE + lampIndex * 8;
  int addr_second = EEPROM_ADDR_LAMP1_OFF_SECOND + lampIndex * 8;
  int addr_period = EEPROM_ADDR_LAMP1_OFF_PERIOD + lampIndex * 8;

  endHour = EEPROM.read(addr_hour);
  endMinute = EEPROM.read(addr_minute);
  endSecond = EEPROM.read(addr_second);
  endPeriod = EEPROM.read(addr_period);
}

void loadLampOnTimeFromEEPROM(int lampIndex) {
  int addr_hour = EEPROM_ADDR_LAMP1_ON_HOUR + lampIndex * 8;
  int addr_minute = EEPROM_ADDR_LAMP1_ON_MINUTE + lampIndex * 8;
  int addr_second = EEPROM_ADDR_LAMP1_ON_SECOND + lampIndex * 8;
  int addr_period = EEPROM_ADDR_LAMP1_ON_PERIOD + lampIndex * 8;

  startHour = EEPROM.read(addr_hour);
  startMinute = EEPROM.read(addr_minute);
  startSecond = EEPROM.read(addr_second);
  startPeriod = EEPROM.read(addr_period);
}
int convertTo24Hour(int hour, bool isPM) {
  if (isPM && hour != 12) {
    return hour + 12;
  } else if (!isPM && hour == 12) {
    return 0;
  } else {
    return hour;
  }
}
void timerFunction(DateTime now) {
  if (manualControlActive) return;  // Skip timer function if manual control is active

  for (int lampIndex = 0; lampIndex < 4; lampIndex++) {
    // Get schedule for the current lamp
    loadLampOnTimeFromEEPROM(lampIndex);
    loadLampOffTimeFromEEPROM(lampIndex);

    // Check if the current time falls within the schedule
    if (isTimeInRange(now, startHour, startMinute, startSecond, startPeriod, endHour, endMinute, endSecond, endPeriod)) {
      // Turn on the lamp
      digitalWrite(relayPin[lampIndex], LOW);
      lampState1 = LOW;
      lampState2 = LOW;
      lampState3 = LOW;
      lampState4 = LOW;

    } else {
      // Turn off the lamp
      digitalWrite(relayPin[lampIndex], HIGH);
      lampState1 = HIGH;
      lampState2 = HIGH;
      lampState3 = HIGH;
      lampState4 = HIGH;
    }
  }
}

// Function to check if the current time is within the specified range
bool isTimeInRange(DateTime now, int startHour, int startMinute, int startSecond, bool startPeriod, int endHour, int endMinute, int endSecond, bool endPeriod) {
  // Convert 12-hour format to 24-hour format
  int currentHour = now.hour();
  int currentMinute = now.minute();
  int currentSecond = now.second();

  int startHour24 = convertTo24Hour(startHour, startPeriod);
  int endHour24 = convertTo24Hour(endHour, endPeriod);

  // Convert times to total seconds since midnight
  int startTotalSeconds = startHour24 * 3600 + startMinute * 60 + startSecond;
  int endTotalSeconds = endHour24 * 3600 + endMinute * 60 + endSecond;
  int currentTotalSeconds = currentHour * 3600 + currentMinute * 60 + currentSecond;

  // Check if the current time is within the range
  if (startTotalSeconds <= endTotalSeconds) {
    // Case 1: Start and end are on the same day
    return (currentTotalSeconds >= startTotalSeconds && currentTotalSeconds <= endTotalSeconds);
  } else {
    // Case 2: End time is on the next day
    return (currentTotalSeconds >= startTotalSeconds || currentTotalSeconds <= endTotalSeconds);
  }
}



void timerSection() {
  u8g2.clearBuffer();

  char hhString[3];
  char mmString[3];
  char ssString[3];

  snprintf(hhString, sizeof(hhString), "%02d", time_h);
  snprintf(mmString, sizeof(mmString), "%02d", time_m);
  snprintf(ssString, sizeof(ssString), "%02d", time_s);

  u8g2.setFont(u8g_font_courB12);
  u8g2.drawStr(25, 15, "Schedule");
  u8g2.setFont(u8g_font_5x8r);

  char timeString[20];
  snprintf(timeString, sizeof(timeString), "%s : %s : %s %s", hhString, mmString, ssString, isPM ? "PM" : "AM");

  u8g2.drawStr(25, 30, timeString);
  u8g2.drawStr(50, 45, lamp_menu_items[lampSelected]);

  u8g2.drawStr(47, 60, "ON");
  u8g2.drawStr(60, 60, "/");
  u8g2.drawStr(67, 60, "OFF");




  int boxX = 29;  // Adjust position based on selected component
  int boxY = 20;  // Adjust position based on selected component
  if (selectedComponent == 1) {
    boxX = 53;
    // Position for AM/PM
  }
  if (selectedComponent == 2) {
    boxX = 78;  // Position for AM/PM
  }

  if (selectedComponent == 3) {
    boxX = 93;  // Position for AM/PM
  }

  if (selectedComponent == 4) {
    boxX = 60;
    boxY = 35;
  }

  if (selectedComponent == 5) {
    boxX = 49;  // Position for AM/PM
    boxY = 52;
    if (backButton.pressed()) {
      // Print the current time
      Serial.print("Time on: ");
      Serial.print(time_h);
      Serial.print(":");
      Serial.print(time_m);
      Serial.print(":");
      Serial.print(time_s);
      saveLampOnTimeToEEPROM(lampSelected);
    }
  }

  if (selectedComponent == 6) {
    boxX = 72;  // Position for AM/PM
    boxY = 52;
    if (backButton.pressed()) {
      Serial.print("Time off: ");
      Serial.print(time_h);
      Serial.print(":");
      Serial.print(time_m);
      Serial.print(":");
      Serial.print(time_s);
      saveLampOffTimeToEEPROM(lampSelected);
    }
  }

  u8g2.setFont(u8g2_font_tiny_simon_tr);
  u8g2.drawGlyph(boxX, boxY, 0x0076);
  u8g2.sendBuffer();
}

void monitoringDisplay(DateTime now) {
  u8g2.clearBuffer();
  int x = 8;
  int y = 12 - scrollOffset;  // Apply scroll offset

  char timeString[20];
  char secondString[5];
  char periodString[6];

  // Format the current time and period
  formatDateTime(now, timeString, sizeof(timeString));
  periodDisplay(now, periodString, sizeof(periodString));
  sprintf(secondString, "%02d", now.second());

  // Display the current time
  u8g2.setFont(u8g_font_5x8r);
  u8g2.drawStr(8, y, timeString);
  u8g2.drawStr(33, y, ":");
  u8g2.drawStr(38, y, secondString);
  u8g2.drawStr(50, y, periodString);

  y += 20;  // Space between the time and the lamp times

  // Display the lamp on and off times
  for (int i = 0; i < 4; i++) {
    int addr_on_hour = EEPROM_ADDR_LAMP1_ON_HOUR + i * 8;
    int addr_on_minute = EEPROM_ADDR_LAMP1_ON_MINUTE + i * 8;
    int addr_on_second = EEPROM_ADDR_LAMP1_ON_SECOND + i * 8;
    int addr_on_period = EEPROM_ADDR_LAMP1_ON_PERIOD + i * 8;

    int addr_off_hour = EEPROM_ADDR_LAMP1_OFF_HOUR + i * 8;
    int addr_off_minute = EEPROM_ADDR_LAMP1_OFF_MINUTE + i * 8;
    int addr_off_second = EEPROM_ADDR_LAMP1_OFF_SECOND + i * 8;
    int addr_off_period = EEPROM_ADDR_LAMP1_OFF_PERIOD + i * 8;

    int onHour = EEPROM.read(addr_on_hour);
    int onMinute = EEPROM.read(addr_on_minute);
    int onSecond = EEPROM.read(addr_on_second);
    int onPeriod = EEPROM.read(addr_on_period);

    int offHour = EEPROM.read(addr_off_hour);
    int offMinute = EEPROM.read(addr_off_minute);
    int offSecond = EEPROM.read(addr_off_second);
    int offPeriod = EEPROM.read(addr_off_period);

    char onTimeString[30];
    char offTimeString[30];
    sprintf(onTimeString, "Lamp %d On: %02d:%02d:%02d %s", i + 1, onHour, onMinute, onSecond, onPeriod == 0 ? "AM" : "PM");
    sprintf(offTimeString, "Lamp %d Off: %02d:%02d:%02d %s", i + 1, offHour, offMinute, offSecond, offPeriod == 0 ? "AM" : "PM");

    u8g2.setFont(u8g_font_5x8r);
    u8g2.drawStr(x, y + (i * 20), onTimeString);        // Adjust y position for each lamp
    u8g2.drawStr(x, y + (i * 20) + 10, offTimeString);  // Adjust y position for each lamp
  }

  u8g2.sendBuffer();
}
int maxScrollOffset() {
  // Calculate the maximum scroll offset based on the number of items to display
  int totalItems = 4;      // Number of lamps
  int itemHeight = 30;     // Height of each item in pixels
  int visibleHeight = 64;  // Height of the display in pixels

  return max(0, (totalItems * itemHeight) - visibleHeight);
}

void handleScrollInput() {
  if (upButton.pressed()) {
    scrollOffset = max(0, scrollOffset - scrollStep);  // Scroll up
  }
  if (downButton.pressed()) {
    scrollOffset = min(scrollOffset + scrollStep, maxScrollOffset());  // Scroll down
  }
}


void Task1(void *pvParameters) {
  while (true) {
    // Format the float values to 2 decimal places
    String formattedCurrent = String(current_mA, 2);
    String formattedVoltage = String(loadvoltage, 2);
    String formattedPower = String(power_mW, 2);

    // Set a timeout for Firebase operations (e.g., 5 seconds)
    unsigned long startTime = millis();
    const unsigned long timeout = 5000;  // 5 seconds

    // Send data to Firebase
    bool success = true;
    success &= Firebase.RTDB.setString(&fbdo, "Sensor/current", formattedCurrent);
    success &= Firebase.RTDB.setString(&fbdo, "Sensor/voltage", formattedVoltage);
    success &= Firebase.RTDB.setString(&fbdo, "Sensor//power", formattedPower);

    // Check if all Firebase operations were successful
    if (success) {
      Serial.println("Firebase update successful.");

      // Print formatted values for confirmation
      Serial.print("Current: ");
      Serial.println(formattedCurrent);
      Serial.print("Voltage: ");
      Serial.println(formattedVoltage);
      Serial.print("Power: ");
      Serial.println(formattedPower);
    } else {
      unsigned long elapsedTime = millis() - startTime;
      if (elapsedTime >= timeout) {
        Serial.println("Firebase update timed out.");
        // Handle timeout error here (e.g., retry or perform alternative action)
      } else {
        Serial.println("Firebase update failed: " + fbdo.errorReason());
      }
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}


void sensorDisplay() {

  u8g2.clearBuffer();
  u8g2.setFont(u8g_font_5x8r);
  u8g2.drawBitmap(0, 0, 128 / 8, 64, epd_bitmap_border);
  u8g2.drawStr(13, 17, "Voltage");
  u8g2.drawStr(13, 40, "Current");
  u8g2.drawStr(85, 17, "Power");

  char voltageStr[10];
  char currentStr[10];
  char powerStr[10];

  dtostrf(loadvoltage, 5, 2, voltageStr);  // Format: 5 characters total with
  dtostrf(current_mA, 4, 4, currentStr);
  dtostrf(power_mW, 5, 2, powerStr);

  u8g2.drawStr(20, 27, voltageStr);  // Voltage
  u8g2.drawStr(20, 50, currentStr);  // Current
  u8g2.drawStr(85, 34, powerStr);    // Power

  u8g2.drawStr(59, 23, "V");  // Voltage
  u8g2.drawStr(59, 47, "A");  // Voltage
  u8g2.drawStr(95, 47, "W");  // Voltage
  u8g2.sendBuffer();
}

void sensorData() {

  // Read sensor data
  current_mA = ina219.getCurrent_mA() / 1000;
  power_mW = ina219.getPower_mW() / 1000;
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  if (current_mA < 0) {
    current_mA = 0.0;
  }
}


void connectWiFi() {
  Serial.println("Connecting to WiFi");

  // Check if already connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Already connected to WiFi.");
    wifiConnected = true;  // Set WiFi connected flag
    return;                // Skip the rest of the function
  }

  // Connect to WiFi if credentials are provided
  if (strlen(WIFI_SSID) > 0 && strlen(WIFI_PASSWORD) > 0) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    // Set WiFi connected flag
    wifiConnected = true;
  } else {
    Serial.println("WiFi credentials not provided. Skipping WiFi connection.");
  }
}

void initializeFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signUp OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void Task2(void *pvParameters) {
  while (true) {
    if (isPhone == true) {
      // Add any additional task functionality here
      // if()
      if (Firebase.RTDB.getString(&lampControl, "Control/Lamp/Lamp 1")) {
        String state = lampControl.stringData();
        if (state == "0") {
          digitalWrite(relayPin[0], HIGH);
          lampState1 = HIGH;
        } else if (state == "1") {
          digitalWrite(relayPin[0], LOW);
          lampState1 = LOW;
        }
      }

      if (Firebase.RTDB.getString(&lampControl, "Control/Lamp/Lamp 2")) {
        String state = lampControl.stringData();
        if (state == "0") {
          digitalWrite(relayPin[1], HIGH);
          lampState2 = HIGH;
        } else if (state == "1") {
          digitalWrite(relayPin[1], LOW);
          lampState2 = LOW;
        }
      }

      if (Firebase.RTDB.getString(&lampControl, "Control/Lamp/Lamp 3")) {
        String state = lampControl.stringData();
        if (state == "0") {
          digitalWrite(relayPin[2], HIGH);
          lampState3 = HIGH;
        } else if (state == "1") {
          digitalWrite(relayPin[2], LOW);
          lampState3 = LOW;
        }
      }

      if (Firebase.RTDB.getString(&lampControl, "Control/Lamp/Lamp 4")) {
        String state = lampControl.stringData();
        if (state == "0") {
          digitalWrite(relayPin[3], HIGH);
          lampState4 = HIGH;
        } else if (state == "1") {
          digitalWrite(relayPin[3], LOW);
          lampState4 = LOW;
        }
      }
    }

    if (Firebase.RTDB.getInt(&modeControl, "Control/isPhone")) {
      int state = modeControl.intData();  // Assuming intData() is the correct method to retrieve integer data
      if (state == 0) {
        isPhone = true;
      } else if (state == 1) {
        Serial.println("IsNotPhoneControl");  // Changed from "IsNotPhoneControl" to "IsNotPhoneControl"
        isPhone = false;
      }
    }

    if (Firebase.RTDB.getInt(&modeControl, "Control/Settings")) {
      int state = modeControl.intData();  // Assuming intData() is the correct method to retrieve integer data

      // Update controlSwitch based on the retrieved state
      if (state == 0) {
        controlSwitch = 0;
      } else if (state == 1) {
        controlSwitch = 1;  // Changed from "IsNotPhoneControl" to setting controlSwitch to 1
      }
    }


    if (Firebase.RTDB.getInt(&modeControl, "Control/InsertTime")) {
      int state = modeControl.intData();  // Assuming intData() is the correct method to retrieve integer data
      // Update controlSwitch based on the retrieved state
      if (state == 1) {
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot1/on/hour")) {
          int hour1 = modeControl.intData();
          int addr_hour = EEPROM_ADDR_LAMP1_ON_HOUR + 0 * 8;
          Serial.println(hour1);
          EEPROM.write(addr_hour, hour1);
          EEPROM.commit();
        }
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot1/on/minute")) {
          int min1 = modeControl.intData();
          int addr_minute = EEPROM_ADDR_LAMP1_ON_MINUTE + 0 * 8;
          Serial.println(min1);
          EEPROM.write(addr_minute, min1);
          EEPROM.commit();
        }
        bool success = Firebase.RTDB.getBool(&modeControl, "timer/time/slot1/on/period");
        if (success) {
          bool per = modeControl.boolData();
          int addr_period = EEPROM_ADDR_LAMP1_ON_PERIOD + 0 * 8;
          Serial.println(per ? "true" : "false");
          if (per == 1) {
            value = false;
          } else {
            value = true;
          }
          EEPROM.write(addr_period, value);
          EEPROM.commit();
        }

        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot1/off/hour")) {
          int hour1 = modeControl.intData();
          int addr_hour = EEPROM_ADDR_LAMP1_OFF_HOUR + 0 * 8;
          Serial.println(hour1);
          EEPROM.write(addr_hour, hour1);
          EEPROM.commit();
        }
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot1/off/minute")) {
          int min1 = modeControl.intData();
          int addr_minute = EEPROM_ADDR_LAMP1_OFF_MINUTE + 0 * 8;
          Serial.println(min1);
          EEPROM.write(addr_minute, min1);
          EEPROM.commit();
        }
        bool success1 = Firebase.RTDB.getBool(&modeControl, "timer/time/slot1/off/period");
        if (success1) {
          bool per = modeControl.boolData();
          int addr_period = EEPROM_ADDR_LAMP1_OFF_PERIOD + 0 * 8;
          Serial.println(per ? "true" : "false");
          if (per == 1) {
            value = false;
          } else {
            value = true;
          }
          EEPROM.write(addr_period, value);
          EEPROM.commit();
        }
        // ??-------------------------------------------------------------------------------------
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot2/on/hour")) {
          int hour1 = modeControl.intData();
          int addr_hour = EEPROM_ADDR_LAMP1_ON_HOUR + 1 * 8;
          Serial.println(hour1);
          EEPROM.write(addr_hour, hour1);
          EEPROM.commit();
        }
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot2/on/minute")) {
          int min1 = modeControl.intData();
          int addr_minute = EEPROM_ADDR_LAMP1_ON_MINUTE + 1 * 8;
          Serial.println(min1);
          EEPROM.write(addr_minute, min1);
          EEPROM.commit();
        }
        bool success3 = Firebase.RTDB.getBool(&modeControl, "timer/time/slot2/on/period");
        if (success3) {
          bool per = modeControl.boolData();
          int addr_period = EEPROM_ADDR_LAMP1_ON_PERIOD + 1 * 8;
          Serial.println(per ? "true" : "false");
          if (per == 1) {
            value = false;
          } else {
            value = true;
          }
          EEPROM.write(addr_period, value);
          EEPROM.commit();
        }

        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot2/off/hour")) {
          int hour2 = modeControl.intData();
          int addr_hour = EEPROM_ADDR_LAMP1_OFF_HOUR + 1 * 8;
          Serial.println(hour2);
          EEPROM.write(addr_hour, hour2);
          EEPROM.commit();
        }
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot2/off/minute")) {
          int min2 = modeControl.intData();
          int addr_minute = EEPROM_ADDR_LAMP1_OFF_MINUTE + 1 * 8;
          Serial.println(min2);
          EEPROM.write(addr_minute, min2);
          EEPROM.commit();
        }
        bool success4 = Firebase.RTDB.getBool(&modeControl, "timer/time/slot2/off/period");
        if (success4) {
          bool per = modeControl.boolData();
          int addr_period = EEPROM_ADDR_LAMP1_OFF_PERIOD + 1 * 8;
          Serial.println(per ? "true" : "false");
          if (per == 1) {
            value = false;
          } else {
            value = true;
          }
          EEPROM.write(addr_period, value);
          EEPROM.commit();
        }

        //------------------------------------------------------------------------------------------------------------------------
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot3/on/hour")) {
          int hour1 = modeControl.intData();
          int addr_hour = EEPROM_ADDR_LAMP1_ON_HOUR + 2 * 8;
          Serial.println(hour1);
          EEPROM.write(addr_hour, hour1);
          EEPROM.commit();
        }
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot3/on/minute")) {
          int min1 = modeControl.intData();
          int addr_minute = EEPROM_ADDR_LAMP1_ON_MINUTE + 2 * 8;
          Serial.println(min1);
          EEPROM.write(addr_minute, min1);
          EEPROM.commit();
        }
        bool success5 = Firebase.RTDB.getBool(&modeControl, "timer/time/slot3/on/period");
        if (success5) {
          bool per = modeControl.boolData();
          int addr_period = EEPROM_ADDR_LAMP1_ON_PERIOD + 2 * 8;
          Serial.println(per ? "true" : "false");
          if (per == 1) {
            value = true;
          } else {
            value = false;
          }
          EEPROM.write(addr_period, value);
          EEPROM.commit();
        }

        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot3/off/hour")) {
          int hour2 = modeControl.intData();
          int addr_hour = EEPROM_ADDR_LAMP1_OFF_HOUR + 2 * 8;
          Serial.println(hour2);
          EEPROM.write(addr_hour, hour2);
          EEPROM.commit();
        }
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot3/off/minute")) {
          int min2 = modeControl.intData();
          int addr_minute = EEPROM_ADDR_LAMP1_OFF_MINUTE + 2 * 8;
          Serial.println(min2);
          EEPROM.write(addr_minute, min2);
          EEPROM.commit();
        }
        bool success6 = Firebase.RTDB.getBool(&modeControl, "timer/time/slot3/off/period");
        if (success6) {
          bool per = modeControl.boolData();
          int addr_period = EEPROM_ADDR_LAMP1_OFF_PERIOD + 2 * 8;
          Serial.println(per ? "true" : "false");
          if (per == 1) {
            value = true;
          } else {
            value = false;
          }
          EEPROM.write(addr_period, value);
          EEPROM.commit();
        }

        //--------------------------------------------------------------------------------------------------------------------------------------------

        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot4/on/hour")) {
          int hour1 = modeControl.intData();
          int addr_hour = EEPROM_ADDR_LAMP1_ON_HOUR + 3 * 8;
          Serial.println(hour1);
          EEPROM.write(addr_hour, hour1);
          EEPROM.commit();
        }
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot4/on/minute")) {
          int min1 = modeControl.intData();
          int addr_minute = EEPROM_ADDR_LAMP1_ON_MINUTE + 3 * 8;
          Serial.println(min1);
          EEPROM.write(addr_minute, min1);
          EEPROM.commit();
        }
        bool success7 = Firebase.RTDB.getBool(&modeControl, "timer/time/slot4/on/period");
        if (success7) {
          bool per = modeControl.boolData();
          int addr_period = EEPROM_ADDR_LAMP1_ON_PERIOD + 3 * 8;
          Serial.println(per ? "true" : "false");
          if (per == 1) {
            value = true;
          } else {
            value = false;
          }
          EEPROM.write(addr_period, value);
          EEPROM.commit();
        }

        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot4/off/hour")) {
          int hour2 = modeControl.intData();
          int addr_hour = EEPROM_ADDR_LAMP1_OFF_HOUR + 3 * 8;
          Serial.println(hour2);
          EEPROM.write(addr_hour, hour2);
          EEPROM.commit();
        }
        if (Firebase.RTDB.getInt(&modeControl, "timer/time/slot4/off/minute")) {
          int min2 = modeControl.intData();
          int addr_minute = EEPROM_ADDR_LAMP1_OFF_MINUTE + 3 * 8;
          Serial.println(min2);
          EEPROM.write(addr_minute, min2);
          EEPROM.commit();
        }
        bool success8 = Firebase.RTDB.getBool(&modeControl, "timer/time/slot4/off/period");
        if (success8) {
          bool per = modeControl.boolData();
          int addr_period = EEPROM_ADDR_LAMP1_OFF_PERIOD + 3 * 8;
          Serial.println(per ? "true" : "false");
          if (per == 1) {
            value = true;
          } else {
            value = false;
          }
          EEPROM.write(addr_period, value);
          EEPROM.commit();
        }
      }
    }
    Serial.println("Task 2 is running");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
