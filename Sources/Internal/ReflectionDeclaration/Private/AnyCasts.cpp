#include "ReflectionDeclaration/Private/AnyCasts.h"

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"

#include "FileSystem/FilePath.h"

namespace DAVA
{
const char* StringToCharPointer(const Any& value)
{
    return value.Get<String>().c_str();
}

FastName StringToFastName(const Any& value)
{
    return FastName(value.Get<String>().c_str());
}

String CharPointerToString(const Any& value)
{
    return String(value.Get<const char*>());
}

FastName CharPointerToFastName(const Any& value)
{
    return FastName(value.Get<const char*>());
}

const char* FastNameToCharPointer(const Any& value)
{
    return value.Get<FastName>().c_str();
}

String FastNameToString(const Any& value)
{
    return String(value.Get<FastName>().c_str());
}

template <typename T>
String IntegralToString(const Any& value)
{
    return std::to_string(value.Get<T>());
}

String FilePathToString(const Any& value)
{
    return value.Get<FilePath>().GetAbsolutePathname();
}

FilePath StringToFilePath(const Any& value)
{
    return FilePath(value.Get<String>());
}

void RegisterAnyCasts()
{
    AnyCast<String, const char*>::Register(&StringToCharPointer);
    AnyCast<String, FastName>::Register(&StringToFastName);
    AnyCast<const char*, String>::Register(&CharPointerToString);
    AnyCast<const char*, FastName>::Register(&CharPointerToFastName);
    AnyCast<FastName, const char*>::Register(&FastNameToCharPointer);
    AnyCast<FastName, String>::Register(&FastNameToString);
    AnyCast<int32, size_t>::RegisterDefault();
    AnyCast<size_t, int32>::RegisterDefault();
    AnyCast<int32, String>::Register(&IntegralToString<int32>);
    AnyCast<size_t, String>::Register(&IntegralToString<size_t>);
    AnyCast<FilePath, String>::Register(&FilePathToString);
    AnyCast<String, FilePath>::Register(&StringToFilePath);
    AnyCast<float64, float32>::RegisterDefault();
    AnyCast<float32, float64>::RegisterDefault();
    AnyCast<uint32, int>::RegisterDefault();
    AnyCast<int, uint32>::RegisterDefault();
    AnyCast<uint16, int>::RegisterDefault();
    AnyCast<int, uint16>::RegisterDefault();
    AnyCast<uint8, int>::RegisterDefault();
    AnyCast<int, uint8>::RegisterDefault();
    AnyCast<int32, int>::RegisterDefault();
    AnyCast<int, int32>::RegisterDefault();
    AnyCast<int16, int>::RegisterDefault();
    AnyCast<int, int16>::RegisterDefault();
    AnyCast<int8, int>::RegisterDefault();
    AnyCast<int, int8>::RegisterDefault();
}

} // namespace DAVA
