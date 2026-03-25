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

inline const byte LED_tep_vysoka = 22;
inline const byte LED_tep_nizka = 13;
inline const byte LED_vlhokost_vysoka = 27;
inline const byte LED_pohyb = 26;
inline const byte LED_wifi_pripojena = 25;

inline const byte horni_mez_teploty = 25;
inline const byte spodni_mez_teploty = 19;
inline const byte horni_mez_vlhkosti = 60;

inline float kompenzace_teploty = 3.0;
inline const float krok_kompenzace_teploty = 0.1;
inline const float maximalni_kompenzace_teploty = 7.0;

inline bool pohyb = false;
inline byte pocet_osob = 0;
inline byte minuly_pocet_osob = 0;
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
inline String URL_CASU_TEPLOTY = "http://antl.tomascmol.net/ziskat_casy_mereni_teploty.php";
inline String URL_CHYBEJICI_TEPLOTY = "http://antl.tomascmol.net/odeslat_chybejici_data_teploty.php";
inline String URL_CASU_POHYBU = "http://antl.tomascmol.net/ziskat_casy_mereni_pohybu.php";
inline String URL_CHYBEJICIHO_POHYBU = "http://antl.tomascmol.net/odeslat_chybejici_data_pohybu.php";