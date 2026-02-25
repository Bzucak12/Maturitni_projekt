#include "cas.h"

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