#pragma once


#include "Base/BaseTypes.h"

#include "TArc/DataProcessing/DataNode.h"
#include "TArc/Controls/SceneTabbar.h"

struct TabDescriptor
{
    DAVA::String tabTitle;
    DAVA::String tabTooltip;

    bool operator==(const TabDescriptor& other) const;

private:
    DAVA_REFLECTION(TabDescriptor)
    {
        DAVA::ReflectionRegistrator<TabDescriptor>::Begin()
        .Field(DAVA::TArc::SceneTabbar::tabTitlePropertyName, &TabDescriptor::tabTitle)
        .Field(DAVA::TArc::SceneTabbar::tabTooltipPropertyName, &TabDescriptor::tabTooltip)
        .End();
    }
};

class SceneTabsModel : public DAVA::TArc::DataNode
{
public:
    DAVA::uint64 activeContexID = 0;

    using TTabsCollection = DAVA::Map<DAVA::uint64, TabDescriptor>;
    TTabsCollection tabs;

private:
    DAVA_VIRTUAL_REFLECTION(SceneTabsModel, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SceneTabsModel>::Begin()
        .Field(DAVA::TArc::SceneTabbar::activeTabPropertyName, &SceneTabsModel::activeContexID)
        .Field(DAVA::TArc::SceneTabbar::tabsPropertyName, &SceneTabsModel::tabs)
        .End();
    }
};

inline bool TabDescriptor::operator==(const TabDescriptor& other) const
{
    return tabTitle == other.tabTitle &&
    tabTooltip == other.tabTooltip;
}

namespace DAVA
{
template <>
struct AnyCompare<TabDescriptor>
{
    static bool IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
    {
        const TabDescriptor& tab1 = v1.Get<TabDescriptor>();
        const TabDescriptor& tab2 = v2.Get<TabDescriptor>();
        return tab1 == tab2;
    }
};

template <>
struct AnyCompare<SceneTabsModel::TTabsCollection>
{
    static bool IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
    {
        const SceneTabsModel::TTabsCollection& tabs1 = v1.Get<SceneTabsModel::TTabsCollection>();
        const SceneTabsModel::TTabsCollection& tabs2 = v2.Get<SceneTabsModel::TTabsCollection>();
        return tabs1 == tabs2;
    }
};
}