#include "QtTools/ProjectInformation/MaterialTemplatesInfo.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/FilePath.h"

namespace MaterialsTemplateInfoDetails
{
using namespace DAVA;

Vector<String> LoadMaterialQualities(FilePath fxPath)
{
    Vector<String> qualities;
    YamlParser* parser = YamlParser::Create(fxPath);
    if (parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if (rootNode)
        {
            const YamlNode* materialTemplateNode = rootNode->Get("MaterialTemplate");
            if (materialTemplateNode)
            {
                static const char* QUALITIES[] = { "LOW", "MEDIUM", "HIGH", "ULTRA_HIGH" };
                for (const char* quality : QUALITIES)
                {
                    const YamlNode* qualityNode = materialTemplateNode->Get(quality);
                    if (qualityNode)
                    {
                        qualities.push_back(quality);
                    }
                }
            }
        }
    }

    return qualities;
}
}

void MaterialTemplatesInfo::Load(const DAVA::FilePath& pathToAssignable)
{
    templates.clear();

    if (DAVA::FileSystem::Instance()->Exists(pathToAssignable))
    {
        DAVA::ScopedPtr<DAVA::YamlParser> parser(DAVA::YamlParser::Create(pathToAssignable));
        DAVA::YamlNode* rootNode = parser->GetRootNode();

        if (nullptr != rootNode)
        {
            DAVA::FilePath materialsListDir = pathToAssignable.GetDirectory();

            for (DAVA::uint32 i = 0; i < rootNode->GetCount(); ++i)
            {
                const DAVA::YamlNode* templateNode = rootNode->Get(i);
                if (nullptr != templateNode)
                {
                    const DAVA::YamlNode* name = templateNode->Get("name");
                    const DAVA::YamlNode* path = templateNode->Get("path");

                    if (nullptr != name && nullptr != path &&
                        name->GetType() == DAVA::YamlNode::TYPE_STRING &&
                        path->GetType() == DAVA::YamlNode::TYPE_STRING)
                    {
                        const DAVA::FilePath templatePath = materialsListDir + path->AsString();
                        if (DAVA::FileSystem::Instance()->Exists(templatePath))
                        {
                            MaterialTemplateInfo info;

                            info.name = name->AsString().c_str();
                            info.path = templatePath.GetFrameworkPath().c_str();
                            info.qualities = MaterialsTemplateInfoDetails::LoadMaterialQualities(templatePath);

                            templates.push_back(info);
                        }
                    }
                }
            }
        }
    }
}
