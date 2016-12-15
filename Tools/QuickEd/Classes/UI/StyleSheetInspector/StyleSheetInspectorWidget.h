#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "EditorSystems/SelectionContainer.h"
#include "UI/UIControl.h"
#include "Model/PackageHierarchy/PackageListener.h"

#include <memory>
#include <QDockWidget>

namespace Ui
{
class StyleSheetInspectorWidget;
}

class Document;

class StyleSheetInspectorWidget : public QDockWidget, PackageListener
{
    Q_OBJECT
public:
    explicit StyleSheetInspectorWidget(QWidget* parent = nullptr);
    ~StyleSheetInspectorWidget() override;

public slots:
    void OnDocumentChanged(Document* context);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);

private:
    // PackageListener
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property) override;
    void StyleSheetsWereRebuilt() override;

    void Update();

    std::unique_ptr<Ui::StyleSheetInspectorWidget> ui;
    DAVA::RefPtr<DAVA::UIControl> currentControl;
    PackageNode* packageNode = nullptr;
};
