#include "prace_s_displejem.h"

void vypis_na_displej(unsigned long aktualni_ms)
{
    static unsigned long minuly_cas_aktualizace_displeje = 0;
    const unsigned long interval_aktualizace_displeje = 1000; 
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
        }
        else
        {
            displej.clearDisplay();
        }
        displej.display();
        minuly_cas_aktualizace_displeje = aktualni_ms;
    }
}