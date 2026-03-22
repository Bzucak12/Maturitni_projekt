#include "synchronizace_dat.h"

void synchronizace_dat(unsigned long ms)
{
  synchronizuj_chybejici_zaznamy_teploty(ms);
  synchronizuj_chybejici_zaznamy_pohybu(ms);
}

void synchronizuj_chybejici_zaznamy_teploty(unsigned long ms)
{
  // Porovna zaznamy na SD karte s databazi a chybejici nahraje na server.
  const int interval_synchronizace = 300000; // 300 sekund
  static unsigned long minuly_cas_synchronizace = 0;

  if (ms - minuly_cas_synchronizace >= interval_synchronizace)
  {
    minuly_cas_synchronizace = ms;

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi neni pripojena. Nelze synchronizovat data.");
      return;
    }

    // Nacte vsechny casove znacky z databaze.
    Serial.println("\n=== Spoustim synchronizaci teploty ===");
    HTTPClient http;
    http.begin(URL_CASU_TEPLOTY);
    int kod_odpovedi = http.GET();

    if (kod_odpovedi != 200)
    {
      Serial.println("Chyba pri nacitani casovych znacek: " + String(kod_odpovedi));
      http.end();
      return;
    }

    String odpoved = http.getString();
    http.end();

    Serial.println("Casove znacky z databaze prijaty");

    // Vytahne pole casovych znacek z JSON (jednoduche parsovani).
    // Format: {"uspech":true,"pocet":N,"casy":["cas1","cas2",...]} 
    int zacatek_klice = odpoved.indexOf("\"casy\"");
    if (zacatek_klice == -1)
    {
      Serial.println("Neplatny format odpovedi - nebyla nalezena sec 'casy'");
      return;
    }

    // Najde [ po "casy"
    int zacatek_pole = odpoved.indexOf("[", zacatek_klice);
    int konec_pole = odpoved.lastIndexOf("]");
    
    if (zacatek_pole == -1 || konec_pole == -1 || konec_pole <= zacatek_pole)
    {
      Serial.println("Neplatny format odpovedi - pole neni ve spravnem formatu");
      return;
    }

    String pole_casovych_znacek = odpoved.substring(zacatek_pole + 1, konec_pole);
    
    // Nacte vsechny zaznamy ze SD karty.
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
          // Ziska casovou znacku (prvni pole).
          int index_carky = aktualni_radek.indexOf(",");
          String sd_casova_znacka = aktualni_radek.substring(0, index_carky);
          
          // Overi, zda tato casova znacka existuje v databazi.
          if (pole_casovych_znacek.indexOf("\"" + sd_casova_znacka + "\"") == -1)
          {
            // Chybejici zaznam nalezen - nahraje se na server.
            Serial.print("Nalezen chybejici zaznam: ");
            Serial.println(sd_casova_znacka);
            
            // Rozparsuje CSV zaznam.
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
            pole[pocet_poli] = aktualni_pole; // Posledni pole.
            
            // Odesle data na server.
            HTTPClient nahravaci_url;
            nahravaci_url.begin(URL_CHYBEJICI_TEPLOTY);
            nahravaci_url.addHeader("Content-Type", "application/x-www-form-urlencoded");
            
            String data = "cas_ulozeni=" + pole[0] +
                            "&teplota=" + pole[1] +
                            "&vlhkost=" + pole[2] +
                            "&tlak=" + pole[3] +
                            "&nadmorska_vyska=" + pole[4];
            
            int odpoved_nahrani = nahravaci_url.POST(data);
            if (odpoved_nahrani == 200)
            {
              Serial.println("Zaznam byl uspesne nahran");
              pocet_chybejicich++;
            }
            else
            {
              Serial.println("Nahrani zaznamu selhalo");
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
    
    Serial.print("=== Synchronizace teploty dokoncena: ");
    Serial.print(pocet_chybejicich);
    Serial.println(" zaznamu nahrano ===\n");
  }
}

void synchronizuj_chybejici_zaznamy_pohybu(unsigned long ms)
{
  // Porovna zaznamy na SD karte s databazi a chybejici nahraje na server.
  const int interval_synchronizace = 300000; // 300 sekund
  static unsigned long minuly_cas_synchronizace = 0;

  if (ms - minuly_cas_synchronizace >= interval_synchronizace)
  {
    minuly_cas_synchronizace = ms;

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi neni pripojena. Nelze synchronizovat data.");
      return;
    }

    // Nacte vsechny casove znacky z databaze.
    Serial.println("\n=== Spoustim synchronizaci pohybu ===");
    HTTPClient http;
    http.begin(URL_CASU_POHYBU);
    int kod_odpovedi = http.GET();

    if (kod_odpovedi != 200)
    {
      Serial.println("Chyba pri nacitani casovych znacek: " + String(kod_odpovedi));
      http.end();
      return;
    }

    String odpoved = http.getString();
    http.end();

    Serial.println("Casove znacky z databaze prijaty");

    // Vytahne pole casovych znacek z JSON (jednoduche parsovani).
    // Format: {"uspech":true,"pocet":N,"casy":["cas1","cas2",...]} 
    int zacatek_klice = odpoved.indexOf("\"casy\"");
    if (zacatek_klice == -1)
    {
      Serial.println("Neplatny format odpovedi - nebyla nalezena sec 'casy'");
      return;
    }

    // Najde [ po "casy"
    int zacatek_pole = odpoved.indexOf("[", zacatek_klice);
    int konec_pole = odpoved.lastIndexOf("]");
    
    if (zacatek_pole == -1 || konec_pole == -1 || konec_pole <= zacatek_pole)
    {
      Serial.println("Neplatny format odpovedi - pole neni ve spravnem formatu");
      return;
    }

    String pole_casovych_znacek = odpoved.substring(zacatek_pole + 1, konec_pole);
    
    // Nacte vsechny zaznamy ze SD karty.
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
          // Ziska casovou znacku (prvni pole).
          int index_carky = aktualni_radek.indexOf(",");
          String sd_casova_znacka = aktualni_radek.substring(0, index_carky);
          
          // Overi, zda tato casova znacka existuje v databazi.
          if (pole_casovych_znacek.indexOf("\"" + sd_casova_znacka + "\"") == -1)
          {
            // Chybejici zaznam nalezen - nahraje se na server.
            Serial.print("Nalezen chybejici zaznam: ");
            Serial.println(sd_casova_znacka);
            
            // Rozparsuje CSV zaznam.
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
            pole[pocet_poli] = aktualni_pole; // Posledni pole.
            
            // Odesle data na server.
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
            }
            else
            {
              Serial.println("Nahrani zaznamu selhalo");
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
    
    Serial.print("=== Synchronizace pohybu dokoncena: ");
    Serial.print(pocet_chybejicich);
    Serial.println(" zaznamu nahrano ===\n");
  }
}