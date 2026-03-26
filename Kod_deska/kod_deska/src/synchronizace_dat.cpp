#include "synchronizace_dat.h"

static const char* FIRST_FAILED_UPLOAD_FILENAME = "/first_failed_upload_timestamp.txt";

String nactiPrvniNeuspesnyUpload()
{
  if (!SD.exists(FIRST_FAILED_UPLOAD_FILENAME))
    return String("");

  File f = SD.open(FIRST_FAILED_UPLOAD_FILENAME, FILE_READ);
  if (!f)
    return String("");

  String cas = f.readStringUntil('\n');
  f.close();
  cas.trim();
  return cas;
}

bool ulozPrvniNeuspesnyUpload(const String &cas)
{
  // Zkontroluje, zda timestamp prvniho selhani uz existuje
  if (SD.exists(FIRST_FAILED_UPLOAD_FILENAME))
  {
    Serial.println("Timestamp prvniho neuspesneho uploadu uz existuje - ponecham puvodni!");
    return false; // Timestamp jiz existuje
  }

  File f = SD.open(FIRST_FAILED_UPLOAD_FILENAME, FILE_WRITE);
  if (!f)
    return false;

  f.println(cas);
  f.close();
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

void smazPrvniNeuspesnyUpload()
{
  if (SD.exists(FIRST_FAILED_UPLOAD_FILENAME))
    SD.remove(FIRST_FAILED_UPLOAD_FILENAME);
}

static const char* LAST_SUCCESS_UPLOAD_TEPLOTA_FILE = "/last_success_upload_teplota.txt";
static const char* LAST_SUCCESS_UPLOAD_POHYB_FILE = "/last_success_upload_pohyb.txt";

void synchronizace_dat(unsigned long ms)
{
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

  String prvniNeuspesnyCas = nactiPrvniNeuspesnyUpload();
  String posledniUspesnyCas = nactiPosledniUspesnyUpload(LAST_SUCCESS_UPLOAD_TEPLOTA_FILE);
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
            if (!ulozPosledniUspesnyUpload(LAST_SUCCESS_UPLOAD_TEPLOTA_FILE, sd_casova_znacka))
            {
              Serial.println("Chyba pri ukladani posledniho uspesneho uploadu teploty");
            }
          }
          else
          {
            Serial.println("Nahrani zaznamu selhalo");
            if (!SD.exists(FIRST_FAILED_UPLOAD_FILENAME))
            {
              if (ulozPrvniNeuspesnyUpload(sd_casova_znacka))
              {
                Serial.println("Ulozen timestamp prvniho neuspesneho uploadu: " + sd_casova_znacka);
              }
              else
              {
                Serial.println("Chyba pri ukladani timestampu prvniho neuspesneho uploadu");
              }
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
    smazPrvniNeuspesnyUpload();
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

  String prvniNeuspesnyCas = nactiPrvniNeuspesnyUpload();
  String posledniUspesnyCas = nactiPosledniUspesnyUpload(LAST_SUCCESS_UPLOAD_POHYB_FILE);
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
            if (!ulozPosledniUspesnyUpload(LAST_SUCCESS_UPLOAD_POHYB_FILE, sd_casova_znacka))
            {
              Serial.println("Chyba pri ukladani posledniho uspesneho uploadu pohybu");
            }
          }
          else
          {
            Serial.println("Nahrani zaznamu selhalo");
            if (!SD.exists(FIRST_FAILED_UPLOAD_FILENAME))
            {
              if (ulozPrvniNeuspesnyUpload(sd_casova_znacka))
              {
                Serial.println("Ulozen timestamp prvniho neuspesneho uploadu: " + sd_casova_znacka);
              }
              else
              {
                Serial.println("Chyba pri ukladani timestampu prvniho neuspesneho uploadu");
              }
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
    smazPrvniNeuspesnyUpload();
    Serial.println("Uspesna synchronizace pohybu po selhani - timestamp smazan.");
  }

  Serial.print("=== Synchronizace pohybu dokoncena: ");
  Serial.print(pocet_chybejicich);
  Serial.println(" zaznamu nahrano ===\n");
}