#include "prace_se_soubory.h"

void cti_soubor(fs::FS &fs, const char *path)
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

void zapis_soubor(fs::FS &fs, const char *path, const char *message)
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

void pridej_do_souboru(fs::FS &fs, const char *path, const char *message)
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

void zapis_data_senzoru_pohybu_do_souboru(unsigned long aktualni_ms)
{
    static unsigned long minuly_cas_zapisu = 0;
    const unsigned long interval_zapisu = 1000;
    if (aktualni_ms - minuly_cas_zapisu >= interval_zapisu)
    {
        minuly_cas_zapisu = aktualni_ms;
        if (pocet_osob != minuly_pocet_osob) {
            String cas = ziskej_cas();
            String data = cas + "," +
                        String(pohyb ? "1" : "0") + "," +
                        String(pocet_osob);

            uloz_data("/pohyb.csv", data);
        }
    }
}

void zapis_data_senzoru_teploty_do_souboru(unsigned long aktualni_ms)
{
    static unsigned long minuly_cas_zapisu = 0;
    const unsigned long interval_zapisu = 60000; 
    if (aktualni_ms - minuly_cas_zapisu >= interval_zapisu)
    {
        String cas = ziskej_cas();
        String data = cas + "," +
                      String(teplota) + "," +
                      String(tlak) + "," +
                      String(vlhkost) + "," +
                      String(nadmorskavyska);

        uloz_data("/teplota.csv", data);
        minuly_cas_zapisu = aktualni_ms;
    }
}