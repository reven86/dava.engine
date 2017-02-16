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
    ///     As field names use constants from file "TArc/Utils/CommonFieldNames.h"
    ///     ActiveContextFieldName : castable to uint64
    ///     ContextsFieldName : collection of reflected objects. Key of concrete object is ID that can be used to set ActiveTabID
    ///     each tab should have:
    ///         ContextIDFieldName : ID of tab. Castable to uint64
    ///         ContextNameFieldName : castable to DAVA::String
    ///         ContextToolTipFieldName: castable to DAVA::String [optional]
    ///         IsContextModifiedFieldName: castable to bool
    SceneTabbar(ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);
    ~SceneTabbar() = default;

    Signal<uint64> closeTab;

private:
    void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields) override;
    void OnActiveTabChanged();
    void OnTabsCollectionChanged();

    Reflection GetSceneTabsModel(const DataContext* context);
    Reflection GetTabModel(uint64 id, const DataContext* context);
    void OnCurrentTabChanged(int currentTab);
    void OnCloseTabRequest(int index);
    void OnCloseCurrentTab();

    uint64 GetIDByWrapper(const DataWrapper& wrapper);
    int GetTabIndexByID(uint64 id);

private:
    ContextAccessor* accessor;
    Reflection model;
    DataWrapper modelWrapper;
    Map<uint64, DataWrapper> tabWrappers;
};

} // namespace TArc
} // namespace DAVA