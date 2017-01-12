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

/** Structure for store meta information about enum in reflection */
// TODO: Wait mechanism implementation to work with enums in reflection
struct EnumMeta
{
    enum Modes
    {
        EM_FLAGS = 1 << 1,
        EM_NOCAST = 1 << 2
    };

    template <typename T>
    inline static DAVA::Meta<EnumMeta> Create(DAVA::int32 mode = 0)
    {
        auto castFn = [](DAVA::int32 value)
        {
            return DAVA::Any(static_cast<T>(value));
        };
        return DAVA::Meta<EnumMeta>(EnumMeta(GlobalEnumMap<T>::Instance(), castFn, mode));
    }

    inline const EnumMap* GetEnumMap() const
    {
        return map;
    }

    inline DAVA::Any Cast(DAVA::int32 value) const
    {
        return ((mode & EM_NOCAST) != 0) ? DAVA::Any(value) : cast(value);
    }

    inline bool IsFlags() const
    {
        return (mode & EM_FLAGS) != 0;
    }

private:
    typedef DAVA::Any (*CastFn)(DAVA::int32);

    EnumMeta(const EnumMap* v, CastFn c, DAVA::int32 mode)
        : map(v)
        , cast(c)
        , mode(mode)
    {
    }

    const EnumMap* const map;
    CastFn const cast;
    const DAVA::int32 mode;
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
