#include "knihovny.h"

void setup()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.begin(115200);
    while (!Serial);                                    
    Serial2.begin(256000, SERIAL_8N1, 16, 15);
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

    incializace_casu("CET-1CEST,M3.5.0,M10.5.0/3");
    delay(5000);
    prvni_cteni_teploty();
    ziskej_cas(); 
    if (!SD.begin(5))
    {
        Serial.println("Pripojeni SD karty selhalo!");
        return;
    }

    File soubor_pohyb = SD.open("/pohyb.csv");
    if (!soubor_pohyb)
    {
        uloz_data("/pohyb.csv", "cas_ulozeni,pohyb,pocet_osob");
    }
    soubor_pohyb.close();
    File soubor_teplota = SD.open("/teplota.csv");
    if (!soubor_teplota)
    {
        uloz_data("/teplota.csv", "cas_ulozeni,teplota,vlhkost,tlak,nadmorska_vyska");
    }
    soubor_teplota.close();
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

void loop()
{
    unsigned long aktualni_ms = millis();
    cti_tlacitka();
    cti_senzory(aktualni_ms);
    vypis_na_displej(aktualni_ms);
    zapis_data_senzoru_pohybu_do_souboru(aktualni_ms);
    zapis_data_senzoru_teploty_do_souboru(aktualni_ms);
    posli_data_senzoru_pohybu_na_server(aktualni_ms);
    posli_data_senzoru_teploty_na_server(aktualni_ms);
    aktualizaceLedek();
    synchronizace_dat(aktualni_ms);
}