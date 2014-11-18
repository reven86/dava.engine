#include "BackgroundPropertiesSection.h"

#include "ValueProperty.h"

using namespace DAVA;

BackgroundPropertiesSection::BackgroundPropertiesSection(UIControl *control, int bgNum, const BackgroundPropertiesSection *sourceSection) : control(NULL), bg(NULL), bgNum(bgNum)
{
    this->control = SafeRetain(control);
    
    bg = SafeRetain(control->GetBackgroundComponent(bgNum));
    if (bg == NULL && sourceSection != NULL && sourceSection->GetBg() != NULL)
    {
        bg = control->CreateBackgroundComponent(bgNum);
        control->SetBackgroundComponent(bgNum, bg);
    }
    
    if (bg)
    {
        const InspInfo *insp = bg->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            ValueProperty *sourceProp = sourceSection == NULL ? NULL : sourceSection->FindProperty(member);
            if (sourceProp && sourceProp->GetValue() != member->Value(bg))
                member->SetValue(bg, sourceProp->GetValue());
            
            ValueProperty *prop = new ValueProperty(bg, member);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

BackgroundPropertiesSection::~BackgroundPropertiesSection()
{
    SafeRelease(control);
    SafeRelease(bg);
}

UIControlBackground *BackgroundPropertiesSection::GetBg() const
{
    return bg;
}

void BackgroundPropertiesSection::CreateControlBackground()
{
    if (!bg)
    {
        bg = control->CreateBackgroundComponent(bgNum);
        control->SetBackgroundComponent(bgNum, bg);
        
        const InspInfo *insp = bg->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            ValueProperty *prop = new ValueProperty(bg, member);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

DAVA::String BackgroundPropertiesSection::GetName() const
{
    return control->GetBackgroundComponentName(bgNum);
}

void BackgroundPropertiesSection::AddPropertiesToNode(YamlNode *node) const
{
    if (bg)
    {
        YamlNode *mapNode = YamlNode::CreateMapNode(false);
        for (auto it = children.begin(); it != children.end(); ++it)
            (*it)->AddPropertiesToNode(mapNode);
        if (mapNode->GetCount() > 0)
            node->Add(GetName(), mapNode);
        else
            SafeRelease(mapNode);
    }
}
