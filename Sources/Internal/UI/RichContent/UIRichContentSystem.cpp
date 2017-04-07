#include "UIRichContentSystem.h"
#include "FileSystem/XMLParser.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIStaticText.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/RichContent/UIRichAliasMap.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "Utils/Utils.h"
#include "Utils/BiDiHelper.h"

namespace DAVA
{
class XMLBuilder final : public XMLParserDelegate
{
public:
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

    void SetBaseClass(const String& clazz)
    {
        classesStack.clear();
        classesStack.push_back(clazz);
    }

    void SetAliases(const UIRichAliasMap& _aliases)
    {
        aliases = _aliases;
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
        if (tag == "span")
        {
        }
        else if (tag == "p")
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

            UIStaticText* ctrl = new UIStaticText();
            PrepareControl(ctrl, true);
            ctrl->SetUtf8Text("*");
            ctrl->SetForceBiDiSupportEnabled(true);
            controls.emplace_back(ctrl);
        }
        else if (tag == "img")
        {
            String src;
            if (GetAttribute(attributes, "src", src))
            {
                UIControl* img = new UIControl();
                PrepareControl(img, true);
                UIControlBackground* bg = img->GetOrCreateComponent<UIControlBackground>();
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

            if (!path.empty() && (!controlName.empty() || !prototypeName.empty()))
            {
                DefaultUIPackageBuilder pkgBuilder;
                UIPackageLoader().LoadPackage(path, &pkgBuilder);
                UIControl* obj = nullptr;
                if (!controlName.empty())
                {
                    obj = pkgBuilder.GetPackage()->GetControl(controlName);
                }
                else if (!prototypeName.empty())
                {
                    obj = pkgBuilder.GetPackage()->GetPrototype(prototypeName);
                }
                if (obj != nullptr)
                {
                    obj = obj->Clone(); // Clone control from package
                    PrepareControl(obj, false); // TODO: Need it for prototypes?
                    controls.emplace_back(obj);
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
        Vector<String> tokens;
        Split(text, " \n", tokens);
        for (auto token : tokens)
        {
            direction = bidiHelper.GetDirectionUTF8String(token);
            UIStaticText* ctrl = new UIStaticText();
            PrepareControl(ctrl, true);
            ctrl->SetUtf8Text(token);
            ctrl->SetForceBiDiSupportEnabled(true);
            controls.emplace_back(ctrl);
        }
    }

private:
    bool needLineBreak = false;
    BiDiHelper::Direction direction = BiDiHelper::Direction::NEUTRAL;
    Vector<String> classesStack;
    Vector<RefPtr<UIControl>> controls;
    BiDiHelper bidiHelper;
    UIRichAliasMap aliases;
};

/*******************************************************************************************************/

void UIRichContentSystem::RegisterControl(UIControl* control)
{
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        component->SetModified(true);
        links.emplace_back(component);
    }
}

void UIRichContentSystem::UnregisterControl(UIControl* control)
{
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        auto findIt = std::find_if(links.begin(), links.end(), [&component](const Link& l) {
            return l.component == component;
        });
        DVASSERT(findIt != links.end());
        findIt->component->GetControl()->RemoveAllControls();
        findIt->component = nullptr; // mark link for delete
    }
}

void UIRichContentSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
        UIRichContentComponent* rich = static_cast<UIRichContentComponent*>(component);
        rich->SetModified(true);
        links.emplace_back(rich);
    }
}

void UIRichContentSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
        auto findIt = std::find_if(links.begin(), links.end(), [&component](const Link& l) {
            return l.component == component;
        });
        DVASSERT(findIt != links.end());
        findIt->component->GetControl()->RemoveAllControls();
        findIt->component = nullptr; // mark link for delete
    }
}

void UIRichContentSystem::OnControlVisible(UIControl* control)
{
}

void UIRichContentSystem::OnControlInvisible(UIControl* control)
{
}

void UIRichContentSystem::Process(float32 elapsedTime)
{
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
        DVASSERT(l.component);
        if (l.component->IsModified())
        {
            l.component->SetModified(false);

            UIControl* root = l.component->GetControl();
            root->RemoveAllControls();

            XMLBuilder builder;
            builder.SetBaseClass(l.component->GetBaseClasses());
            builder.SetAliases(l.component->GetAliases());
            if (builder.Build("<span>" + l.component->GetText() + "</span>"))
            {
                for (const RefPtr<UIControl>& ctrl : builder.GetControls())
                {
                    root->AddControl(ctrl.Get());
                }
            }
        }
    }
}
}
