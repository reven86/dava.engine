#ifndef __DAVAENGINE_UI_STYLESHEET_PROPERTIES_TABLE_H__
#define __DAVAENGINE_UI_STYLESHEET_PROPERTIES_TABLE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/StaticSingleton.h"
#include "FileSystem/VariantType.h"
#include "UI/Styles/UIStyleSheetStructs.h"

namespace DAVA
{
class UIStyleSheetPropertyDataBase :
public StaticSingleton<UIStyleSheetPropertyDataBase>
{
public:
    virtual ~UIStyleSheetPropertyDataBase();
    static const int32 STYLE_SHEET_PROPERTY_COUNT = 66;

    UIStyleSheetPropertyDataBase();

    uint32 GetStyleSheetPropertyIndex(const FastName& name) const;
    bool IsValidStyleSheetProperty(const FastName& name) const;
    const UIStyleSheetPropertyDescriptor& GetStyleSheetPropertyByIndex(uint32 index) const;
    int32 FindStyleSheetPropertyByMember(const InspMember* memberInfo) const;

private:
    UIStyleSheetPropertyGroup controlGroup;
    UIStyleSheetPropertyGroup bgGroup;
    UIStyleSheetPropertyGroup staticTextGroup;
    UIStyleSheetPropertyGroup textFieldGroup;

    UIStyleSheetPropertyGroup linearLayoutGroup;
    UIStyleSheetPropertyGroup flowLayoutGroup;
    UIStyleSheetPropertyGroup flowLayoutHintGroup;
    UIStyleSheetPropertyGroup ignoreLayoutGroup;
    UIStyleSheetPropertyGroup sizePolicyGroup;
    UIStyleSheetPropertyGroup anchorGroup;

    Array<UIStyleSheetPropertyDescriptor, STYLE_SHEET_PROPERTY_COUNT> properties; // have to be after groups declaration

    UnorderedMap<FastName, uint32> propertyNameToIndexMap;
};

typedef Bitset<UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT> UIStyleSheetPropertySet;
};


#endif
