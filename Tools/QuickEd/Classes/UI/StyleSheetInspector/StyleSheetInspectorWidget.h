#pragma once

#include "EditorSystems/SelectionContainer.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include "Base/BaseTypes.h"
#include "ui_StyleSheetInspectorWidget.h"
#include <QWidget>
#include <QDockWidget>
#include <QPointer>

class StyleSheetInspectorWidget : public QDockWidget, PackageListener
{
    Q_OBJECT
public:
    explicit StyleSheetInspectorWidget(QWidget* parent = nullptr);

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
