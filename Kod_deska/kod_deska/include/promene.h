#pragma once

#include "knihovny.h"

#define I2C_SDA 33
#define I2C_SCL 32

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME280_ADDRESS (0x77)
#define i2c_Address 0x3c
inline Adafruit_SH1106G displej = Adafruit_SH1106G(128, 64, &Wire, -1); 
inline Adafruit_BME280 bme;                                             
inline LD2450 radar;

inline const int LED_tep_vysoka = 22;
inline const int LED_tep_nizka = 13;
inline const int LED_vlhokost_vysoka = 27;
inline const int LED_pohyb = 26;
inline const int LED_wifi_pripojena = 25;

inline const double horni_mez_teploty = 25.0;
inline const double spodni_mez_teploty = 19.0;
inline const double horni_mez_vlhkosti = 60.0;

inline bool pohyb = false;
inline bool minuly_pohyb = false;
inline byte pocet_osob = 0;
inline double teplota = 0.0;
inline double tlak = 0.0;
inline double vlhkost = 0.0;
inline int nadmorskavyska = 0;

inline EasyButton tlacitko1(39);
inline EasyButton tlacitko2(34);
inline EasyButton tlacitko3(35);
inline bool tlacitko2_stav = false;
inline byte strana_menu = 0;

inline String URL_POHYB = "http://antl.tomascmol.net/uloz_data_pohybu.php";
inline String URL_TEPLOTA = "http://antl.tomascmol.net/uloz_data_teploty.php";