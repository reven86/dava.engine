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
    DAVA_REFLECTION(EngineSettings)
    {
        auto& registrator = ReflectionRegistrator<EngineSettings>::Begin();
        AddReflectionField<SETTING_LANDSCAPE_INSTANCING, bool>(registrator, "Landscape.Instancing", true);
        AddReflectionField<SETTING_LANDSCAPE_MORPHING, bool>(registrator, "Landscape.Morphing", true);
        AddReflectionField<SETTING_TEST, eSettingTestEnum>(registrator, "TestSetting", TEST_ENUM_1);
        registrator.End();
    }

public:
    enum eSetting : uint32
    {
        SETTING_LANDSCAPE_INSTANCING = 0,
        SETTING_LANDSCAPE_MORPHING,
        SETTING_TEST,

        //don't forget add new enum values to reflection block
        SETTING_COUNT
    };

    enum eSettingTestEnum
    {
        TEST_ENUM_1 = 0,
        TEST_ENUM_2,

        TEST_ENUM_COUNT
    };

    EngineSettings();

    void Reset();
    bool Load(const FilePath& filepath);

    template <eSetting ID>
    const Any& GetSetting() const;

    template <eSetting ID>
    void SetSetting(const Any& value);

    static const FastName& GetSettingName(eSetting setting);
    static eSetting GetSettingByName(const FastName& name);

    Signal<eSetting> settingChanged;

protected:
    template <eSetting ID, typename T>
    static void AddReflectionField(ReflectionRegistrator<EngineSettings>& registrator, const char* name, const T& defaultValue);

    template <eSetting ID, typename T>
    const T& GetSettingRefl() const;

    template <eSetting ID, typename T>
    void SetSettingRefl(const T& value);

    static std::array<Any, EngineSettings::SETTING_COUNT> settingDefault;
    static std::array<FastName, EngineSettings::SETTING_COUNT> settingName;

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
inline void EngineSettings::AddReflectionField(ReflectionRegistrator<EngineSettings>& registrator, const char* name, const T& defaultValue)
{
    settingDefault[ID] = defaultValue;
    settingName[ID] = FastName(name);
    registrator.Field(GetSettingName(ID).c_str(), &EngineSettings::GetSettingRefl<ID, T>, &EngineSettings::SetSettingRefl<ID, T>);
}

} //ns DAVA
