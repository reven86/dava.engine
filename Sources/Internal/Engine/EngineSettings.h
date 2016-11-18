#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Functional/Signal.h"
#include "Reflection/Registrator.h"

#if !defined(__DAVAENGINE_COREV2__)
#include "Base/Singleton.h"
#endif

namespace DAVA
{
class FilePath;

/**
    \ingroup engine
        Engine setting contain global setting if whole engine. List of settings is predefined.
        To add new setting you should to add new enum value and call `SetupSetting` in reflection-impl block.
        In setting-setup you should specify setting-type, setting string-name and default value.
        For now type of setting can be `bool`, `int32`, `float32`, `String` and `enum`.
        Also you can specify range of valid values, it will be added as meta-data in reflection.
        To access range just get meta-data from reflection by type `SettingRange`.

        If you want to use enum as setting-type - add necessary values to `eSettingValue`, 
        call `SetupSettingValue` in reflection-impl block to set string-name of value. 
        Then setup setting with type `eSettingValue` and specify range of valid enum values.

        It is possible to load setting from yaml-file. File represents as list of key-value pairs, for example:
            Landscape.RenderMode: Landscape.RenderMode.NoInstancing
        The key is a setting-name. Value depends of setting-type. If setting has enum-type, the key is string-name of `eSettingValue`
*/

#ifdef __DAVAENGINE_COREV2__
class EngineSettings
#else
class EngineSettings : public Singleton<EngineSettings>
#endif
{
    DAVA_REFLECTION(EngineSettings);

public:
    /**
        List of engine setting
    */
    enum eSetting : uint32
    {
        SETTING_LANDSCAPE_RENDERMODE = 0,

        //don't forget setup new enum values in reflection block
        SETTING_COUNT
    };

    /**
        List of setting-values for settings with enum-type
    */
    enum eSettingValue : uint32
    {
        //'SETTING_LANDSCAPE_MODE'
        LANDSCAPE_NO_INSTANCING = 0,
        LANDSCAPE_INSTANCING,
        LANDSCAPE_MORPHING,

        //don't forget setup new enum values in reflection block
        SETTING_VALUE_COUNT
    };

    /**
        Range of valid values for setting. Used to retrieving from reflection meta-data 
    */
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

    /**
        Reset setting to defaults
    */
    void Reset();

    /**
        Load setting from yaml-file
    */
    bool Load(const FilePath& filepath);

    /**
        Returns value of setting with `ID`
    */
    template <eSetting ID>
    const Any& GetSetting() const;

    /**
        Set value for setting with `ID`
    */
    template <eSetting ID>
    void SetSetting(const Any& value);

    /**
        Returns string-name of `setting`
    */
    static const FastName& GetSettingName(eSetting setting);

    /**
        Returns string-name of `value`
    */
    static const FastName& GetSettingValueName(eSettingValue value);

    /**
        Returns `eStringValue` by `name`
    */
    static eSettingValue GetSettingValueByName(const FastName& name);

    Signal<eSetting> settingChanged; //!< Emitted when any setting is changed. Setting-ID passes as param

protected:
    template <eSetting ID, typename T>
    static void SetupSetting(ReflectionRegistrator<EngineSettings>& registrator, const char* name, const T& defaultValue, const T& rangeStart = T(), const T& rangeEnd = T());
    static void SetupSettingValue(eSettingValue value, const char* name);

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
inline void EngineSettings::SetupSetting(ReflectionRegistrator<EngineSettings>& registrator, const char* name, const T& defaultValue, const T& rangeStart, const T& rangeEnd)
{
    DVASSERT(rangeStart <= rangeEnd);

    settingDefault[ID] = defaultValue;
    settingName[ID] = FastName(name);

    if (rangeStart != rangeEnd)
        registrator.Field(GetSettingName(ID).c_str(), &EngineSettings::GetSettingRefl<ID, T>, &EngineSettings::SetSettingRefl<ID, T>)[Meta<SettingRange>(Any(rangeStart), Any(rangeEnd))];
    else
        registrator.Field(GetSettingName(ID).c_str(), &EngineSettings::GetSettingRefl<ID, T>, &EngineSettings::SetSettingRefl<ID, T>);
}

inline void EngineSettings::SetupSettingValue(eSettingValue value, const char* name)
{
    DVASSERT(value < SETTING_VALUE_COUNT);
    settingValueName[value] = FastName(name);
}

} //ns DAVA
