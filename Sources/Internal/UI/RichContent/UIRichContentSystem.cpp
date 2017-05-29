#include "UI/RichContent/UIRichContentSystem.h"

#include "Base/ObjectFactory.h"
#include "Debug/DVAssert.h"
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
#include "UI/RichContent/UIRichContentAliasesComponent.h"
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
    XMLRichContentBuilder(UIRichContentSystem::Link* link_, bool editorMode = false)
        : link(link_)
        , isEditorMode(editorMode)
    {
        DVASSERT(link);
        defaultClasses = link->component->GetBaseClasses();
        classesInheritance = link->component->GetClassesInheritance();

        PutClass(defaultClasses);
    }

    bool Build(const String& text)
    {
        controls.clear();
        direction = bidiHelper.GetDirectionUTF8String(text); // Detect text direction
        RefPtr<XMLParser> parser(new XMLParser());
        return parser->ParseBytes(reinterpret_cast<const unsigned char*>(text.c_str()), static_cast<int32>(text.length()), this);
    }

    const Vector<RefPtr<UIControl>>& GetControls() const
    {
        return controls;
    }

    void PutClass(const String& clazz)
    {
        String compositeClass;
        if (classesInheritance)
        {
            compositeClass = GetClass();
            if (!clazz.empty())
            {
                compositeClass += " ";
            }
        }
        compositeClass += clazz;

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
        ctrl->SetClassesFromString(ctrl->GetClassesAsString() + " " + GetClass());

        if (isEditorMode)
        {
            UILayoutSourceRectComponent* src = ctrl->GetOrCreateComponent<UILayoutSourceRectComponent>();
            src->SetSize(ctrl->GetSize());
            src->SetPosition(ctrl->GetPosition());
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
    }

    void OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes) override
    {
        for (UIRichContentAliasesComponent* c : link->aliasesComponents)
        {
            const UIRichAliasMap& aliases = c->GetAliases();
            if (aliases.HasAlias(elementName))
            {
                const UIRichAliasMap::Alias& alias = aliases.GetAlias(elementName);
                ProcessTagBegin(alias.tag, alias.attributes);
                return;
            }
        }
        ProcessTagBegin(elementName, attributes);
    }

    void OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName) override
    {
        for (UIRichContentAliasesComponent* c : link->aliasesComponents)
        {
            const UIRichAliasMap& aliases = c->GetAliases();
            if (aliases.HasAlias(elementName))
            {
                const UIRichAliasMap::Alias& alias = aliases.GetAlias(elementName);
                ProcessTagEnd(alias.tag);
                return;
            }
        }
        ProcessTagEnd(elementName);
    }

    void OnFoundCharacters(const String& chars) override
    {
        ProcessText(chars);
    }

    void ProcessTagBegin(const String& tag, const Map<String, String>& attributes)
    {
        // Global attributes
        String classes;
        if (!GetAttribute(attributes, "class", classes) && !classesInheritance)
        {
            classes = defaultClasses;
        }
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
                    UIControl* ctrl = link->control;
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
                        link->component->onCreateObject.Emit(obj);
                        controls.emplace_back(SafeRetain(obj));
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
            controls.emplace_back(ctrl);
        }
    }

private:
    bool needLineBreak = false;
    bool isEditorMode = false;
    bool classesInheritance = false;
    BiDiHelper::Direction direction = BiDiHelper::Direction::NEUTRAL;
    String defaultClasses;
    Vector<String> classesStack;
    Vector<RefPtr<UIControl>> controls;
    BiDiHelper bidiHelper;
    UIRichContentSystem::Link* link = nullptr;
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
    else if (component->GetType() == UIRichContentAliasesComponent::C_TYPE)
    {
        AddAliases(control, static_cast<UIRichContentAliasesComponent*>(component));
    }
}

void UIRichContentSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
        RemoveLink(static_cast<UIRichContentComponent*>(component));
    }
    else if (component->GetType() == UIRichContentAliasesComponent::C_TYPE)
    {
        RemoveAliases(control, static_cast<UIRichContentAliasesComponent*>(component));
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
        if (l.component)
        {
            bool update = l.component->IsModified();
            l.component->SetModified(false);
            for (UIRichContentAliasesComponent* c : l.aliasesComponents)
            {
                update |= c->IsModified();
                c->SetModified(false);
            }

            if (update)
            {
                UIControl* root = l.component->GetControl();
                l.RemoveItems();

                XMLRichContentBuilder builder(&l, isEditorMode);
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
}

void UIRichContentSystem::AddLink(UIRichContentComponent* component)
{
    DVASSERT(component);
    component->SetModified(true);

    Link link;
    link.component = component;
    link.control = component->GetControl();

    uint32 count = link.control->GetComponentCount<UIRichContentAliasesComponent>();
    for (uint32 i = 0; i < count; ++i)
    {
        UIRichContentAliasesComponent* c = link.control->GetComponent<UIRichContentAliasesComponent>(i);
        link.AddAliases(c);
    }

    appendLinks.push_back(std::move(link));
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

    appendLinks.erase(std::remove_if(appendLinks.begin(), appendLinks.end(), [&component](const Link& l) {
                          return l.component == component;
                      }),
                      appendLinks.end());
}

void UIRichContentSystem::AddAliases(UIControl* control, UIRichContentAliasesComponent* component)
{
    auto findIt = std::find_if(links.begin(), links.end(), [&control](const Link& l) {
        return l.control == control;
    });
    if (findIt != links.end())
    {
        findIt->AddAliases(component);
    }
}

void UIRichContentSystem::RemoveAliases(UIControl* control, UIRichContentAliasesComponent* component)
{
    auto findIt = std::find_if(links.begin(), links.end(), [&control](const Link& l) {
        return l.control == control;
    });
    if (findIt != links.end())
    {
        findIt->RemoveAliases(component);
    }
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

void UIRichContentSystem::Link::AddAliases(UIRichContentAliasesComponent* component)
{
    auto it = std::find(aliasesComponents.begin(), aliasesComponents.end(), component);
    if (it == aliasesComponents.end())
    {
        aliasesComponents.push_back(component);
    }
}

void UIRichContentSystem::Link::RemoveAliases(UIRichContentAliasesComponent* component)
{
    auto it = std::find(aliasesComponents.begin(), aliasesComponents.end(), component);
    if (it != aliasesComponents.end())
    {
        aliasesComponents.erase(it);
    }
}
}
