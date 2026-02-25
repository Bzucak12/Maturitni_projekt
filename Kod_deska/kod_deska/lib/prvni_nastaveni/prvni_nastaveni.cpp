#include "prvni_nastaveni.h"

void prvni_nastaveni()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.begin(115200);
    while (!Serial);                           // Cekam na spusteni seriove linky
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
    Serial.println(" PRIPOJENO!");

    configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
    delay(5000);
    ziskej_cas(); // Ziskani casu pro inicializaci
    if (!SD.begin(5))
    {
        Serial.println("Pripojeni SD karty selhalo!");
        return;
    }

    File soubor_pohyb = SD.open("/pohyb.csv");
    if (!soubor_pohyb)
    {
        uloz_data("/pohyb.csv", "cas,pohyb,pocet_osob");
    }
    soubor_pohyb.close();
    File soubor_teplota = SD.open("/teplota.csv");
    if (!soubor_teplota)
    {
        uloz_data("/teplota.csv", "cas,pohyb,pocet_osob");
    }
    soubor_teplota.close();
    cti_senzory(0); // Nacteni senzoru pro inicializaci
    prvni_mereni_bme = false;
    tlacitko1.begin();
    tlacitko2.begin();
    tlacitko3.begin();
    pinMode(LED_tep_vysoka, OUTPUT);
    pinMode(LED_tep_nizka, OUTPUT);
    pinMode(LED_vlhokost_vysoka, OUTPUT);
    pinMode(LED_pohyb, OUTPUT);
    pinMode(LED_wifi_pripojena, OUTPUT);
    digitalWrite(LED_tep_vysoka, LOW);
    digitalWrite(LED_tep_nizka, LOW);
    digitalWrite(LED_vlhokost_vysoka, LOW);
    digitalWrite(LED_pohyb, LOW);
    digitalWrite(LED_wifi_pripojena, LOW);
}