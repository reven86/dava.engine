#pragma once

#include "Base/BaseTypes.h"

#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataListener.h"

#include "Reflection/Reflection.h"

#include <QTabBar>

namespace DAVA
{
namespace TArc
{
class SceneTabbar : public QTabBar, private DataListener
{
public:
    /// Model structure:
    ///     "ActiveTabID" : castable to uint64
    ///     "Tabs" : collection of reflected objects. Key of concrete object is ID that can be used to set ActiveTabID
    ///     each tab should have:
    ///         "Title": castable to DAVA::String
    ///         "Tooltip": castable to DAVA::String [optional]
    SceneTabbar(ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);
    ~SceneTabbar() = default;

    Signal<uint64> closeTab;
    static const char* activeTabPropertyName;
    static const char* tabsPropertyName;
    static const char* tabTitlePropertyName;
    static const char* tabTooltipPropertyName;

private:
    void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields) override;
    void OnActiveTabChanged();
    void OnTabsCollectionChanged();

    Reflection GetSceneTabsModel(const DataContext* context);
    void OnCurrentTabChanged(int currentTab);
    void OnCloseTabRequest(int index);
    void OnCloseCurrentTab();

private:
    Reflection model;
    DataWrapper modelWrapperWrapper;

    bool inTabChanging = false;
};

} // namespace TArc
} // namespace DAVA