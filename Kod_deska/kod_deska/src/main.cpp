#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SH110X.h> // https://github.com/adafruit/Adafruit_SH110X
#include <Adafruit_Sensor.h> // https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h> // https://github.com/adafruit/Adafruit_BME280_Library
#include <LD2450.h>
#include "FS.h"
#include "SD.h"
#include <WiFi.h>
#include "time.h"
#include <EasyButton.h>
#include "tajnosti.h"

#define I2C_SDA 33
#define I2C_SCL 32

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME280_ADDRESS (0x77)
#define i2c_Address 0x3c
Adafruit_SH1106G displej = Adafruit_SH1106G(128, 64, &Wire, -1); // Nastavi displej
Adafruit_BME280 bme;                                             // I2C
LD2450 radar;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;     
const int daylightOffset_sec = 3600; 

const int LED_tep_vysoka = 22;
const int LED_tep_nizka = 13;
const int LED_vlhokost_vysoka = 27;
const int LED_pohyb = 26;
const int LED_data_ulozena = 25;

bool pohyb = false;
byte pocet_osob = 0;
float teplota = 0.0;
float tlak = 0.0;
float vlhkost = 0.0;
int nadmorskavyska = 0;

EasyButton tlacitko1(39);
EasyButton tlacitko2(34);
EasyButton tlacitko3(35);
bool tlacitko2_stav = false;
byte strana_menu = 0;

void readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Cteni souboru: %s\n", path);

  fs::File file = fs.open(path);
  if (!file)
  {
    Serial.println("Nepodarilo se otevrit soubor pro cteni");
    return;
  }

  Serial.print("Cteni ze souboru: ");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Zapis do souboru: %s\n", path);

  fs::File file = fs.open(path, "w");
  if (!file)
  {
    Serial.println("Nepodarilo se otevrit soubor pro zapis");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Soubor zapsan");
  }
  else
  {
    Serial.println("Zapis do souboru selhal");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Pripisovani k souboru: %s\n", path);

  fs::File file = fs.open(path, "a");
  if (!file)
  {
    Serial.println("Nepodarilo se otevrit soubor pro pripisovani");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Pripisani uspesne");
  }
  else
  {
    Serial.println("Pripisovani selhalo");
  }
  file.close();
}

void uloz_data(const char *path, String data)
{
  File file = SD.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Nepodarilo se otevrit soubor pro zapisovani");
    return;
  }
  if (file.println(data))
  {
    Serial.println("Data ulozena: " + data);
  }
  else
  {
    Serial.println("Zapis dat selhal");
  }
  file.close();
}

void cti_tlacitka()
{
  tlacitko1.read();
  tlacitko2.read();
  tlacitko3.read();

  if (tlacitko2.wasPressed())
  {
    tlacitko2_stav = !tlacitko2_stav;
  }
  if (tlacitko2_stav)
  {
    if (tlacitko3.wasPressed())
    {
      if (strana_menu <= 0)
      {
        strana_menu = 0;
      }
      else
      {
        strana_menu--;
      }
    }
    if (tlacitko1.wasPressed())
    {
      strana_menu++;
      if (strana_menu >= 1)
      {
        strana_menu = 1;
      }
    }
  }
}

void zapis_data_do_souboru(unsigned long aktualni_ms)
{
  static unsigned long minuly_cas_zapisu = 0;
  const unsigned long interval_zapisu = 1000; // ms
  if (aktualni_ms - minuly_cas_zapisu >= interval_zapisu)
  {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Nepodarilo se ziskat cas");
      return;
    }
    String cas = String((timeinfo.tm_year) + 1900) + "-" +
                 String((timeinfo.tm_mon) + 1) + "-" +
                 String(timeinfo.tm_mday) + " " +
                 String(timeinfo.tm_hour) + ":" +
                 String(timeinfo.tm_min) + ":" +
                 String(timeinfo.tm_sec);

    String data = cas + "," +
                  String(teplota) + "," +
                  String(tlak) + "," +
                  String(vlhkost) + "," +
                  String(nadmorskavyska) + "," +
                  String(pohyb ? "1" : "0") + "," +
                  String(pocet_osob);

    uloz_data("/data.csv", data);
    minuly_cas_zapisu = aktualni_ms;
  }
}

void cti_senzory(unsigned long aktualni_ms)
{
  static unsigned long minuly_cas_cteni_radaru = 0;
  static unsigned long minuly_cas_cteni_bme = 0;
  const unsigned long interval_cteni_radaru = 1000; // ms
  const unsigned long interval_cteni_bme = 60000;   // ms
  if (aktualni_ms - minuly_cas_cteni_radaru >= interval_cteni_radaru)
  {
    radar.read();
    const LD2450::RadarTarget target0 = radar.getTarget(0);
    const LD2450::RadarTarget target1 = radar.getTarget(1);
    const LD2450::RadarTarget target2 = radar.getTarget(2);
    pohyb = target0.valid;
    pocet_osob = target0.valid + target1.valid + target2.valid;
    minuly_cas_cteni_radaru = aktualni_ms;
  }
  else
  {
    return;
  }
  if (aktualni_ms - minuly_cas_cteni_bme >= interval_cteni_bme)
  {
    bme.takeForcedMeasurement(); // Nutno provest pred ctenim
    teplota = bme.readTemperature();
    tlak = bme.readPressure() / 100.0F;
    vlhkost = bme.readHumidity();
    nadmorskavyska = int(bme.readAltitude(SEALEVELPRESSURE_HPA));
    minuly_cas_cteni_bme = aktualni_ms;
  }
  else
  {
    return;
  }
}

void vypis_na_displej(unsigned long aktualni_ms)
{
  static unsigned long minuly_cas_aktualizace_displeje = 0;
  const unsigned long interval_aktualizace_displeje = 1000; // ms
  if (aktualni_ms - minuly_cas_aktualizace_displeje >= interval_aktualizace_displeje)
  {
    Serial.print(tlacitko2_stav ? "DISPLEJ ON " : "DISPLEJ OFF ");
    Serial.println("STRANA MENU: " + String(strana_menu) + " ");
    if (tlacitko2_stav)
    {
      displej.clearDisplay();
      displej.setCursor(0, 0);
      switch (strana_menu)
      {
      case 0:
      {
        String s_teplota = "Telpota: " + String(teplota) + "C";
        String s_tlak = "Tlak: " + String(tlak) + "hPa";
        String s_vlhkost = "Vlhkost: " + String(vlhkost) + "%";
        String s_nadmorskaVyska = "Nadmorska vyska: " + String(nadmorskavyska) + "m";
        displej.println(s_teplota);
        displej.println(s_tlak);
        displej.println(s_vlhkost);
        displej.println(s_nadmorskaVyska);
        break;
      }
      case 1:
      {
        String s_pohyb = "Pohyb: " + String(pohyb ? "ANO" : "NE");
        String s_pocetOsob = "Pocet osob: " + String(pocet_osob);
        displej.println(s_pohyb);
        displej.println(s_pocetOsob);
        break;
      }
      default:
        break;
      }
    } else {
      displej.clearDisplay();
    }
    displej.display();
    minuly_cas_aktualizace_displeje = aktualni_ms;
  }
}
void aktualizaceLedek(unsigned long aktualni_ms)
{
  static unsigned long minuly_cas_aktualizace_led = 0;
  const unsigned long interval_aktualizace_led = 1000; // ms
  if (aktualni_ms - minuly_cas_aktualizace_led >= interval_aktualizace_led)
  {
    if (teplota > 25.0)
    {
      digitalWrite(LED_tep_vysoka, HIGH);
      digitalWrite(LED_tep_nizka, LOW);
    }
    else if (teplota < 18.0)
    {
      digitalWrite(LED_tep_vysoka, LOW);
      digitalWrite(LED_tep_nizka, HIGH);
    }
    else
    {
      digitalWrite(LED_tep_vysoka, LOW);
      digitalWrite(LED_tep_nizka, LOW);
    }

    if (vlhkost > 60.0)
    {
      digitalWrite(LED_vlhokost_vysoka, HIGH);
    }
    else
    {
      digitalWrite(LED_vlhokost_vysoka, LOW);
    }

    if (pohyb)
    {
      digitalWrite(LED_pohyb, HIGH);
    }
    else
    {
      digitalWrite(LED_pohyb, LOW);
    }
    minuly_cas_aktualizace_led = aktualni_ms;
  }

  // LED pro uspesne ulozeni dat muze blikat na kazdy zapis
}
void setup()
{
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  while (!Serial);  // Cekam na spusteni seriove linky
  Serial2.begin(256000, SERIAL_8N1, 16, 15); // RX, TX
  radar.begin(Serial2, false);
  displej.begin(i2c_Address, true);
  displej.clearDisplay();
  displej.setTextColor(SH110X_WHITE);
  displej.setTextSize(1);
  if (!bme.begin(BME280_ADDRESS))
  {
    Serial.println("Nepodarilo se najit platny senzor BME280, zkontrolujte zapojeni, adresu nebo ID!");
    Serial.print("ID senzoru bylo: 0x");
    Serial.println(bme.sensorID(), 16);
    Serial.print("        ID 0xFF pravdepodobne znamena spatnou adresu, BMP 180 nebo BMP 085\n");
    Serial.print("   ID 0x56-0x58 odpovida BMP 280,\n");
    Serial.print("        ID 0x60 odpovida BME 280.\n");
    Serial.print("        ID 0x61 odpovida BME 680.\n");
    while (1)
      delay(10);
  }
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // teplota
                  Adafruit_BME280::SAMPLING_X1, // tlak
                  Adafruit_BME280::SAMPLING_X1, // vlhkost
                  Adafruit_BME280::FILTER_OFF);

  Serial.print("Pripojovani k WiFi");
  WiFi.begin(SSID, HESLO);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Pripojeno k WiFi!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (!SD.begin(5))
  {
    Serial.println("Pripojeni SD karty selhalo!");
    return;
  }

  File file = SD.open("/data.csv");
  if (!file)
  {
    uloz_data("/data.csv", "cas,teplota,tlak,vlhkost,nadmorska_vyska,pohyb,pocet_osob");
  }
  file.close();
  tlacitko1.begin();
  tlacitko2.begin();
  tlacitko3.begin();
  pinMode(LED_tep_vysoka, OUTPUT);
  pinMode(LED_tep_nizka, OUTPUT); 
  pinMode(LED_vlhokost_vysoka, OUTPUT);
  pinMode(LED_pohyb, OUTPUT);
  pinMode(LED_data_ulozena, OUTPUT);
  digitalWrite(LED_tep_vysoka, LOW);
  digitalWrite(LED_tep_nizka, LOW);
  digitalWrite(LED_vlhokost_vysoka, LOW);
  digitalWrite(LED_pohyb, LOW);
  digitalWrite(LED_data_ulozena, LOW);
}

void loop()
{
  unsigned long aktualni_ms = millis();
  cti_tlacitka();
  cti_senzory(aktualni_ms);
  vypis_na_displej(aktualni_ms);
  zapis_data_do_souboru(aktualni_ms);
  aktualizaceLedek(aktualni_ms);
}