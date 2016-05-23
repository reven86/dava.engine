#ifndef __RESOURCEEDITOR_PROPERTYPANEL_H__
#define __RESOURCEEDITOR_PROPERTYPANEL_H__

#include "core_ui_framework/i_view.hpp"
#include "core_ui_framework/i_ui_framework.hpp"
#include "core_ui_framework/i_ui_application.hpp"
#include "core_reflection/reflected_object.hpp"
#include "core_data_model/i_tree_model.hpp"

#include <memory>
#include <QObject>

namespace DAVA
{
class InspBase;
class InspInfo;
}

class SceneEditor2;
class SelectableGroup;

class PropertyPanel : public QObject, public IViewEventListener
{
    Q_OBJECT
    DECLARE_REFLECTED
public:
    PropertyPanel();
    ~PropertyPanel();

    void Initialize(IUIFramework& uiFramework, IUIApplication& uiApplication);
    void Finalize();

    ObjectHandle GetPropertyTree() const;
    void SetPropertyTree(const ObjectHandle& dummyTree);

    Q_SLOT void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void SetObject(DAVA::InspBase* object);

private:
    void onFocusIn(IView* view) override;
    void onFocusOut(IView* view) override;

private:
    std::unique_ptr<IView> view;
    std::shared_ptr<ITreeModel> model;

    bool visible = false;
    bool isSelectionDirty = false;
    DAVA::InspBase* selectedObject = nullptr;
};

#endif // __RESOURCEEDITOR_PROPERTYPANEL_H__