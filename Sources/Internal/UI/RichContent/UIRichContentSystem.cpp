#include "UI/RichContent/UIRichContentSystem.h"

#include "Base/ObjectFactory.h"
#include "FileSystem/XMLParser.h"
#include "Logger/Logger.h"
#include "UI/DefaultUIPackageBuilder.h"
#include "UI/UIPackageLoader.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIStaticText.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Layouts/UILayoutSourceRectComponent.h"
#include "UI/RichContent/UIRichAliasMap.h"
#include "UI/RichContent/UIRichContentComponent.h"
#include "UI/RichContent/UIRichContentObjectComponent.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "Utils/BiDiHelper.h"
#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"

namespace DAVA
{
class RichContentUIPackageBuilder : public DefaultUIPackageBuilder
{
public:
    RichContentUIPackageBuilder(UIPackagesCache* _cache = nullptr)
        : DefaultUIPackageBuilder(_cache)
    {
    }

protected:
    RefPtr<DAVA::UIControl> CreateControlByName(const DAVA::String& customClassName, const DAVA::String& className) override
    {
        if (ObjectFactory::Instance()->IsTypeRegistered(customClassName))
        {
            return RefPtr<UIControl>(ObjectFactory::Instance()->New<UIControl>(customClassName));
        }
        return RefPtr<UIControl>(ObjectFactory::Instance()->New<UIControl>(className));
    }

    std::unique_ptr<DAVA::DefaultUIPackageBuilder> CreateBuilder(DAVA::UIPackagesCache* packagesCache) override
    {
        return std::make_unique<RichContentUIPackageBuilder>(packagesCache);
    }
};

class XMLRichContentBuilder final : public XMLParserDelegate
{
public:
    XMLRichContentBuilder(UIRichContentComponent* component_, bool editorMode = false)
        : component(component_)
        , isEditorMode(editorMode)
    {
        DVASSERT(component);
        PutClass(component->GetBaseClasses());
    }

    bool Build(const String& text)
    {
        controls.clear();
        RefPtr<XMLParser> parser(new XMLParser());
        return parser->ParseBytes(reinterpret_cast<const unsigned char*>(text.c_str()), static_cast<int32>(text.length()), this);
    }

    const Vector<RefPtr<UIControl>>& GetControls() const
    {
        return controls;
    }

    void PutClass(const String& clazz)
    {
        String compositeClass = GetClass() + " " + clazz;
        classesStack.push_back(compositeClass);
    }

    void PopClass()
    {
        classesStack.pop_back();
    }

    const String& GetClass() const
    {
        if (classesStack.empty())
        {
            static const String EMPTY;
            return EMPTY;
        }
        return classesStack.back();
    }

    void PrepareControl(UIControl* ctrl, bool autosize)
    {
        ctrl->SetClassesFromString(GetClass());

        if (isEditorMode)
        {
            ctrl->GetOrCreateComponent<UILayoutSourceRectComponent>();
        }

        if (autosize)
        {
            UISizePolicyComponent* sp = ctrl->GetOrCreateComponent<UISizePolicyComponent>();
            sp->SetHorizontalPolicy(UISizePolicyComponent::eSizePolicy::PERCENT_OF_CONTENT);
            sp->SetVerticalPolicy(UISizePolicyComponent::eSizePolicy::PERCENT_OF_CONTENT);
        }

        UIFlowLayoutHintComponent* flh = ctrl->GetOrCreateComponent<UIFlowLayoutHintComponent>();
        flh->SetContentDirection(direction);
        if (needLineBreak)
        {
            flh->SetNewLineBeforeThis(needLineBreak);
            needLineBreak = false;
        }

        //ctrl->GetOrCreateComponent<UIRichContentItemComponent>();
    }

    void OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes) override
    {
        const UIRichAliasMap& aliases = component->GetAliases();
        if (aliases.HasAlias(elementName))
        {
            const UIRichAliasMap::Alias& alias = aliases.GetAlias(elementName);
            ProcessTagBegin(alias.tag, alias.attributes);
        }
        else
        {
            ProcessTagBegin(elementName, attributes);
        }
    }

    void OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName) override
    {
        const UIRichAliasMap& aliases = component->GetAliases();
        if (aliases.HasAlias(elementName))
        {
            const UIRichAliasMap::Alias& alias = aliases.GetAlias(elementName);
            ProcessTagEnd(alias.tag);
        }
        else
        {
            ProcessTagEnd(elementName);
        }
    }

    void OnFoundCharacters(const String& chars) override
    {
        ProcessText(chars);
    }

    void ProcessTagBegin(const String& tag, const Map<String, String>& attributes)
    {
        // Global attributes
        String classes;
        GetAttribute(attributes, "class", classes);
        PutClass(classes);

        // Tag
        if (tag == "p")
        {
            needLineBreak = true;
        }
        else if (tag == "br")
        {
            needLineBreak = true;
        }
        else if (tag == "ul")
        {
            needLineBreak = true;
        }
        else if (tag == "li")
        {
            needLineBreak = true;
            ProcessText("*"); // TODO: Change to create "bullet" control
        }
        else if (tag == "img")
        {
            String src;
            if (GetAttribute(attributes, "src", src))
            {
                UIControl* img = new UIControl();
                PrepareControl(img, true);
                UIControlBackground* bg = img->GetOrCreateComponent<UIControlBackground>();
                bg->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
                bg->SetSprite(FilePath(src));
                controls.emplace_back(img);
            }
        }
        else if (tag == "object")
        {
            String path;
            GetAttribute(attributes, "path", path);
            String controlName;
            GetAttribute(attributes, "control", controlName);
            String prototypeName;
            GetAttribute(attributes, "prototype", prototypeName);
            String name;
            GetAttribute(attributes, "name", name);

            if (!path.empty() && (!controlName.empty() || !prototypeName.empty()))
            {
                // Check that we not load self as rich object
                bool valid = true;
                {
                    UIControl* ctrl = component->GetControl();
                    while (ctrl != nullptr)
                    {
                        UIRichContentObjectComponent* objComp = ctrl->GetComponent<UIRichContentObjectComponent>();
                        if (objComp)
                        {
                            if (path == objComp->GetPackagePath() &&
                                controlName == objComp->GetControlName() &&
                                prototypeName == objComp->GetPrototypeName())
                            {
                                valid = false;
                                break;
                            }
                        }
                        ctrl = ctrl->GetParent();
                    }
                }

                if (valid)
                {
                    std::unique_ptr<DefaultUIPackageBuilder> pkgBuilder = isEditorMode ? std::make_unique<RichContentUIPackageBuilder>() : std::make_unique<DefaultUIPackageBuilder>();
                    UIPackageLoader().LoadPackage(path, pkgBuilder.get());
                    UIControl* obj = nullptr;
                    UIPackage* pkg = pkgBuilder->GetPackage();
                    if (pkg != nullptr)
                    {
                        if (!controlName.empty())
                        {
                            obj = pkg->GetControl(controlName);
                        }
                        else if (!prototypeName.empty())
                        {
                            obj = pkg->GetPrototype(prototypeName);
                        }
                    }
                    if (obj != nullptr)
                    {
                        PrepareControl(obj, false);

                        UIRichContentObjectComponent* objComp = obj->GetOrCreateComponent<UIRichContentObjectComponent>();
                        objComp->SetPackagePath(path);
                        objComp->SetControlName(controlName);
                        objComp->SetPrototypeName(prototypeName);

                        if (!name.empty())
                        {
                            obj->SetName(name);
                        }
                        component->onCreateObject.Emit(obj);
                        controls.emplace_back(obj);
                    }
                }
                else
                {
                    Logger::Error("[UIRichContentSystem] Recursive object in rich content from '%s' with name '%s'!",
                                  path.c_str(),
                                  controlName.empty() ? prototypeName.c_str() : controlName.c_str());
                }
            }
        }
    }

    void ProcessTagEnd(const String& tag)
    {
        PopClass();

        if (tag == "p")
        {
            needLineBreak = true;
        }
        else if (tag == "ul")
        {
            needLineBreak = true;
        }
        else if (tag == "li")
        {
            needLineBreak = true;
        }
    }

    void ProcessText(const String& text)
    {
        const static String LTR_MARK = UTF8Utils::EncodeToUTF8(L"\u200E");
        const static String RTL_MARK = UTF8Utils::EncodeToUTF8(L"\u200F");

        Vector<String> tokens;
        Split(text, " \n\r\t", tokens);
        for (String& token : tokens)
        {
            BiDiHelper::Direction wordDirection = bidiHelper.GetDirectionUTF8String(token);
            if (wordDirection == BiDiHelper::Direction::NEUTRAL)
            {
                if (direction == BiDiHelper::Direction::RTL)
                {
                    token = RTL_MARK + token;
                }
                else if (direction == BiDiHelper::Direction::LTR)
                {
                    token = LTR_MARK + token;
                }
            }
            else
            {
                direction = wordDirection;
            }

            UIStaticText* ctrl = new UIStaticText();
            PrepareControl(ctrl, true);
            ctrl->SetUtf8Text(token);
#if _DEBUG // TODO: Remove before merge
            ctrl->SetForceBiDiSupportEnabled(true);
#endif
            controls.emplace_back(ctrl);
        }
    }

private:
    bool needLineBreak = false;
    bool isEditorMode = false;
    BiDiHelper::Direction direction = BiDiHelper::Direction::NEUTRAL;
    Vector<String> classesStack;
    Vector<RefPtr<UIControl>> controls;
    BiDiHelper bidiHelper;
    UIRichContentComponent* component = nullptr;
};

/*******************************************************************************************************/

void UIRichContentSystem::SetEditorMode(bool editorMode)
{
    isEditorMode = editorMode;
}

void UIRichContentSystem::RegisterControl(UIControl* control)
{
    UISystem::RegisterControl(control);
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        AddLink(component);
    }
}

void UIRichContentSystem::UnregisterControl(UIControl* control)
{
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        RemoveLink(component);
    }

    UISystem::UnregisterControl(control);
}

void UIRichContentSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    UISystem::RegisterComponent(control, component);

    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
        AddLink(static_cast<UIRichContentComponent*>(component));
    }
}

void UIRichContentSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
        RemoveLink(static_cast<UIRichContentComponent*>(component));
    }

    UISystem::UnregisterComponent(control, component);
}

void UIRichContentSystem::Process(float32 elapsedTime)
{
    // Add new links
    if (!appendLinks.empty())
    {
        links.insert(links.end(), appendLinks.begin(), appendLinks.end());
        appendLinks.clear();
    }
    // Remove empty links
    if (!links.empty())
    {
        links.erase(std::remove_if(links.begin(), links.end(), [](const Link& l) {
                        return l.component == nullptr;
                    }),
                    links.end());
    }

    // Process links
    for (Link& l : links)
    {
        if (l.component && l.component->IsModified())
        {
            l.component->SetModified(false);

            UIControl* root = l.component->GetControl();
            l.RemoveItems();

            XMLRichContentBuilder builder(l.component, isEditorMode);
            if (builder.Build("<span>" + l.component->GetText() + "</span>"))
            {
                for (const RefPtr<UIControl>& ctrl : builder.GetControls())
                {
                    root->AddControl(ctrl.Get());
                    l.AddItem(ctrl);
                }
            }
        }
    }
}

void UIRichContentSystem::AddLink(UIRichContentComponent* component)
{
    DVASSERT(component);
    component->SetModified(true);
    appendLinks.emplace_back(component);
}

void UIRichContentSystem::RemoveLink(UIRichContentComponent* component)
{
    DVASSERT(component);
    auto findIt = std::find_if(links.begin(), links.end(), [&component](const Link& l) {
        return l.component == component;
    });
    if (findIt != links.end())
    {
        findIt->RemoveItems();
        findIt->component = nullptr; // mark link for delete
    }
}

UIRichContentSystem::Link::Link(UIRichContentComponent* c)
    : component(c)
    , richItems()
{
}

void UIRichContentSystem::Link::AddItem(const RefPtr<UIControl>& item)
{
    richItems.push_back(item);
}

void UIRichContentSystem::Link::RemoveItems()
{
    if (component != nullptr)
    {
        UIControl* ctrl = component->GetControl();
        if (ctrl != nullptr)
        {
            for (const RefPtr<UIControl>& item : richItems)
            {
                ctrl->RemoveControl(item.Get());
            }
        }
    }
    richItems.clear();
}
}
