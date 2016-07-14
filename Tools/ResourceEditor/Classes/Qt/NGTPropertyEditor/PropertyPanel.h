#pragma once

#include "Extensions.h"

#include "QtTools/WarningGuard/QtWarningsHandler.h"

#include <core_ui_framework/i_view.hpp>
#include <core_ui_framework/i_ui_framework.hpp>
#include <core_ui_framework/i_ui_application.hpp>
#include <core_reflection/reflected_object.hpp>

#include <memory>
#include <QObject>

namespace DAVA
{
class InspBase;
class InspInfo;
}

class SceneEditor2;
class SelectableGroup;

namespace wgt
{
class ReflectedPropertyModel;
}

class PropertyPanel : public QObject, public wgt::IViewEventListener, public EntityInjectDataExtension::Delegate
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
    DECLARE_REFLECTED

public:
    PropertyPanel();
    ~PropertyPanel();

    void Initialize(wgt::IUIFramework& uiFramework, wgt::IUIApplication& uiApplication);
    void Finalize(wgt::IUIApplication& uiApplication);

    wgt::ObjectHandle GetPropertyTree() const;
    void SetPropertyTree(const wgt::ObjectHandle& dummyTree);

    Q_SLOT void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void SetObject(const std::vector<DAVA::InspBase*>& object);

protected:
    void timerEvent(QTimerEvent* e) override;

private:
    void onFocusIn(wgt::IView* view) override;
    void onFocusOut(wgt::IView* view) override;
    void onLoaded(wgt::IView* view) override;

    void UpdateModel();

    void BeginBatch(const DAVA::String& name, DAVA::uint32 commandCount) override;
    void Exec(DAVA::Command::Pointer&& command) override;
    void EndBatch() override;

private:
    std::unique_ptr<wgt::IView> view;
    std::unique_ptr<wgt::ReflectedPropertyModel> model;

    int updateTimerId = -1;

    bool visible = false;
    bool isSelectionDirty = false;
    DAVA::Vector<DAVA::InspBase*> selectedObjects;
};
