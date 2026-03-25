# pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SH110X.h> // https://github.com/adafruit/Adafruit_SH110X
#include <Fonts/FreeSansBold9pt7b.h>
#include <Adafruit_Sensor.h> // https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h> // https://github.com/adafruit/Adafruit_BME280_Library
#include <LD2450.h>
#include "FS.h"
#include "SD.h"
#include <WiFi.h>
#include "time.h"
#include <EasyButton.h>
#include "tajnosti.h"
#include <HTTPClient.h>
#include "promene.h"
#include "cas.h"
#include "prace_se_soubory.h"
#include "prace_se_serverem.h"
#include "prace_s_tlacitky.h"
#include "prace_se_senzory.h"
#include "prace_s_displejem.h"
#include "prace_s_ledkami.h"
#include "synchronizace_dat.h"