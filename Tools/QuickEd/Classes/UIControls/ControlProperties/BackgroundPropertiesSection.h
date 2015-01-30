#ifndef __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__
#define __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

namespace DAVA
{
    class UIControl;
    class UIControlBackground;
}

class BackgroundPropertiesSection : public PropertiesSection
{
public:
    BackgroundPropertiesSection(DAVA::UIControl *control, int bgNum, const BackgroundPropertiesSection *sourceSection, eCopyType copyType);
    virtual ~BackgroundPropertiesSection();
    
    DAVA::UIControlBackground *GetBg() const;
    void CreateControlBackground();

    DAVA::String GetName() const;
    
    virtual bool HasChanges() const override;
    virtual void Serialize(PackageSerializer *serializer) const override;

private:
    DAVA::UIControl *control;
    DAVA::UIControlBackground *bg;
    
    int bgNum;
};

#endif // __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__
