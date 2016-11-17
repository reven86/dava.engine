#include "Engine/EngineSettings.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"

namespace DAVA
{
std::array<Any, EngineSettings::SETTING_COUNT> EngineSettings::settingDefault;
std::array<FastName, EngineSettings::SETTING_COUNT> EngineSettings::settingName;
std::array<FastName, EngineSettings::SETTING_VALUE_COUNT> EngineSettings::settingValueName;

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
    if (FileSystem::Instance()->Exists(filepath))
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

} // ns DAVA
