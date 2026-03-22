#include "cas.h"

void nastav_casove_pasmo(String casove_pasmo){
  Serial.printf("  Nastavovani casoveho pasma na %s\n",casove_pasmo.c_str());
  setenv("TZ",casove_pasmo.c_str(),1);
  tzset();
}

void incializace_casu(String casove_pasmo){
  struct tm casove_informace;

  Serial.println("Nastavovani casu");
  configTime(0, 0, "pool.ntp.org");
  if(!getLocalTime(&casove_informace)){
    Serial.println("  Chyba pri ziskavani casu");
    return;
  }
  Serial.println("  Cas ziskan z NTP");
  nastav_casove_pasmo(casove_pasmo);
}

String ziskej_cas()
{
    struct tm casove_infomace;
    char buf[32] = {0};
    if (!getLocalTime(&casove_infomace))
    {
        return String("zadny-cas");
    }
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &casove_infomace);
    return String(buf);
}

String ziskej_hodiny_minuty_sekundy()
{
    struct tm casove_infomace;
    char buf[16] = {0};
    if (!getLocalTime(&casove_infomace))
    {
        return String("zadny-cas");
    }
    strftime(buf, sizeof(buf), "%H:%M:%S", &casove_infomace);
    return String(buf);
}

String ziskej_den_mesic_rok()
{
    struct tm casove_infomace;
    char buf[16] = {0};
    if (!getLocalTime(&casove_infomace))
    {
        return String("zadny-cas");
    }
    strftime(buf, sizeof(buf), "%d.%m.%Y", &casove_infomace);
    return String(buf);
}