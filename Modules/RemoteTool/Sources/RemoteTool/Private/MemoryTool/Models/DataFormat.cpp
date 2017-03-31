#include "RemoteTool/Private/MemoryTool/Models/DataFormat.h"

#include <Base/TemplateHelpers.h>

using namespace DAVA;

String FormatNumberWithDigitGroups(uint32 value)
{
    char buf[32];
    int n = Snprintf(buf, COUNT_OF(buf), "%u", value);
    int g = n / 3 - ((n % 3) == 0);

    int from = n - 1;
    int to = from + g;
    buf[to + 1] = '\0';
    while (g > 0)
    {
        for (int j = 0; j < 3; ++j)
        {
            buf[to] = buf[from];
            to -= 1;
            from -= 1;
        }
        buf[to] = ' ';
        to -= 1;
        g -= 1;
    }
    return String(buf);
}
