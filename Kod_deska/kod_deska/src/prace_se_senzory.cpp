#include "prace_se_senzory.h"

void cti_senzory(unsigned long aktualni_ms)
{
    static unsigned long minuly_cas_cteni_radaru = 0;
    static unsigned long minuly_cas_cteni_bme = 0;
    const unsigned long interval_cteni_radaru = 1000; 
    const unsigned long interval_cteni_bme = 60000;   
    if (aktualni_ms - minuly_cas_cteni_radaru >= interval_cteni_radaru)
    {
        radar.read();
        const LD2450::RadarTarget osoba0 = radar.getTarget(0);
        const LD2450::RadarTarget osoba1 = radar.getTarget(1);
        const LD2450::RadarTarget osoba2 = radar.getTarget(2);
        pohyb = osoba0.valid;
        minuly_pocet_osob = pocet_osob;
        pocet_osob = osoba0.valid + osoba1.valid + osoba2.valid;
        minuly_cas_cteni_radaru = aktualni_ms;
    }
    else
    {
        return;
    }
    if (aktualni_ms - minuly_cas_cteni_bme >= interval_cteni_bme)
    {
        if (kompenzace_teploty < maximalni_kompenzace_teploty) {
            kompenzace_teploty += krok_kompenzace_teploty;
        }
        bme.takeForcedMeasurement();
        teplota = bme.readTemperature() - kompenzace_teploty;
        tlak = bme.readPressure() / 100.0F;
        vlhkost = bme.readHumidity();
        nadmorskavyska = int(bme.readAltitude(SEALEVELPRESSURE_HPA));
        minuly_cas_cteni_bme = aktualni_ms;
    }
    else
    {
        return;
    }
}

void prvni_cteni_teploty() {
    bme.takeForcedMeasurement();
    teplota = bme.readTemperature() - kompenzace_teploty;
    tlak = bme.readPressure() / 100.0F;
    vlhkost = bme.readHumidity();
    nadmorskavyska = int(bme.readAltitude(SEALEVELPRESSURE_HPA));
}