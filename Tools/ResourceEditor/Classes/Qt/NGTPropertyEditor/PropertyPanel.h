#pragma once

#include <core_ui_framework/i_view.hpp>
#include <core_ui_framework/i_ui_framework.hpp>
#include <core_ui_framework/i_ui_application.hpp>
#include <core_reflection/reflected_object.hpp>
#include <core_data_model/i_tree_model.hpp>

#include <memory>
#include <QObject>

namespace DAVA
{
class InspBase;
class InspInfo;
}

class SceneEditor2;
class SelectableGroup;

class PropertyPanel : public QObject, public wgt::IViewEventListener
{
    Q_OBJECT
    DECLARE_REFLECTED
public:
    PropertyPanel();
    ~PropertyPanel();

    void Initialize(wgt::IUIFramework& uiFramework, wgt::IUIApplication& uiApplication);
    void Finalize(wgt::IUIApplication& uiApplication);

    wgt::ObjectHandle GetPropertyTree() const;
    void SetPropertyTree(const wgt::ObjectHandle& dummyTree);

    Q_SLOT void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void SetObject(DAVA::InspBase* object);

private:
    void onFocusIn(wgt::IView* view) override;
    void onFocusOut(wgt::IView* view) override;
    void onLoaded(wgt::IView* view) override {}

private:
    std::unique_ptr<wgt::IView> view;
    std::shared_ptr<wgt::ITreeModel> model;

    bool visible = false;
    bool isSelectionDirty = false;
    DAVA::InspBase* selectedObject = nullptr;
};
