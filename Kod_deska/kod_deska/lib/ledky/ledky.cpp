#include "ledky.h"

void aktualizace_ledek()
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

    if ((WiFi.status() == WL_CONNECTED))
    {
        digitalWrite(LED_wifi_pripojena, HIGH);
    }
    else
    {
        digitalWrite(LED_wifi_pripojena, LOW);
    }
}