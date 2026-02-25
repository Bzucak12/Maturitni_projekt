#include "opakovani.h"

void opakovani()
{
    unsigned long aktualni_ms = millis();
    cti_tlacitka();
    cti_senzory(aktualni_ms);
    vypis_na_displej(aktualni_ms);
    zapis_data_senzoru_pohybu_do_souboru(aktualni_ms);
    zapis_data_senzoru_teploty_do_souboru(aktualni_ms);
    posli_data_senzoru_pohybu_na_server(aktualni_ms);
    posli_data_senzoru_teploty_na_server(aktualni_ms);
    aktualizace_ledek();
}