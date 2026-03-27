#include "synchronizace_dat.h"

bool potrebuje_synchronizaci = false;
bool potrebuje_synchronizaci_teplota = false;
bool potrebuje_synchronizaci_pohyb = false;

static void updatePotrebujeSynchronizaci()
{
  potrebuje_synchronizaci = potrebuje_synchronizaci_teplota || potrebuje_synchronizaci_pohyb;
}

String nactiPrvniNeuspesnyUpload(const char* filename)
{
  if (!SD.exists(filename))
    return String("");

  File f = SD.open(filename, FILE_READ);
  if (!f)
    return String("");

  String cas = f.readStringUntil('\n');
  f.close();
  cas.trim();
  if (cas.length() > 0)
  {
    if (strcmp(filename, SOUBOR_PRVNI_NEUSPESNY_UPLOAD_TEPLOTY) == 0)
      potrebuje_synchronizaci_teplota = true;
    else if (strcmp(filename, SOUBOR_PRVNI_NEUSPESNY_UPLOAD_POHYBU) == 0)
      potrebuje_synchronizaci_pohyb = true;

    updatePotrebujeSynchronizaci();
  }
  return cas;
}

bool ulozPrvniNeuspesnyUpload(const char* filename, const String &cas)
{
  // Zkontroluje, zda timestamp prvniho selhani uz existuje
  if (SD.exists(filename))
  {
    Serial.println("Timestamp prvniho neuspesneho uploadu uz existuje - ponecham puvodni!");
    return false; // Timestamp jiz existuje
  }

  File f = SD.open(filename, FILE_WRITE);
  if (!f)
    return false;

  f.println(cas);
  f.close();

  if (strcmp(filename, SOUBOR_PRVNI_NEUSPESNY_UPLOAD_TEPLOTY) == 0)
    potrebuje_synchronizaci_teplota = true;
  else if (strcmp(filename, SOUBOR_PRVNI_NEUSPESNY_UPLOAD_POHYBU) == 0)
    potrebuje_synchronizaci_pohyb = true;

  updatePotrebujeSynchronizaci();
  Serial.println("Ulozen prvni timestamp selhani: " + cas);
  return true;
}

String nactiPosledniUspesnyUpload(const char* filename)
{
  if (!SD.exists(filename))
    return String("");

  File f = SD.open(filename, FILE_READ);
  if (!f)
    return String("");

  String cas = f.readStringUntil('\n');
  f.close();
  cas.trim();
  return cas;
}

bool ulozPosledniUspesnyUpload(const char* filename, const String &cas)
{
  File f = SD.open(filename, FILE_WRITE);
  if (!f)
    return false;

  f.println(cas);
  f.close();
  return true;
}

void smazPrvniNeuspesnyUpload(const char* filename)
{
  if (SD.exists(filename))
    SD.remove(filename);

  if (strcmp(filename, SOUBOR_PRVNI_NEUSPESNY_UPLOAD_TEPLOTY) == 0)
    potrebuje_synchronizaci_teplota = false;
  else if (strcmp(filename, SOUBOR_PRVNI_NEUSPESNY_UPLOAD_POHYBU) == 0)
    potrebuje_synchronizaci_pohyb = false;

  updatePotrebujeSynchronizaci();
}

void synchronizace_dat(unsigned long ms)
{
  if (!potrebuje_synchronizaci)
    return;
  synchronizuj_chybejici_zaznamy_teploty(ms);
  synchronizuj_chybejici_zaznamy_pohybu(ms);
}

void synchronizuj_chybejici_zaznamy_teploty(unsigned long ms)
{
  const int interval_synchronizace = 300000; // 300 sekund
  static unsigned long minuly_cas_synchronizace = 0;

  if (ms - minuly_cas_synchronizace < interval_synchronizace)
    return;

  minuly_cas_synchronizace = ms;

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi neni pripojena. Nelze synchronizovat data.");
    return;
  }

  Serial.println("\n=== Spoustim synchronizaci teploty (local-only) ===");

  String prvniNeuspesnyCas = nactiPrvniNeuspesnyUpload(SOUBOR_PRVNI_NEUSPESNY_UPLOAD_TEPLOTY);
  String posledniUspesnyCas = nactiPosledniUspesnyUpload(SOUBOR_POSLEDNI_USPESNY_UPLOAD_TEPLOTY);
  String startCas = "";

  if (prvniNeuspesnyCas.length() > 0)
  {
    startCas = prvniNeuspesnyCas;
    Serial.println("Obnova od prvniho neuspesneho uploadu: " + startCas);
  }
  else if (posledniUspesnyCas.length() > 0)
  {
    startCas = posledniUspesnyCas;
    Serial.println("Obnova od posledniho uspesneho uploadu: " + startCas);
  }

  bool maSelhani = (prvniNeuspesnyCas.length() > 0);
  bool errorPriSynchronizaci = false;

  File soubor = SD.open("/teplota.csv");
  if (!soubor)
  {
    Serial.println("Soubor na SD karte nebyl nalezen");
    return;
  }

  int pocet_chybejicich = 0;
  String aktualni_radek = "";

  while (soubor.available())
  {
    char c = soubor.read();
    if (c == '\n')
    {
      if (aktualni_radek.length() > 0 && aktualni_radek.indexOf("cas_ulozeni") == -1)
      {
        int index_carky = aktualni_radek.indexOf(",");
        String sd_casova_znacka = aktualni_radek.substring(0, index_carky);

        if (!startCas.length() || sd_casova_znacka > startCas)
        {
          Serial.print("Zpracovavam lokalni zaznam: ");
          Serial.println(sd_casova_znacka);

          int pocet_poli = 0;
          String pole[5] = {"", "", "", "", ""};
          String aktualni_pole = "";

          for (int i = 0; i < aktualni_radek.length(); i++)
          {
            if (aktualni_radek[i] == ',')
            {
              pole[pocet_poli++] = aktualni_pole;
              aktualni_pole = "";
            }
            else
            {
              aktualni_pole += aktualni_radek[i];
            }
          }
          pole[pocet_poli] = aktualni_pole;

          HTTPClient nahravaci_url;
          nahravaci_url.begin(URL_CHYBEJICI_TEPLOTY);
          nahravaci_url.addHeader("Content-Type", "application/x-www-form-urlencoded");

          String data = "cas_ulozeni=" + pole[0] +
                        "&teplota=" + pole[1] +
                        "&tlak=" + pole[2] +
                        "&vlhkost=" + pole[3] +
                        "&nadmorska_vyska=" + pole[4];

          int odpoved_nahrani = nahravaci_url.POST(data);
          if (odpoved_nahrani == 200)
          {
            Serial.println("Zaznam byl uspesne nahran");
            pocet_chybejicich++;
            if (!ulozPosledniUspesnyUpload(SOUBOR_POSLEDNI_USPESNY_UPLOAD_TEPLOTY, sd_casova_znacka))
            {
              Serial.println("Chyba pri ukladani posledniho uspesneho uploadu teploty");
            }
          }
          else
          {
            Serial.println("Nahrani zaznamu selhalo");
            if (ulozPrvniNeuspesnyUpload(SOUBOR_PRVNI_NEUSPESNY_UPLOAD_TEPLOTY, sd_casova_znacka))
            {
              Serial.println("Ulozen timestamp prvniho neuspesneho uploadu teploty: " + sd_casova_znacka);
            }
            errorPriSynchronizaci = true;
          }

          nahravaci_url.end();
        }
      }
      aktualni_radek = "";
    }
    else
    {
      aktualni_radek += c;
    }
  }

  soubor.close();

  if (maSelhani && !errorPriSynchronizaci)
  {
    smazPrvniNeuspesnyUpload(SOUBOR_PRVNI_NEUSPESNY_UPLOAD_TEPLOTY);
    Serial.println("Uspesna synchronizace teploty po selhani - timestamp smazan.");
  }

  Serial.print("=== Synchronizace teploty dokoncena: ");
  Serial.print(pocet_chybejicich);
  Serial.println(" zaznamu nahrano ===\n");
}

void synchronizuj_chybejici_zaznamy_pohybu(unsigned long ms)
{
  const int interval_synchronizace = 300000; // 300 sekund
  static unsigned long minuly_cas_synchronizace = 0;

  if (ms - minuly_cas_synchronizace < interval_synchronizace)
    return;

  minuly_cas_synchronizace = ms;

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi neni pripojena. Nelze synchronizovat data.");
    return;
  }

  Serial.println("\n=== Spoustim synchronizaci pohybu (local-only) ===");

  String prvniNeuspesnyCas = nactiPrvniNeuspesnyUpload(SOUBOR_PRVNI_NEUSPESNY_UPLOAD_POHYBU);
  String posledniUspesnyCas = nactiPosledniUspesnyUpload(SOUBOR_POSLEDNI_USPESNY_UPLOAD_POHYBU);
  String startCas = "";

  if (prvniNeuspesnyCas.length() > 0)
  {
    startCas = prvniNeuspesnyCas;
    Serial.println("Obnova od prvniho neuspesneho uploadu: " + startCas);
  }
  else if (posledniUspesnyCas.length() > 0)
  {
    startCas = posledniUspesnyCas;
    Serial.println("Obnova od posledniho uspesneho uploadu: " + startCas);
  }

  bool maSelhani = (prvniNeuspesnyCas.length() > 0);
  bool errorPriSynchronizaci = false;

  File soubor = SD.open("/pohyb.csv");
  if (!soubor)
  {
    Serial.println("Soubor na SD karte nebyl nalezen");
    return;
  }

  int pocet_chybejicich = 0;
  String aktualni_radek = "";

  while (soubor.available())
  {
    char c = soubor.read();
    if (c == '\n')
    {
      if (aktualni_radek.length() > 0 && aktualni_radek.indexOf("cas_ulozeni") == -1)
      {
        int index_carky = aktualni_radek.indexOf(",");
        String sd_casova_znacka = aktualni_radek.substring(0, index_carky);

        if (!startCas.length() || sd_casova_znacka > startCas)
        {
          Serial.print("Zpracovavam lokalni zaznam: ");
          Serial.println(sd_casova_znacka);

          int pocet_poli = 0;
          String pole[3] = {"", "", ""};
          String aktualni_pole = "";

          for (int i = 0; i < aktualni_radek.length(); i++)
          {
            if (aktualni_radek[i] == ',')
            {
              pole[pocet_poli++] = aktualni_pole;
              aktualni_pole = "";
            }
            else
            {
              aktualni_pole += aktualni_radek[i];
            }
          }
          pole[pocet_poli] = aktualni_pole;

          HTTPClient nahravaci_url;
          nahravaci_url.begin(URL_CHYBEJICIHO_POHYBU);
          nahravaci_url.addHeader("Content-Type", "application/x-www-form-urlencoded");

          String data = "cas_ulozeni=" + pole[0] +
                        "&pohyb=" + pole[1] +
                        "&pocet_osob=" + pole[2];

          int odpoved_nahrani = nahravaci_url.POST(data);
          if (odpoved_nahrani == 200)
          {
            Serial.println("Zaznam byl uspesne nahran");
            pocet_chybejicich++;
            if (!ulozPosledniUspesnyUpload(SOUBOR_POSLEDNI_USPESNY_UPLOAD_POHYBU, sd_casova_znacka))
            {
              Serial.println("Chyba pri ukladani posledniho uspesneho uploadu pohybu");
            }
          }
          else
          {
            Serial.println("Nahrani zaznamu selhalo");
            if (ulozPrvniNeuspesnyUpload(SOUBOR_PRVNI_NEUSPESNY_UPLOAD_POHYBU, sd_casova_znacka))
            {
              Serial.println("Ulozen timestamp prvniho neuspesneho uploadu pohybu: " + sd_casova_znacka);
            }
            errorPriSynchronizaci = true;
          }

          nahravaci_url.end();
        }
      }
      aktualni_radek = "";
    }
    else
    {
      aktualni_radek += c;
    }
  }

  soubor.close();

  if (maSelhani && !errorPriSynchronizaci)
  {
    smazPrvniNeuspesnyUpload(SOUBOR_PRVNI_NEUSPESNY_UPLOAD_POHYBU);
    Serial.println("Uspesna synchronizace pohybu po selhani - timestamp smazan.");
  }

  Serial.print("=== Synchronizace pohybu dokoncena: ");
  Serial.print(pocet_chybejicich);
  Serial.println(" zaznamu nahrano ===\n");
}