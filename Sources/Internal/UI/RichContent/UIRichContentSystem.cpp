#include "UIRichContentSystem.h"
#include "FileSystem/XMLParser.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIStaticText.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "Utils/Utils.h"
#include "Utils/BiDiHelper.h"

namespace DAVA
{
class XMLBuilder final : public XMLParserDelegate
{
public:
    void Reset()
    {
        controls.clear();
        classesStack.clear();
        context = nullptr;
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

    void SetContext(UIControlPackageContext* _context)
    {
        context = _context;
    }

    void SetTopClass(const String& clazz)
    {
        classesStack.clear();
        classesStack.push_back(clazz);
    }

    void PutClass(const String& clazz)
    {
        String compositeClass;
        Merge(classesStack, ' ', compositeClass);
        compositeClass += ' ' + clazz;
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

    void PrepareControl(UIControl* ctrl)
    {
        ctrl->SetPackageContext(context);
        ctrl->SetClassesFromString(GetClass());

        UISizePolicyComponent* sp = ctrl->GetOrCreateComponent<UISizePolicyComponent>();
        sp->SetHorizontalPolicy(UISizePolicyComponent::eSizePolicy::PERCENT_OF_CONTENT);
        sp->SetVerticalPolicy(UISizePolicyComponent::eSizePolicy::PERCENT_OF_CONTENT);

        if (isRtl || needLineBreak)
        {
            UIFlowLayoutHintComponent* flh = ctrl->GetOrCreateComponent<UIFlowLayoutHintComponent>();
            flh->SetRtlContent(isRtl);
            flh->SetNewLineBeforeThis(needLineBreak);
            needLineBreak = false;
        }
    }

    void OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes) override
    {
        //Logger::Debug("OnElementStarted: '%s', '%s', '%s', %s", elementName.c_str(), namespaceURI.c_str(), qualifedName.c_str(), PrintMap(attributes).c_str());

        String classes;
        GetAttribute(attributes, "class", classes);
        PutClass(classes);

        if (elementName == "span")
        {
        }
        else if (elementName == "p")
        {
            needLineBreak = true;
        }
        else if (elementName == "br")
        {
            textMode = false;
            needLineBreak = true;
        }
        else if (elementName == "img")
        {
            textMode = false;

            String src;
            if (GetAttribute(attributes, "src", src))
            {
                UIControl* img = new UIControl();
                PrepareControl(img);
                UIControlBackground* bg = img->GetOrCreateComponent<UIControlBackground>();
                bg->SetSprite(FilePath(src));
                controls.emplace_back(img);
            }
        }
        if (elementName == "object")
        {
            textMode = false;
        }
    }

    void OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName) override
    {
        //Logger::Debug("OnElementEnded: '%s', '%s', '%s'", elementName.c_str(), namespaceURI.c_str(), qualifedName.c_str());

        textMode = true;
        PopClass();

        if (elementName == "p")
        {
            needLineBreak = true;
        }
    }

    void OnFoundCharacters(const String& chars) override
    {
        //Logger::Debug("OnFoundCharacters: '%s'", chars.c_str());

        if (textMode)
        {
            Vector<String> tokens;
            Split(chars, " \n", tokens);
            for (auto token : tokens)
            {
                isRtl = bidiHelper.IsRtlUTF8String(token);
                UIStaticText* text = new UIStaticText();
                PrepareControl(text);
                text->SetUtf8Text(token);
                text->SetForceBiDiSupportEnabled(true);
                controls.emplace_back(text);
            }
        }
    }

    String PrintMap(const Map<String, String>& map)
    {
        String out = "[";
        if (!map.empty())
        {
            for (const auto& pair : map)
            {
                out += pair.first + "='" + pair.second + "', ";
            }
            out.resize(out.length() - 2); //remove last ', '
        }
        out += "]";
        return out;
    }

private:
    bool textMode = true;
    bool needLineBreak = false;
    bool isRtl = false;
    UIControlPackageContext* context = nullptr;
    Vector<String> classesStack;
    Vector<RefPtr<UIControl>> controls;
    BiDiHelper bidiHelper;
};

/*******************************************************************************************************/

UIRichContentSystem::UIRichContentSystem()
{
}

UIRichContentSystem::~UIRichContentSystem()
{
}

void UIRichContentSystem::RegisterControl(UIControl* control)
{
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
        links.push_back(Link(component));
    }
}

void UIRichContentSystem::UnregisterControl(UIControl* control)
{
    UIRichContentComponent* component = control->GetComponent<UIRichContentComponent>();
    if (component)
    {
    }
}

void UIRichContentSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
        UIRichContentComponent* rich = static_cast<UIRichContentComponent*>(component);
        links.push_back(Link(rich));
    }
}

void UIRichContentSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIRichContentComponent::C_TYPE)
    {
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
    for (Link& l : links)
    {
        if (l.component->IsModified())
        {
            l.component->ResetModify();

            UIControl* root = l.component->GetControl();
            root->RemoveAllControls();

            XMLBuilder builder;
            builder.SetTopClass(l.component->GetBaseClasses());
            // builder.SetContext(root->GetPackageContext()); // TODO: need it?
            if (builder.Build("<root>" + l.component->GetUTF8Text() + "</root>"))
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
