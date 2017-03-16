#pragma once

#include "EditorSystems/SelectionContainer.h"
#include "Model/PackageHierarchy/PackageListener.h"

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <UI/UIControl.h>

#include <QtTools/Updaters/ContinuousUpdater.h>

#include <memory>
#include <QDockWidget>

namespace Ui
{
class StyleSheetInspectorWidget;
}

class Document;
namespace DAVA
{
class Any;
}

class StyleSheetInspectorWidget : public QDockWidget, PackageListener
{
    Q_OBJECT
public:
    explicit StyleSheetInspectorWidget(QWidget* parent = nullptr);
    ~StyleSheetInspectorWidget() override;

public slots:
    void OnDocumentChanged(Document* context);
    void OnSelectionChanged(const DAVA::Any& selection);

private:
    // PackageListener
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property) override;
    void StyleSheetsWereRebuilt() override;

    void Update();

    std::unique_ptr<Ui::StyleSheetInspectorWidget> ui;
    DAVA::RefPtr<DAVA::UIControl> currentControl;
    ContinuousUpdater updater;
    PackageNode* packageNode = nullptr;
};
