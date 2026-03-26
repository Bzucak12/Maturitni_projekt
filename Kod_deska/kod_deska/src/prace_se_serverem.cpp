#include "prace_se_serverem.h"
#include "synchronizace_dat.h"

static const char* FIRST_FAILED_UPLOAD_FILENAME = "/first_failed_upload_timestamp.txt";
static const char* LAST_SUCCESS_UPLOAD_POHYB_FILE = "/last_success_upload_pohyb.txt";
static const char* LAST_SUCCESS_UPLOAD_TEPLOTA_FILE = "/last_success_upload_teplota.txt";

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

                if (http_kod_odpovedi == 200)
                {
                    String odpoved = http.getString();
                    Serial.println("Odpoved serveru: " + odpoved);
                    if (!ulozPosledniUspesnyUpload(LAST_SUCCESS_UPLOAD_POHYB_FILE, cas))
                    {
                        Serial.println("Chyba pri ukladani posledniho uspesneho uploadu pohybu");
                    }
                }
                else
                {
                    Serial.println("Chyba pri odeslani POST: " + String(http_kod_odpovedi));
                    ulozPrvniNeuspesnyUpload(cas);
                }

                http.end();
            }
            else
            {
                Serial.println("WiFi neni pripojena. Nelze poslat data.");
                String cas = ziskej_cas();
                ulozPrvniNeuspesnyUpload(cas);
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

            if (http_kod_odpovedi == 200)
            {
                String odpoved = http.getString();
                Serial.println("Odpoved serveru: " + odpoved);
                if (!ulozPosledniUspesnyUpload(LAST_SUCCESS_UPLOAD_TEPLOTA_FILE, cas))
                {
                    Serial.println("Chyba pri ukladani posledniho uspesneho uploadu teploty");
                }
            }
            else
            {
                Serial.println("Chyba pri odeslani POST: " + String(http_kod_odpovedi));
                ulozPrvniNeuspesnyUpload(cas);
            }

            http.end();
        }
        else
        {
            Serial.println("WiFi neni pripojena. Nelze poslat data.");
            String cas = ziskej_cas();
            ulozPrvniNeuspesnyUpload(cas);
        }
    }
}