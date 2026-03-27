#pragma once

#include "knihovny.h"

void synchronizace_dat(unsigned long ms);
void synchronizuj_chybejici_zaznamy_teploty(unsigned long ms);
void synchronizuj_chybejici_zaznamy_pohybu(unsigned long ms);

extern bool potrebuje_synchronizaci;
extern bool potrebuje_synchronizaci_teplota;
extern bool potrebuje_synchronizaci_pohyb;

bool ulozPrvniNeuspesnyUpload(const char* filename, const String &cas);
String nactiPrvniNeuspesnyUpload(const char* filename);
void smazPrvniNeuspesnyUpload(const char* filename);

bool ulozPosledniUspesnyUpload(const char* filename, const String &cas);
String nactiPosledniUspesnyUpload(const char* filename);