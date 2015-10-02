/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "UIPackageLoader.h"

#include "Base/ObjectFactory.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlHelpers.h"
#include "UI/UIPackage.h"
#include "UI/Components/UIComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "Utils/Utils.h"

namespace DAVA
{

UIPackageLoader::UIPackageLoader()
{
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

bool UIPackageLoader::LoadPackage(const FilePath &packagePath, AbstractUIPackageBuilder *builder)
{
    if (!loadingQueue.empty())
    {
        DVASSERT(false);
        loadingQueue.clear();
    }

    if (!packagePath.Exists())
        return false;

    RefPtr<YamlParser> parser(YamlParser::Create(packagePath));
    if (!parser.Valid())
        return false;

    YamlNode *rootNode = parser->GetRootNode();
    if (!rootNode)//empty yaml equal to empty UIPackage
    {
        builder->BeginPackage(packagePath);
        builder->EndPackage();
        return true;
    }

    return LoadPackage(rootNode, packagePath, builder);
}

bool UIPackageLoader::LoadPackage(const YamlNode *rootNode, const FilePath &packagePath, AbstractUIPackageBuilder *builder)
{
    const YamlNode *headerNode = rootNode->Get("Header");
    if (!headerNode)
        return false;

    const YamlNode *versionNode = headerNode->Get("version");
    if (versionNode == nullptr || versionNode->GetType() != YamlNode::TYPE_STRING)
    {
        return false;
    }
    
    int32 packageVersion = versionNode->AsInt();
    if (packageVersion < MIN_SUPPORTED_VERSION || CURRENT_VERSION < packageVersion)
    {
        return false;
    }
    

    builder->BeginPackage(packagePath);

    const YamlNode *importedPackagesNode = rootNode->Get("ImportedPackages");
    if (importedPackagesNode)
    {
        int32 count = (int32) importedPackagesNode->GetCount();
        for (int32 i = 0; i < count; i++)
            builder->ProcessImportedPackage(importedPackagesNode->Get(i)->AsString(), this);
    }
    
    version = packageVersion; // store version in instance variables after importing packages
    
    const YamlNode *styleSheetsNode = rootNode->Get("StyleSheets");
    if (styleSheetsNode)
    {
        LoadStyleSheets(styleSheetsNode, builder);
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
                LoadControl(loadingQueue[i].node, true, builder);
                loadingQueue[i].status = STATUS_LOADED;
            }
        }

        loadingQueue.clear();
    }


    builder->EndPackage();

    return true;
}

bool UIPackageLoader::LoadControlByName(const String &name, AbstractUIPackageBuilder *builder)
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
                    LoadControl(loadingQueue[index].node, true, builder);
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

void UIPackageLoader::LoadStyleSheets(const YamlNode *styleSheetsNode, AbstractUIPackageBuilder *builder)
{
    const Vector<YamlNode*> &styleSheetMap = styleSheetsNode->AsVector();
    const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();
    
    for (YamlNode *styleSheetNode : styleSheetMap)
    {
        const MultiMap<String, YamlNode*> &styleSheet = styleSheetNode->AsMap();
        
        auto propertiesSectionIter = styleSheet.find("properties");
        
        if (propertiesSectionIter != styleSheet.end())
        {
            Vector<UIStyleSheetProperty> propertiesToSet;
            
            for (const auto& propertyIter : propertiesSectionIter->second->AsMap())
            {
                uint32 index = propertyDB->GetStyleSheetPropertyIndex(FastName(propertyIter.first));
                const UIStyleSheetPropertyDescriptor& propertyDescr = propertyDB->GetStyleSheetPropertyByIndex(index);
                if (propertyDescr.memberInfo != nullptr)
                {
                    const YamlNode *propertyNode = propertyIter.second;
                    const YamlNode *valueNode = propertyNode;
                    if (propertyNode->GetType() == YamlNode::TYPE_MAP)
                        valueNode = propertyNode->Get("value");
                    
                    if (valueNode)
                    {
                        VariantType value(valueNode->AsVariantType(propertyDescr.memberInfo));

                        UIStyleSheetProperty property{ index, value };
                        
                        if (propertyNode->GetType() == YamlNode::TYPE_MAP)
                        {
                            const YamlNode *transitionTime = propertyNode->Get("transitionTime");
                            if (transitionTime)
                            {
                                property.transition = true;
                                property.transitionTime = transitionTime->AsFloat();

                                const YamlNode *transitionFunction = propertyNode->Get("transitionFunction");
                                if (transitionFunction)
                                {
                                    int32 transitionFunctionType = Interpolation::LINEAR;
                                    GlobalEnumMap<Interpolation::FuncType>::Instance()->ToValue(transitionFunction->AsString().c_str(), transitionFunctionType);
                                    property.transitionFunction = (Interpolation::FuncType)transitionFunctionType;
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
            
            Vector<String> selectorList;
            Split(styleSheet.find("selector")->second->AsString(), ",", selectorList);
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

void UIPackageLoader::LoadControl(const YamlNode *node, bool root, AbstractUIPackageBuilder *builder)
{
    UIControl *control = nullptr;
    const YamlNode *pathNode = node->Get("path");
    const YamlNode *prototypeNode = node->Get("prototype");
    const YamlNode *classNode = node->Get("class");
    const YamlNode *nameNode = node->Get("name");

    //DVASSERT(nameNode || pathNode);

    if (pathNode)
    {
        control = builder->BeginControlWithPath(pathNode->AsString());
    }
    else if (prototypeNode)
    {
        const YamlNode *customClassNode = node->Get("customClass");
        const String *customClass = customClassNode == nullptr ? nullptr : &(customClassNode->AsString());
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
        if (nameNode)
            control->SetName(nameNode->AsString());
        LoadControlPropertiesFromYamlNode(control, control->GetTypeInfo(), node, builder);
        LoadComponentPropertiesFromYamlNode(control, node, builder);
        LoadBgPropertiesFromYamlNode(control, node, builder);
        LoadInternalControlPropertiesFromYamlNode(control, node, builder);
        
        if (version == VERSION_WITH_LEGACY_ALIGNS)
        {
            ProcessLegacyAligns(control, node, builder);
        }
        
        // load children
        const YamlNode * childrenNode = node->Get("children");
        if (childrenNode)
        {
            uint32 count = childrenNode->GetCount();
            for (uint32 i = 0; i < count; i++)
                LoadControl(childrenNode->Get(i), false, builder);
        }

        control->LoadFromYamlNodeCompleted();

    }
    builder->EndControl(root);
}

void UIPackageLoader::LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node, AbstractUIPackageBuilder *builder)
{
    const InspInfo *baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        LoadControlPropertiesFromYamlNode(control, baseInfo, node, builder);

    builder->BeginControlPropertiesSection(typeInfo->Name().c_str());
    for (int32 i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);

        VariantType res;
        if (node)
            res = ReadVariantTypeFromYamlNode(member, node, member->Name().c_str());
        builder->ProcessProperty(member, res);
    }
    builder->EndControlPropertiesSection();
}

void UIPackageLoader::LoadComponentPropertiesFromYamlNode(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder)
{
    Vector<ComponentNode> components = ExtractComponentNodes(node);
    for (auto &nodeDescr : components)
    {
        UIComponent *component = builder->BeginComponentPropertiesSection(nodeDescr.type, nodeDescr.index);
        if (component)
        {
            const InspInfo *insp = component->GetTypeInfo();
            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                VariantType res = ReadVariantTypeFromYamlNode(member, nodeDescr.node, member->Name().c_str());
                builder->ProcessProperty(member, res);
            }
        }

        builder->EndComponentPropertiesSection();
    }
}
    
void UIPackageLoader::ProcessLegacyAligns(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder)
{
    bool hasAnchorProperties = false;
    for (const auto &it : legacyAlignsMap)
    {
        if (node->Get(it.second))
        {
            hasAnchorProperties = true;
            break;
        }
    }
    
    if (hasAnchorProperties)
    {
        UIComponent *component = builder->BeginComponentPropertiesSection(UIComponent::ANCHOR_COMPONENT, 0);
        if (component)
        {
            const InspInfo *insp = component->GetTypeInfo();
            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                VariantType res = ReadVariantTypeFromYamlNode(member, node, legacyAlignsMap[String(member->Name().c_str())]);
                builder->ProcessProperty(member, res);
            }
        }
        
        builder->EndComponentPropertiesSection();
    }
    
}

Vector<UIPackageLoader::ComponentNode> UIPackageLoader::ExtractComponentNodes(const YamlNode *node)
{
    const YamlNode *componentsNode = node ? node->Get("components") : nullptr;

    Vector<ComponentNode> components;

    if (componentsNode)
    {
        const EnumMap *componentTypes = GlobalEnumMap<UIComponent::eType>::Instance();

        for (uint32 i = 0; i < componentsNode->GetCount(); i++)
        {
            const String &fullName = componentsNode->GetItemKeyName(i);
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

        std::stable_sort(components.begin(), components.end(), [](ComponentNode l, ComponentNode r) {
            return l.type == r.type ? l.index < r.index : l.type < r.type;
        });
    }
    return components;
}

void UIPackageLoader::LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder)
{
    const YamlNode *componentsNode = node ? node->Get("components") : nullptr;

    for (int32 i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        const YamlNode *componentNode = nullptr;

        if (componentsNode)
            componentNode = componentsNode->Get(control->GetBackgroundComponentName(i));

        UIControlBackground *bg = builder->BeginBgPropertiesSection(i, componentNode != nullptr);
        if (bg)
        {
            const InspInfo *insp = bg->GetTypeInfo();
            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);
                VariantType res;
                if (componentNode)
                    res = ReadVariantTypeFromYamlNode(member, componentNode, member->Name().c_str());
                builder->ProcessProperty(member, res);
            }
        }
        builder->EndBgPropertiesSection();
    }
}

void UIPackageLoader::LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node, AbstractUIPackageBuilder *builder)
{
    const YamlNode *componentsNode = node ? node->Get("components") : nullptr;
    for (int32 i = 0; i < control->GetInternalControlsCount(); i++)
    {
        const YamlNode *componentNode = nullptr;
        if (componentsNode)
            componentNode = componentsNode->Get(control->GetInternalControlName(i) + control->GetInternalControlDescriptions());

        UIControl *internalControl = builder->BeginInternalControlSection(i, componentNode != nullptr);
        if (internalControl)
        {
            const InspInfo *insp = internalControl->GetTypeInfo();

            for (int32 j = 0; j < insp->MembersCount(); j++)
            {
                const InspMember *member = insp->Member(j);

                VariantType value;
                if (componentNode)
                    value = ReadVariantTypeFromYamlNode(member, componentNode, member->Name().c_str());
                builder->ProcessProperty(member, value);
            }
        }
        builder->EndInternalControlSection();
    }
}

VariantType UIPackageLoader::ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node, const String &propertyName)
{
    const YamlNode *valueNode = node->Get(propertyName);

    if (valueNode)
    {
        return valueNode->AsVariantType(member);
    }
    return VariantType();
}


}
