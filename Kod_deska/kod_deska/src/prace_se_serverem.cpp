#include "prace_se_serverem.h"

void posli_data_senzoru_pohybu_na_server(unsigned long ms)
{
    const int interval_poslani = 1000;
    static unsigned long minuly_cas_poslani = 0;

    if (ms - minuly_cas_poslani >= interval_poslani)
    {
        minuly_cas_poslani = ms;
        if (pocet_osob != minuly_pocet_osob) {
            if (WiFi.status() == WL_CONNECTED)
            {
                HTTPClient http;
                http.begin(URL_POHYB);
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");

                String cas = ziskej_cas();
                String data = "cas_ulozeni=" + cas +
                            "&pohyb=" + String(int(pohyb)) +
                            "&pocet_osob=" + String(pocet_osob);

                int http_kod_odpovedi = http.POST(data);

                if (http_kod_odpovedi > 0)
                {
                    String odpoved = http.getString();
                    Serial.println("Odpoved serveru: " + odpoved);
                }
                else
                {
                    Serial.println("Chyba pri odeslani POST: " + String(http_kod_odpovedi));
                }

                http.end();
            }
            else
            {
                Serial.println("WiFi neni pripojena. Nelze poslat data.");
            }
        }
    }
}

void posli_data_senzoru_teploty_na_server(unsigned long ms)
{
    const int interval_poslani = 60000;
    static unsigned long minuly_cas_poslani = 0;

    if (ms - minuly_cas_poslani >= interval_poslani)
    {
        if (teplota == 0.0 && tlak == 0.0 && vlhkost == 0.0 && nadmorskavyska == 0)
        {
            return;
        }
        minuly_cas_poslani = ms;

        if (WiFi.status() == WL_CONNECTED)
        {
            HTTPClient http;
            http.begin(URL_TEPLOTA);
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");

            String cas = ziskej_cas();
            String data = "cas_ulozeni=" + cas +
                          "&teplota=" + String(teplota) +
                          "&tlak=" + String(tlak) +
                          "&vlhkost=" + String(vlhkost) +
                          "&nadmorska_vyska=" + String(nadmorskavyska);

            int http_kod_odpovedi = http.POST(data);

            if (http_kod_odpovedi > 0)
            {
                String odpoved = http.getString();
                Serial.println("Odpoved serveru: " + odpoved);
            }
            else
            {
                Serial.println("Chyba pri odeslani POST: " + String(http_kod_odpovedi));
            }

            http.end();
        }
        else
        {
            Serial.println("WiFi neni pripojena. Nelze poslat data.");
        }
    }
}