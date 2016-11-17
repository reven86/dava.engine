#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Functional/Function.h"
#include "Functional/Signal.h"
#include "Reflection/Registrator.h"

#if !defined(__DAVAENGINE_COREV2__)
#include "Base/Singleton.h"
#endif

namespace DAVA
{
class FilePath;
#ifdef __DAVAENGINE_COREV2__
class EngineSettings
#else
class EngineSettings : public Singleton<EngineSettings>
#endif
{
    //TODO: move to .cpp after merge reflection forward declaration
    DAVA_REFLECTION(EngineSettings)
    {
#define SETUP_SETTING(eSetting, type, name, defvalue) SetupSetting<eSetting, type>(registrator, name, defvalue);
#define SETUP_SETTING_WITH_RANGE(eSetting, type, name, defvalue, minvalue, maxvalue) SetupSetting<eSetting, type>(registrator, name, defvalue, std::make_pair(minvalue, maxvalue));
#define SETUP_SETTING_VALUE(eSettingValue, name) settingValueName[eSettingValue] = FastName(name);
        auto& registrator = ReflectionRegistrator<EngineSettings>::Begin();

        SETUP_SETTING_WITH_RANGE(SETTING_LANDSCAPE_RENDERMODE, eSettingValue, "Landscape.RenderMode", LANDSCAPE_MORPHING, LANDSCAPE_NO_INSTANCING, LANDSCAPE_MORPHING);

        SETUP_SETTING_VALUE(LANDSCAPE_NO_INSTANCING, "Landscape.RenderMode.NoInstancing");
        SETUP_SETTING_VALUE(LANDSCAPE_INSTANCING, "Landscape.RenderMode.Instancing");
        SETUP_SETTING_VALUE(LANDSCAPE_MORPHING, "Landscape.RenderMode.Morphing");

        registrator.End();
#undef SETUP_SETTING
#undef SETUP_SETTING_WITH_RANGE
#undef SETUP_SETTING_VALUE
    }

public:
    enum eSetting : uint32
    {
        SETTING_LANDSCAPE_RENDERMODE = 0,

        //don't forget setup new enum values in reflection block
        SETTING_COUNT
    };

    enum eSettingValue : uint32
    {
        //'SETTING_LANDSCAPE_MODE'
        LANDSCAPE_NO_INSTANCING = 0,
        LANDSCAPE_INSTANCING,
        LANDSCAPE_MORPHING,

        //don't forget setup new enum values in reflection block
        SETTING_VALUE_COUNT
    };

    struct SettingRange
    {
        SettingRange(const Any& _min, const Any& _max)
            : min(_min)
            , max(_max)
        {
        }

        Any min;
        Any max;
    };

    EngineSettings();

    void Reset();
    bool Load(const FilePath& filepath);

    template <eSetting ID>
    const Any& GetSetting() const;

    template <eSetting ID>
    void SetSetting(const Any& value);

    static const FastName& GetSettingName(eSetting setting);
    static const FastName& GetSettingValueName(eSettingValue value);
    static eSettingValue GetSettingValueByName(const FastName& name);

    Signal<eSetting> settingChanged;

protected:
    template <eSetting ID, typename T>
    static void SetupSetting(ReflectionRegistrator<EngineSettings>& registrator, const char* name, const T& defaultValue, const std::pair<T, T>& range = std::make_pair(T(), T()));

    template <eSetting ID, typename T>
    const T& GetSettingRefl() const;

    template <eSetting ID, typename T>
    void SetSettingRefl(const T& value);

    static std::array<Any, SETTING_COUNT> settingDefault;
    static std::array<FastName, SETTING_COUNT> settingName;
    static std::array<FastName, SETTING_VALUE_COUNT> settingValueName;

    std::array<Any, SETTING_COUNT> setting;
};

template <EngineSettings::eSetting ID>
inline const Any& EngineSettings::GetSetting() const
{
    return setting[ID];
}

template <EngineSettings::eSetting ID>
inline void EngineSettings::SetSetting(const Any& value)
{
    DVASSERT(setting[ID].GetType() == value.GetType());
    if (setting[ID] != value)
    {
        setting[ID] = value;
        settingChanged.Emit(ID);
    }
}

template <EngineSettings::eSetting ID, typename T>
inline const T& EngineSettings::GetSettingRefl() const
{
    const Any& v = GetSetting<ID>();
    return v.Get<T>();
}

template <EngineSettings::eSetting ID, typename T>
inline void EngineSettings::SetSettingRefl(const T& value)
{
    SetSetting<ID>(value);
}

template <EngineSettings::eSetting ID, typename T>
inline void EngineSettings::SetupSetting(ReflectionRegistrator<EngineSettings>& registrator, const char* name, const T& defaultValue, const std::pair<T, T>& range)
{
    settingDefault[ID] = defaultValue;
    settingName[ID] = FastName(name);

    if (range != std::make_pair(T(), T()))
        registrator.Field(GetSettingName(ID).c_str(), &EngineSettings::GetSettingRefl<ID, T>, &EngineSettings::SetSettingRefl<ID, T>)[Meta<SettingRange>(Any(range.first), Any(range.second))];
    else
        registrator.Field(GetSettingName(ID).c_str(), &EngineSettings::GetSettingRefl<ID, T>, &EngineSettings::SetSettingRefl<ID, T>);
}

} //ns DAVA
