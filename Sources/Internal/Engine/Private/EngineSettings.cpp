#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"

namespace DAVA
{
std::array<Any, EngineSettings::SETTING_COUNT> EngineSettings::settingDefault;
std::array<FastName, EngineSettings::SETTING_COUNT> EngineSettings::settingName;
std::array<FastName, EngineSettings::SETTING_VALUE_COUNT> EngineSettings::settingValueName;

DAVA_REFLECTION_IMPL(EngineSettings)
{
    auto& registrator = ReflectionRegistrator<EngineSettings>::Begin();

    //settings setup
    SetupSetting<SETTING_LANDSCAPE_RENDERMODE, eSettingValue>(registrator, "Landscape.RenderMode", LANDSCAPE_MORPHING, LANDSCAPE_NO_INSTANCING, LANDSCAPE_MORPHING);

    //setting enum values setup
    SetupSettingValue(LANDSCAPE_NO_INSTANCING, "Landscape.RenderMode.NoInstancing");
    SetupSettingValue(LANDSCAPE_INSTANCING, "Landscape.RenderMode.Instancing");
    SetupSettingValue(LANDSCAPE_MORPHING, "Landscape.RenderMode.Morphing");

    registrator.End();
}

EngineSettings::SettingRange::SettingRange(const Any& _min, const Any& _max)
    : min(_min)
    , max(_max)
{
}

EngineSettings::EngineSettings()
{
    ReflectedType::Get<EngineSettings>(); //ensure that settings was setup

    Reset();
}

void EngineSettings::Reset()
{
    for (int32 i = 0; i < int32(SETTING_COUNT); ++i)
        setting[i] = settingDefault[i];
}

bool EngineSettings::Load(const FilePath& filepath)
{
#ifdef __DAVAENGINE_COREV2__
    if (Engine::Instance()->GetContext()->fileSystem->Exists(filepath))
#else
    if (FileSystem::Instance()->Exists(filepath))
#endif
    {
        ScopedPtr<YamlParser> parser(YamlParser::Create(filepath));
        YamlNode* rootNode = parser->GetRootNode();

        if (rootNode)
        {
            for (uint32 i = 0; i < SETTING_COUNT; ++i)
            {
                const FastName& settingName = EngineSettings::GetSettingName(eSetting(i));
                const YamlNode* settingNode = rootNode->Get(settingName.c_str());
                if (settingNode && settingNode->GetType() == YamlNode::TYPE_STRING)
                {
                    if (setting[i].GetType() == Type::Instance<bool>())
                        setting[i] = settingNode->AsBool();
                    else if (setting[i].GetType() == Type::Instance<int32>())
                        setting[i] = settingNode->AsInt32();
                    else if (setting[i].GetType() == Type::Instance<float32>())
                        setting[i] = settingNode->AsFloat();
                    else if (setting[i].GetType() == Type::Instance<String>())
                        setting[i] = settingNode->AsString();
                    else if (setting[i].GetType() == Type::Instance<eSettingValue>())
                        setting[i] = GetSettingValueByName(FastName(settingNode->AsString().c_str()));
                }
            }

            return true;
        }
    }

    return false;
}

const FastName& EngineSettings::GetSettingName(eSetting setting)
{
    DVASSERT(setting < SETTING_COUNT);
    return settingName[setting];
}

const FastName& EngineSettings::GetSettingValueName(eSettingValue value)
{
    DVASSERT(value < SETTING_VALUE_COUNT);
    return settingValueName[value];
}

EngineSettings::eSettingValue EngineSettings::GetSettingValueByName(const FastName& name)
{
    uint32 i = 0;
    for (; i < SETTING_VALUE_COUNT; ++i)
    {
        if (settingValueName[i] == name)
            break;
    }

    return EngineSettings::eSettingValue(i);
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

} // ns DAVA
