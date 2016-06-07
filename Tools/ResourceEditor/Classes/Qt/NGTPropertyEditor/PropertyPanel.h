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
class ReflectedPropertyModel;

class PropertyPanel : public QObject, public IViewEventListener, public EntityInjectDataExtension::Delegate
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
    DECLARE_REFLECTED

public:
    PropertyPanel();
    ~PropertyPanel();

    void Initialize(IUIFramework& uiFramework, IUIApplication& uiApplication);
    void Finalize(IUIApplication& uiApplication);

    ObjectHandle GetPropertyTree() const;
    void SetPropertyTree(const ObjectHandle& dummyTree);

    Q_SLOT void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void SetObject(const std::vector<DAVA::InspBase*>& object);

protected:
    void timerEvent(QTimerEvent* e) override;

private:
    void onFocusIn(IView* view) override;
    void onFocusOut(IView* view) override;

    void UpdateModel();

    void StartBatch(const DAVA::String& name, DAVA::uint32 commandCount) override;
    void Exec(Command2::Pointer&& command) override;
    void EndBatch() override;

private:
    std::unique_ptr<IView> view;
    std::unique_ptr<ReflectedPropertyModel> model;

    int updateTimerId = -1;

    bool visible = false;
    bool isSelectionDirty = false;
    std::vector<DAVA::InspBase*> selectedObjects;
};
