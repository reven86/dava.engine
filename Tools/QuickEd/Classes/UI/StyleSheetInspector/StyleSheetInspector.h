#pragma once

#include "EditorSystems/SelectionContainer.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include "Base/BaseTypes.h"
#include "ui_StyleSheetInspector.h"
#include <QWidget>
#include <QDockWidget>
#include <QPointer>

class StyleSheetInspector : public QDockWidget, public Ui::StyleSheetInspector, PackageListener
{
    Q_OBJECT
public:
    explicit StyleSheetInspector(QWidget* parent = 0);

public slots:
    void OnDocumentChanged(Document* context);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);

private:
    void StyleSheetsWereRebuilt() override;

    void Update();

    DAVA::RefPtr<DAVA::UIControl> currentControl;
    PackageNode* packageNode = nullptr;
};
