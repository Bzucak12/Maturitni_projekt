#pragma once
#include "knihovny.h"

void readFile(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void uloz_data(const char *cesta, const String data);
void zapis_data_senzoru_pohybu_do_souboru(unsigned long aktualni_ms);
void zapis_data_senzoru_teploty_do_souboru(unsigned long aktualni_ms);
