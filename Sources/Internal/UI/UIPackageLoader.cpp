#include "UIPackageLoader.h"

#include "Base/ObjectFactory.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlHelpers.h"
#include "UI/UIPackage.h"


namespace DAVA
{

UIPackageLoader::UIPackageLoader(AbstractUIPackageBuilder *builder) : builder(builder)
{
}

UIPackageLoader::~UIPackageLoader()
{
    this->builder = NULL;
}
    
UIPackage *UIPackageLoader::LoadPackage(const FilePath &packagePath)
{
    if (!loadingQueue.empty())
    {
        DVASSERT(false);
        loadingQueue.clear();
    }

    if (!packagePath.Exists())
        return NULL;
    
    ScopedPtr<YamlParser> parser(YamlParser::Create(packagePath));
    
    YamlNode *rootNode = parser->GetRootNode();
    if (!rootNode)
        return NULL;
    
    const YamlNode *headerNode = rootNode->Get("Header");
    if (!headerNode)
        return NULL;
    
    const YamlNode *versionNode = headerNode->Get("version");
    if (versionNode == NULL || versionNode->GetType() != YamlNode::TYPE_STRING)
        return NULL;
    
    UIPackage *package = builder->BeginPackage(packagePath);

    const YamlNode *importedPackagesNode = rootNode->Get("ImportedPackages");
    if (importedPackagesNode)
    {
        for (int32 i = 0; i < (int32) importedPackagesNode->GetCount(); i++)
            builder->ProcessImportedPackage(importedPackagesNode->Get(i)->AsString(), this);
    }

    const YamlNode *controlsNode = rootNode->Get("Controls");
    if (controlsNode)
    {
        int32 count = (int32) controlsNode->GetCount();
        for (int32 i = 0; i < count; i++)
        {
            const YamlNode *node = controlsNode->Get(i);
            QueueItem item;
            item.name = node->Get("name")->AsString();
            item.node = node;
            item.status = STATUS_WAIT;
            loadingQueue.push_back(item);
        }
        
        for (int32 i = 0; i < count; i++)
        {
            if (loadingQueue[i].status == STATUS_WAIT)
            {
                loadingQueue[i].status = STATUS_LOADING;
                LoadControl(loadingQueue[i].node, true);
                loadingQueue[i].status = STATUS_LOADED;
            }
        }
        for (int32 i = 0; i < count; i++)
        {
            DVASSERT(loadingQueue[i].status == STATUS_LOADED);
        }
        loadingQueue.clear();
    }
    builder->EndPackage();
    
    return package;
}
    
bool UIPackageLoader::LoadControlByName(const String &name)
{
    for (size_t index = 0; index < loadingQueue.size(); index++)
    {
        if (loadingQueue[index].name == name)
        {
            switch (loadingQueue[index].status)
            {
                case STATUS_WAIT:
                    loadingQueue[index].status = STATUS_LOADING;
                    LoadControl(loadingQueue[index].node, true);
                    loadingQueue[index].status = STATUS_LOADED;
                    return true;
                    
                case STATUS_LOADED:
                    return true;
                    
                case STATUS_LOADING:
                    return false;
                    
                default:
                    DVASSERT(false);
                    return false;
            }
        }
    }
    return false;
}

void UIPackageLoader::LoadControl(const YamlNode *node, bool root)
{
    UIControl *control = NULL;
    const YamlNode *pathNode = node->Get("path");
    const YamlNode *prototypeNode = node->Get("prototype");
    const YamlNode *classNode = node->Get("class");
    if (pathNode)
    {
        control = builder->BeginControlWithPath(pathNode->AsString());
    }
    else if (prototypeNode)
    {
        const YamlNode *customClassNode = node->Get("customClass");
        String customClass = customClassNode == NULL ? "" : customClassNode->AsString();
        String controlName = prototypeNode->AsString();
        String packageName = "";
        size_t pos = controlName.find('/');
        if (pos != String::npos)
        {
            packageName = controlName.substr(0, pos);
            controlName = controlName.substr(pos + 1, controlName.length() - pos - 1);
        }
        
        control = builder->BeginControlWithPrototype(packageName, controlName, customClass, this);
    }
    else if (classNode)
    {
        const YamlNode *customClassNode = node->Get("customClass");
        if (customClassNode)
            control = builder->BeginControlWithCustomClass(customClassNode->AsString(), classNode->AsString());
        else
            control = builder->BeginControlWithClass(classNode->AsString());
    }
    else
    {
        builder->BeginUnknownControl(node);
    }

    if (control)
    {
        LoadControlPropertiesFromYamlNode(control, control->GetTypeInfo(), node);
        LoadBgPropertiesFromYamlNode(control, node);
        LoadInternalControlPropertiesFromYamlNode(control, node);

        // load children
        const YamlNode * childrenNode = node->Get("children");
        if (childrenNode)
        {
            uint32 count = childrenNode->GetCount();
            for (uint32 i = 0; i < count; i++)
                LoadControl(childrenNode->Get(i), false);
        }

        control->LoadFromYamlNodeCompleted();
        if (root)
            control->ApplyAlignSettingsForChildren();
        // yamlLoader->PostLoad(control);

    }
    builder->EndControl();
}

void UIPackageLoader::LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node)
{
    const InspInfo *baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        LoadControlPropertiesFromYamlNode(control, baseInfo, node);
    
    builder->BeginControlPropretiesSection(typeInfo->Name());
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        String memberName = member->Name();
        
        VariantType res;
        if (node)
            res = ReadVariantTypeFromYamlNode(member, node);
        builder->ProcessProperty(member, res);
    }
    builder->EndControlPropertiesSection();
}
    
void UIPackageLoader::LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node)
{
    const YamlNode *componentsNode = node ? node->Get("components") : NULL;
    
    for (int i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        const YamlNode *componentNode = NULL;
        
        if (componentsNode)
            componentNode = componentsNode->Get(control->GetBackgroundComponentName(i));
        
        UIControlBackground *bg = builder->BeginBgPropertiesSection(i, componentNode != NULL);
        if (bg)
        {
            const InspInfo *insp = bg->GetTypeInfo();
            for (int j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                VariantType res;
                if (componentNode)
                    res = ReadVariantTypeFromYamlNode(member, componentNode);
                builder->ProcessProperty(member, res);
            }
        }
        builder->EndBgPropertiesSection();
    }
}

void UIPackageLoader::LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node)
{
    const YamlNode *componentsNode = node ? node->Get("components") : NULL;
    for (int i = 0; i < control->GetInternalControlsCount(); i++)
    {
        const YamlNode *componentNode = NULL;
        if (componentsNode)
            componentNode = componentsNode->Get(control->GetInternalControlName(i) + control->GetInternalControlDescriptions());
        
        UIControl *internalControl = builder->BeginInternalControlSection(i, componentNode != NULL);
        if (internalControl)
        {
            const InspInfo *insp = internalControl->GetTypeInfo();
            String bgName = control->GetInternalControlName(i);
            
            for (int j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                String memberName = member->Name();
                
                VariantType value;
                if (componentNode)
                    value = ReadVariantTypeFromYamlNode(member, componentNode);
                builder->ProcessProperty(member, value);
            }
        }
        builder->EndInternalControlSection();
    }
}

VariantType UIPackageLoader::ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node)
{
    const YamlNode *valueNode = node->Get(member->Name());
    if (valueNode)
    {
        if (member->Desc().type == InspDesc::T_ENUM)
        {
            int val = 0;
            if (member->Desc().enumMap->ToValue(valueNode->AsString().c_str(), val))
            {
                return VariantType(val);
            }
            else
            {
                DVASSERT(false);
            }
        }
        else if (member->Desc().type == InspDesc::T_FLAGS)
        {
            int val = 0;
            for (uint32 i = 0; i < valueNode->GetCount(); i++)
            {
                const YamlNode *flagNode = valueNode->Get(i);
                int flag = 0;
                if (member->Desc().enumMap->ToValue(flagNode->AsString().c_str(), flag))
                {
                    val |= flag;
                }
                else
                {
                    DVASSERT(false);
                }
            }
            return VariantType(val);
        }
        else if (member->Type() == MetaInfo::Instance<bool>())
            return VariantType(valueNode->AsBool());
        else if (member->Type() == MetaInfo::Instance<int32>())
            return VariantType(valueNode->AsInt32());
        else if (member->Type() == MetaInfo::Instance<uint32>())
            return VariantType(valueNode->AsUInt32());
        else if (member->Type() == MetaInfo::Instance<String>())
            return VariantType(valueNode->AsString());
        else if (member->Type() == MetaInfo::Instance<WideString>())
            return VariantType(valueNode->AsWString());
        else if (member->Type() == MetaInfo::Instance<float32>())
            return VariantType(valueNode->AsFloat());
        else if (member->Type() == MetaInfo::Instance<Vector2>())
            return VariantType(valueNode->AsVector2());
        else if (member->Type() == MetaInfo::Instance<Color>())
            return VariantType(valueNode->AsColor());
        else if (member->Type() == MetaInfo::Instance<FilePath>())
            return VariantType(FilePath(valueNode->AsString()));
        else
        {
            DVASSERT(false);
            return VariantType();
        }
    }
    return VariantType();
}


}
