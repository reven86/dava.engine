#include "Engine/EngineSettings.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"

namespace DAVA
{
std::array<Any, EngineSettings::SETTING_COUNT> EngineSettings::settingDefault;
std::array<FastName, EngineSettings::SETTING_COUNT> EngineSettings::settingName;

EngineSettings::EngineSettings()
{
    ReflectedType::Get<EngineSettings>(); //ensure that default values and names are initilized

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
                    else if (setting[i].GetType() == Type::Instance<uint32>())
                        setting[i] = settingNode->AsUInt32();
                    else if (setting[i].GetType() == Type::Instance<int64>())
                        setting[i] = settingNode->AsInt64();
                    else if (setting[i].GetType() == Type::Instance<uint64>())
                        setting[i] = settingNode->AsUInt64();
                    else if (setting[i].GetType() == Type::Instance<float32>())
                        setting[i] = settingNode->AsFloat();
                    else if (setting[i].GetType() == Type::Instance<FastName>())
                        setting[i] = settingNode->AsFastName();
                    else if (setting[i].GetType() == Type::Instance<String>())
                        setting[i] = settingNode->AsString();
                    else if (setting[i].GetType() == Type::Instance<WideString>())
                        setting[i] = settingNode->AsWString();
                    else if (setting[i].GetType() == Type::Instance<eSettingTestEnum>())
                        setting[i] = eSettingTestEnum(settingNode->AsInt32());
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

EngineSettings::eSetting EngineSettings::GetSettingByName(const FastName& name)
{
    uint32 i = 0;
    for (; i < SETTING_COUNT; i++)
    {
        if (name == settingName[i])
            break;
    }

    return eSetting(i);
}

} // ns DAVA
