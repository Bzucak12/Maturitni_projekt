#pragma once

#include "knihovny.h"

const byte I2C_SDA = 33;
const byte I2C_SCL = 32;

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME280_ADDRESS (0x77)
#define i2c_Address 0x3c
Adafruit_SH1106G displej = Adafruit_SH1106G(128, 64, &Wire, -1); // Nastavi displej
Adafruit_BME280 bme;                                             // I2C
LD2450 radar;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const int LED_tep_vysoka = 25;
const int LED_tep_nizka = 26;
const int LED_vlhokost_vysoka = 27;
const int LED_pohyb = 13;
const int LED_wifi_pripojena = 22;

bool pohyb = false;
byte pocet_osob = 0;
double teplota = 0.0;
double tlak = 0.0;
double vlhkost = 0.0;
int nadmorskavyska = 0;

EasyButton tlacitko1(39);
EasyButton tlacitko2(34);
EasyButton tlacitko3(35);
bool tlacitko2_stav = false;
byte strana_menu = 0;

String URL_POHYB = "http://192.168.1.4/Maturitni_projekt_web/uloz_data_pohybu.php";
String URL_TEPLOTA = "http://192.168.1.4/Maturitni_projekt_web/uloz_data_teploty.php";

bool prvni_mereni_bme = true;