#ifndef __DAVAENGINE_GLOBAL_ENUM_H__
#define __DAVAENGINE_GLOBAL_ENUM_H__

#include "Base/EnumMap.h"
#include "Base/Any.h"
#include "Reflection/ReflectedMeta.h"

template <typename T>
class GlobalEnumMap
{
public:
    explicit GlobalEnumMap();
    ~GlobalEnumMap();

    static const EnumMap* Instance();

protected:
    static void RegisterAll();
    static void Register(const int e, const char* s);
};

template <typename T>
const EnumMap* GlobalEnumMap<T>::Instance()
{
    static EnumMap enumMap;
    static bool initialized = false;

    if (!initialized)
    {
        initialized = true;
        RegisterAll();
    }

    return &enumMap;
}

template <typename T>
void GlobalEnumMap<T>::Register(const int e, const char* s)
{
    Instance()->Register(e, s);
}

/** Struct for store meta information about enum in reflection */
// TODO: Wait mechanism implementation to work with enums in reflection
struct EnumMeta
{
    template <typename T>
    static DAVA::Meta<EnumMeta> Create(bool autocast = true)
    {
        if (autocast)
        {
            auto cast = [](DAVA::int32 value) -> Any
            {
                return DAVA::Any(static_cast<T>(value));
            };
            return DAVA::Meta<EnumMeta>(EnumMeta(GlobalEnumMap<T>::Instance(), cast));
        }
        else
        {
            auto nocast = [](DAVA::int32 value) -> Any
            {
                return DAVA::Any(value);
            };
            return DAVA::Meta<EnumMeta>(EnumMeta(GlobalEnumMap<T>::Instance(), nocast));
        }
    }

    typedef DAVA::Any (*CastFn)(DAVA::int32);

    EnumMeta(const EnumMap* v, CastFn c)
        : value(v)
        , cast(c)
    {
    }

    const EnumMap* const value;
    CastFn const cast;
};

#define ENUM_DECLARE(eType) template <> void GlobalEnumMap<eType>::RegisterAll()
#define ENUM_ADD(eValue) Register(eValue, #eValue)
#define ENUM_ADD_DESCR(eValue, eDescr) Register(eValue, eDescr)

// Define:
//	ENUM_DECLARE(AnyEnumType)
//	{
//		ENUM_ADDS(AnyEnumType::Value1);
//		ENUM_ADD_DESCR(AnyEnumType::Value2, "Value2");
//	}
//
// Usage:
//  GlobalEnumMap::Instance<AnyEnumType>();

#endif // __DAVAENGINE_GLOBAL_ENUM_H__
