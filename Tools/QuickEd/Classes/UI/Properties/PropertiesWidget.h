#ifndef __QUICKED_PROPERTIES_WIDGET_H__
#define __QUICKED_PROPERTIES_WIDGET_H__

#include <QDockWidget>
#include "Base/BaseTypes.h"
#include "ui_PropertiesWidget.h"
#include "EditorSystems/SelectionContainer.h"

class ControlNode;
class StyleSheetNode;
class Document;
class PackageBaseNode;
class PropertiesModel;
class QtModelPackageCommandExecutor;

class PropertiesWidget : public QDockWidget, public Ui::PropertiesWidget
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget* parent = nullptr);

public slots:
    void UpdateModel(PackageBaseNode* node);
    void OnDocumentChanged(Document* doc);

    void OnAddComponent(QAction* action);
    void OnAddStyleProperty(QAction* action);
    void OnAddStyleSelector();
    void OnRemove();

    void OnSelectionChanged(const QItemSelection& selected,
                            const QItemSelection& deselected);
    void OnModelUpdated();

private slots:
    void OnExpanded(const QModelIndex& index);
    void OnCollapsed(const QModelIndex& index);

private:
    QAction* CreateAddComponentAction();
    QAction* CreateAddStyleSelectorAction();
    QAction* CreateAddStylePropertyAction();
    QAction* CreateRemoveAction();
    QAction* CreateSeparator();

    void UpdateActions();

    void ApplyExpanding();

    QAction* addComponentAction = nullptr;
    QAction* addStylePropertyAction = nullptr;
    QAction* addStyleSelectorAction = nullptr;
    QAction* removeAction = nullptr;

    PropertiesModel* propertiesModel = nullptr;

    DAVA::Map<DAVA::String, bool> itemsState;

    SelectionContainer selectionContainer;

    DAVA::String lastTopIndexPath;
    QtModelPackageCommandExecutor* commandExecutor = nullptr;
    PackageBaseNode* selectedNode = nullptr; //node used to build model
};

#endif //__QUICKED_PROPERTIES_WIDGET_H__
