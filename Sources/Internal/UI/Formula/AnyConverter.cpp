#include "AnyConverter.h"

#include "Utils/StringFormat.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
String AnyConverter::ToString(const Any& val)
{
    if (val.CanGet<int32>())
    {
        return Format("%d", val.Get<int32>());
    }
    else if (val.CanGet<uint64>())
    {
        return Format("%ld", val.Get<uint64>());
    }
    else if (val.CanGet<int64>())
    {
        return Format("%ldL", val.Get<int64>());
    }
    else if (val.CanGet<uint16>())
    {
        return Format("%d", val.Get<int16>());
    }
    else if (val.CanGet<int16>())
    {
        return Format("%d", val.Get<int16>());
    }
    else if (val.CanGet<uint8>())
    {
        return Format("%d", val.Get<uint8>());
    }
    else if (val.CanGet<int8>())
    {
        return Format("%d", val.Get<int8>());
    }
    else if (val.CanGet<float>())
    {
        return Format("%f", val.Get<float>());
    }
    else if (val.CanGet<String>())
    {
        return val.Get<String>();
    }
    else if (val.CanGet<FilePath>())
    {
        return val.Get<FilePath>().GetFrameworkPath();
    }
    else if (val.CanGet<bool>())
    {
        return val.Get<bool>() ? "true" : "false";
    }

    return String("");
}
}
