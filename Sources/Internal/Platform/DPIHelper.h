#ifndef __DPIHELPER_H__
#define __DPIHELPER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

namespace DAVA
{
class DPIHelper
{
public:
    static uint32 GetScreenDPI();
    static float64 GetDpiScaleFactor(int32 screenId);
    static Size2i GetScreenSize();
};
};
#endif // __DPIHELPER_H__