#include "UIPackageLoader.h"

#include "Base/ObjectFactory.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlHelpers.h"
#include "UI/UIPackage.h"
#include "UI/Components/UIComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"

namespace DAVA
{
UIPackageLoader::UIPackageLoader()
    : UIPackageLoader(Map<String, Set<FastName>>())
{
}

UIPackageLoader::UIPackageLoader(const Map<String, DAVA::Set<FastName>>& legacyPrototypes_)
    : legacyPrototypes(legacyPrototypes_)
{
    version = DAVA::UIPackage::CURRENT_VERSION;
    if (MIN_SUPPORTED_VERSION <= VERSION_WITH_LEGACY_ALIGNS)
    {
        legacyAlignsMap["leftAnchorEnabled"] = "leftAlignEnabled";
        legacyAlignsMap["leftAnchor"] = "leftAlign";
        legacyAlignsMap["hCenterAnchorEnabled"] = "hcenterAlignEnabled";
        legacyAlignsMap["hCenterAnchor"] = "hcenterAlign";
        legacyAlignsMap["rightAnchorEnabled"] = "rightAlignEnabled";
        legacyAlignsMap["rightAnchor"] = "rightAlign";
        legacyAlignsMap["topAnchorEnabled"] = "topAlignEnabled";
        legacyAlignsMap["topAnchor"] = "topAlign";
        legacyAlignsMap["vCenterAnchorEnabled"] = "vcenterAlignEnabled";
        legacyAlignsMap["vCenterAnchor"] = "vcenterAlign";
        legacyAlignsMap["bottomAnchorEnabled"] = "bottomAlignEnabled";
        legacyAlignsMap["bottomAnchor"] = "bottomAlign";
    }
    else
    {
        DVASSERT(false); // we have to remove legacy aligns support if min supported version more than version with legacy aligns
    }
}

UIPackageLoader::~UIPackageLoader()
{
}

bool UIPackageLoader::LoadPackage(const FilePath& packagePath, AbstractUIPackageBuilder* builder)
{
    if (!loadingQueue.empty())
    {
        DVASSERT(false);
        loadingQueue.clear();
    }

    if (!FileSystem::Instance()->Exists(packagePath))
        return false;

    RefPtr<YamlParser> parser(YamlParser::Create(packagePath));
    if (!parser.Valid())
        return false;

    YamlNode* rootNode = parser->GetRootNode();
    if (!rootNode) //empty yaml equal to empty UIPackage
    {
        builder->BeginPackage(packagePath);
        builder->EndPackage();
        return true;
    }

    return LoadPackage(rootNode, packagePath, builder);
}

bool UIPackageLoader::LoadPackage(const YamlNode* rootNode, const FilePath& packagePath, AbstractUIPackageBuilder* builder)
{
    const YamlNode* headerNode = rootNode->Get("Header");
    if (!headerNode)
        return false;

    const YamlNode* versionNode = headerNode->Get("version");
    if (versionNode == nullptr || versionNode->GetType() != YamlNode::TYPE_STRING)
    {
        return false;
    }

    int32 packageVersion = versionNode->AsInt();
    if (packageVersion < MIN_SUPPORTED_VERSION || UIPackage::CURRENT_VERSION < packageVersion)
    {
        return false;
    }

    builder->BeginPackage(packagePath);

    const YamlNode* importedPackagesNode = rootNode->Get("ImportedPackages");
    if (importedPackagesNode)
    {
        int32 count = static_cast<int32>(importedPackagesNode->GetCount());
        for (int32 i = 0; i < count; i++)
            builder->ProcessImportedPackage(importedPackagesNode->Get(i)->AsString(), this);
    }

    version = packageVersion; // store version in instance variables after importing packages

    const YamlNode* styleSheetsNode = rootNode->Get("StyleSheets");
    if (styleSheetsNode)
    {
        LoadStyleSheets(styleSheetsNode, builder);
    }

    const YamlNode* prototypesNode = rootNode->Get("Prototypes");
    if (prototypesNode)
    {
        int32 count = static_cast<int32>(prototypesNode->GetCount());
        for (int32 i = 0; i < count; i++)
        {
            const YamlNode* node = prototypesNode->Get(i);
            QueueItem item;
            item.name = node->Get("name")->AsFastName();
            item.node = node;
            item.status = STATUS_WAIT;
            loadingQueue.push_back(item);
        }

        for (int32 i = 0; i < count; i++)
        {
            if (loadingQueue[i].status == STATUS_WAIT)
            {
                loadingQueue[i].status = STATUS_LOADING;
                LoadControl(loadingQueue[i].node, AbstractUIPackageBuilder::TO_PROTOTYPES, builder);
                loadingQueue[i].status = STATUS_LOADED;
            }
        }

        loadingQueue.clear();
    }

    const YamlNode* controlsNode = rootNode->Get("Controls");
    if (controlsNode)
    {
        int32 count = static_cast<int32>(controlsNode->GetCount());
        for (int32 i = 0; i < count; i++)
        {
            const YamlNode* node = controlsNode->Get(i);
            QueueItem item;
            item.name = node->Get("name")->AsFastName();
            item.node = node;
            item.status = STATUS_WAIT;
            loadingQueue.push_back(item);
        }

        for (int32 i = 0; i < count; i++)
        {
            if (loadingQueue[i].status == STATUS_WAIT)
            {
                loadingQueue[i].status = STATUS_LOADING;
                AbstractUIPackageBuilder::eControlPlace controlPlace = AbstractUIPackageBuilder::TO_CONTROLS;
                if (version <= LAST_VERSION_WITHOUT_PROTOTYPES_SUPPORT)
                {
                    auto it = legacyPrototypes.find(packagePath.GetFrameworkPath());
                    if (it != legacyPrototypes.end())
                    {
                        if (it->second.find(loadingQueue[i].name) != it->second.end())
                        {
                            controlPlace = AbstractUIPackageBuilder::TO_PROTOTYPES;
                        }
                    }
                }

                LoadControl(loadingQueue[i].node, controlPlace, builder);
                loadingQueue[i].status = STATUS_LOADED;
            }
        }

        loadingQueue.clear();
    }

    builder->EndPackage();

    return true;
}

bool UIPackageLoader::LoadControlByName(const FastName& name, AbstractUIPackageBuilder* builder)
{
    size_t size = loadingQueue.size();
    for (size_t index = 0; index < size; index++)
    {
        if (loadingQueue[index].name == name)
        {
            switch (loadingQueue[index].status)
            {
            case STATUS_WAIT:
                loadingQueue[index].status = STATUS_LOADING;
                LoadControl(loadingQueue[index].node, AbstractUIPackageBuilder::TO_PROTOTYPES, builder);
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

void UIPackageLoader::LoadStyleSheets(const YamlNode* styleSheetsNode, AbstractUIPackageBuilder* builder)
{
    const Vector<YamlNode*>& styleSheetMap = styleSheetsNode->AsVector();
    const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();

    for (YamlNode* styleSheetNode : styleSheetMap)
    {
        const YamlNode* properties = styleSheetNode->Get("properties");

        if (properties != nullptr)
        {
            Vector<UIStyleSheetProperty> propertiesToSet;

            for (uint32 propertyIndex = 0; propertyIndex < properties->GetCount(); propertyIndex++)
            {
                FastName propertyName(properties->GetItemKeyName(propertyIndex));
                if (propertyDB->IsValidStyleSheetProperty(propertyName))
                {
                    uint32 index = propertyDB->GetStyleSheetPropertyIndex(propertyName);
                    const UIStyleSheetPropertyDescriptor& propertyDescr = propertyDB->GetStyleSheetPropertyByIndex(index);
                    if (propertyDescr.memberInfo != nullptr)
                    {
                        const YamlNode* propertyNode = properties->Get(propertyIndex);
                        const YamlNode* valueNode = propertyNode;
                        if (propertyNode->GetType() == YamlNode::TYPE_MAP)
                            valueNode = propertyNode->Get("value");

                        if (valueNode)
                        {
                            VariantType value(valueNode->AsVariantType(propertyDescr.memberInfo));

                            UIStyleSheetProperty property{ index, value };

                            if (propertyNode->GetType() == YamlNode::TYPE_MAP)
                            {
                                const YamlNode* transitionTime = propertyNode->Get("transitionTime");
                                if (transitionTime)
                                {
                                    property.transition = true;
                                    property.transitionTime = transitionTime->AsFloat();

                                    const YamlNode* transitionFunction = propertyNode->Get("transitionFunction");
                                    if (transitionFunction)
                                    {
                                        int32 transitionFunctionType = Interpolation::LINEAR;
                                        GlobalEnumMap<Interpolation::FuncType>::Instance()->ToValue(transitionFunction->AsString().c_str(), transitionFunctionType);
                                        property.transitionFunction = static_cast<Interpolation::FuncType>(transitionFunctionType);
                                    }
                                }
                            }

                            propertiesToSet.push_back(property);
                        }
                        else
                        {
                            DVASSERT(valueNode);
                        }
                    }
                }
                else
                {
                    Logger::Error("Unknown property name: %s", propertyName.c_str());
                    DVASSERT(false);
                }
            }

            Vector<String> selectorList;
            Split(styleSheetNode->Get("selector")->AsString(), ",", selectorList);
            Vector<UIStyleSheetSelectorChain> selectorChains;
            selectorChains.reserve(selectorList.size());

            for (const String& selectorString : selectorList)
            {
                selectorChains.push_back(UIStyleSheetSelectorChain(selectorString));
            }

            builder->ProcessStyleSheet(selectorChains, propertiesToSet);
        }
    }
}

void UIPackageLoader::LoadControl(const YamlNode* node, AbstractUIPackageBuilder::eControlPlace controlPlace, AbstractUIPackageBuilder* builder)
{
    UIControl* control = nullptr;
    const YamlNode* pathNode = node->Get("path");
    const YamlNode* prototypeNode = node->Get("prototype");
    const YamlNode* classNode = node->Get("class");
    const YamlNode* nameNode = node->Get("name");

    DVASSERT(nameNode || pathNode);

    FastName controlName;
    if (nameNode)
    {
        controlName = nameNode->AsFastName();
    }

    if (pathNode)
    {
        control = builder->BeginControlWithPath(pathNode->AsString());
    }
    else if (prototypeNode)
    {
        const YamlNode* customClassNode = node->Get("customClass");
        const String* customClass = customClassNode == nullptr ? nullptr : &(customClassNode->AsString());
        String prototypeName = prototypeNode->AsString();
        String packageName = "";
        size_t pos = prototypeName.find('/');
        if (pos != String::npos)
        {
            packageName = prototypeName.substr(0, pos);
            prototypeName = prototypeName.substr(pos + 1, prototypeName.length() - pos - 1);
        }
        control = builder->BeginControlWithPrototype(controlName, packageName, FastName(prototypeName), customClass, this);
    }
    else if (classNode)
    {
        const YamlNode* customClassNode = node->Get("customClass");
        if (customClassNode)
            control = builder->BeginControlWithCustomClass(controlName, customClassNode->AsString(), classNode->AsString());
        else
            control = builder->BeginControlWithClass(controlName, classNode->AsString());
    }
    else
    {
        builder->BeginUnknownControl(controlName, node);
    }

    if (control != nullptr)
    {
        LoadControlPropertiesFromYamlNode(control, control->GetTypeInfo(), node, builder);
        LoadComponentPropertiesFromYamlNode(control, node, builder);

        if (version <= VERSION_WITH_LEGACY_ALIGNS)
        {
            ProcessLegacyAligns(control, node, builder);
        }
    }

    // load children
    const YamlNode* childrenNode = node->Get("children");
    if (childrenNode)
    {
        uint32 count = childrenNode->GetCount();
        for (uint32 i = 0; i < count; i++)
            LoadControl(childrenNode->Get(i), AbstractUIPackageBuilder::TO_PREVIOUS_CONTROL, builder);
    }

    if (control != nullptr)
    {
        control->LoadFromYamlNodeCompleted();
    }

    builder->EndControl(controlPlace);
}

void UIPackageLoader::LoadControlPropertiesFromYamlNode(UIControl* control, const InspInfo* typeInfo, const YamlNode* node, AbstractUIPackageBuilder* builder)
{
    const InspInfo* baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        LoadControlPropertiesFromYamlNode(control, baseInfo, node, builder);

    builder->BeginControlPropertiesSection(typeInfo->Name().c_str());
    for (int32 i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember* member = typeInfo->Member(i);

        VariantType res;
        if (node)
            res = ReadVariantTypeFromYamlNode(member, node, member->Name().c_str());
        builder->ProcessProperty(member, res);
    }
    builder->EndControlPropertiesSection();
}

void UIPackageLoader::LoadComponentPropertiesFromYamlNode(UIControl* control, const YamlNode* node, AbstractUIPackageBuilder* builder)
{
    Vector<ComponentNode> components = ExtractComponentNodes(node);
    bool bgProcessed = false;
    for (ComponentNode& nodeDescr : components)
    {
        if (nodeDescr.type == UIComponent::BACKGROUND_COMPONENT)
        {
            bgProcessed = true;
        }
        UIComponent* component = builder->BeginComponentPropertiesSection(nodeDescr.type, nodeDescr.index);
        if (component)
        {
            const InspInfo* insp = component->GetTypeInfo();
            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember* member = insp->Member(j);
                VariantType res;
                if (nodeDescr.type == UIComponent::LINEAR_LAYOUT_COMPONENT && version <= LAST_VERSION_WITH_LINEAR_LAYOUT_LEGACY_ORIENTATION)
                {
                    static const FastName propertyName("orientation");
                    if (member->Name() == propertyName)
                    {
                        const YamlNode* valueNode = nodeDescr.node->Get(member->Name().c_str());
                        if (valueNode)
                        {
                            if (valueNode->AsString() == "Horizontal")
                            {
                                res.SetInt32(UILinearLayoutComponent::LEFT_TO_RIGHT);
                            }
                            else if (valueNode->AsString() == "Vertical")
                            {
                                res.SetInt32(UILinearLayoutComponent::TOP_DOWN);
                            }
                            else
                            {
                                DVASSERT(false);
                            }
                        }
                    }
                }
                if (nodeDescr.type == UIComponent::BACKGROUND_COMPONENT && version <= LAST_VERSION_WITH_LEGACY_SPRITE_MODIFICATION)
                {
                    static const FastName propertyName("spriteModification");
                    if (member->Name() == propertyName)
                    {
                        const YamlNode* valueNode = nodeDescr.node->Get(member->Name().c_str());
                        if (valueNode)
                        {
                            res.SetInt32(valueNode->AsInt32());
                        }
                    }
                }

                if (res.GetType() == VariantType::TYPE_NONE)
                {
                    res = ReadVariantTypeFromYamlNode(member, nodeDescr.node, member->Name().c_str());
                }

                builder->ProcessProperty(member, res);
            }
        }

        builder->EndComponentPropertiesSection();
    }
    if (!bgProcessed)
    {
        builder->BeginComponentPropertiesSection(UIComponent::BACKGROUND_COMPONENT, 0);
        builder->EndComponentPropertiesSection();
    }
}

void UIPackageLoader::ProcessLegacyAligns(UIControl* control, const YamlNode* node, AbstractUIPackageBuilder* builder)
{
    bool hasAnchorProperties = false;
    for (const auto& it : legacyAlignsMap)
    {
        if (node->Get(it.second))
        {
            hasAnchorProperties = true;
            break;
        }
    }

    if (hasAnchorProperties)
    {
        UIComponent* component = builder->BeginComponentPropertiesSection(UIComponent::ANCHOR_COMPONENT, 0);
        if (component)
        {
            const InspInfo* insp = component->GetTypeInfo();
            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember* member = insp->Member(j);
                VariantType res = ReadVariantTypeFromYamlNode(member, node, legacyAlignsMap[String(member->Name().c_str())]);
                builder->ProcessProperty(member, res);
            }
        }

        builder->EndComponentPropertiesSection();
    }
}

Vector<UIPackageLoader::ComponentNode> UIPackageLoader::ExtractComponentNodes(const YamlNode* node)
{
    const YamlNode* componentsNode = node ? node->Get("components") : nullptr;

    Vector<ComponentNode> components;

    if (componentsNode)
    {
        const EnumMap* componentTypes = GlobalEnumMap<UIComponent::eType>::Instance();

        for (uint32 i = 0; i < componentsNode->GetCount(); i++)
        {
            const String& fullName = componentsNode->GetItemKeyName(i);
            String::size_type lastChar = fullName.find_last_not_of("0123456789");
            String componentName = fullName.substr(0, lastChar + 1);
            uint32 componentIndex = atoi(fullName.substr(lastChar + 1).c_str());

            int32 componentType = 0;
            if (componentTypes->ToValue(componentName.c_str(), componentType))
            {
                if (componentType < UIComponent::COMPONENT_COUNT)
                {
                    ComponentNode n;
                    n.node = componentsNode->Get(i);
                    n.type = componentType;
                    n.index = componentIndex;
                    components.push_back(n);
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }

        std::stable_sort(components.begin(), components.end(), [](const ComponentNode& l, const ComponentNode& r) {
            return l.type == r.type ? l.index < r.index : l.type < r.type;
        });
    }
    return components;
}

VariantType UIPackageLoader::ReadVariantTypeFromYamlNode(const InspMember* member, const YamlNode* node, const String& propertyName)
{
    const YamlNode* valueNode = node->Get(propertyName);

    if (valueNode)
    {
        return valueNode->AsVariantType(member);
    }
    return VariantType();
}
}
