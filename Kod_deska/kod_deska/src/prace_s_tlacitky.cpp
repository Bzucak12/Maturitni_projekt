#include "prace_s_tlacitky.h"

void cti_tlacitka()
{
    tlacitko1.read();
    tlacitko2.read();
    tlacitko3.read();

    if (tlacitko2.wasPressed())
    {
        tlacitko2_stav = !tlacitko2_stav;
    }
    if (tlacitko2_stav)
    {
        if (tlacitko3.wasPressed())
        {
            if (strana_menu <= 0)
            {
                strana_menu = 0;
            }
            else
            {
                strana_menu--;
            }
        }
        if (tlacitko1.wasPressed())
        {
            strana_menu++;
            if (strana_menu >= 2)
            {
                strana_menu = 2;
            }
        }
    }
}