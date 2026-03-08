#pragma once

#include "knihovny.h"

void cti_soubor(fs::FS &fs, const char *path);
void zapis_soubor(fs::FS &fs, const char *path, const char *message);
void pridej_do_souboru(fs::FS &fs, const char *path, const char *message);
void uloz_data(const char *path, String data);
void zapis_data_senzoru_pohybu_do_souboru(unsigned long aktualni_ms);
void zapis_data_senzoru_teploty_do_souboru(unsigned long aktualni_ms);