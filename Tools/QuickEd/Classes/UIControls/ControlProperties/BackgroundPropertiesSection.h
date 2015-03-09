#ifndef __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__
#define __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

class BackgroundPropertiesSection : public PropertiesSection
{
public:
    BackgroundPropertiesSection(DAVA::UIControl *control, int bgNum, const BackgroundPropertiesSection *sourceSection);
    virtual ~BackgroundPropertiesSection();
    
    DAVA::UIControlBackground *GetBg() const;
    void CreateControlBackground();

    DAVA::String GetName() const;
    
    void AddPropertiesToNode(DAVA::YamlNode *node) const;

private:
    DAVA::UIControl *control;
    DAVA::UIControlBackground *bg;
    
    int bgNum;
};

#endif // __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__
