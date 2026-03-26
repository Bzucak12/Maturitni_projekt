#pragma once

#include "knihovny.h"

void synchronizace_dat(unsigned long ms);
void synchronizuj_chybejici_zaznamy_teploty(unsigned long ms);
void synchronizuj_chybejici_zaznamy_pohybu(unsigned long ms);

bool ulozPrvniNeuspesnyUpload(const String &cas);
String nactiPrvniNeuspesnyUpload();
void smazPrvniNeuspesnyUpload();

bool ulozPosledniUspesnyUpload(const char* filename, const String &cas);
String nactiPosledniUspesnyUpload(const char* filename);