#pragma once

#include <QDockWidget>
#include "Base/BaseTypes.h"
#include "ui_PropertiesWidget.h"
#include "EditorSystems/SelectionContainer.h"

namespace DAVA
{
namespace TArc
{
class FieldBinder;
class ContextAccessor;
}
}

class Project;
class PackageNode;
class PackageBaseNode;
class PropertiesModel;
class PropertiesTreeItemDelegate;

class PropertiesWidget : public QDockWidget, public Ui::PropertiesWidget
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget* parent = nullptr);
    ~PropertiesWidget();

    void SetAccessor(DAVA::TArc::ContextAccessor* accessor);

public slots:
    void SetProject(const Project* project);
    void UpdateModel(PackageBaseNode* node);

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
    void OnComponentAdded(const QModelIndex& index);

private:
    QAction* CreateAddComponentAction();
    QAction* CreateAddStyleSelectorAction();
    QAction* CreateAddStylePropertyAction();
    QAction* CreateRemoveAction();
    QAction* CreateSeparator();

    void UpdateActions();

    void ApplyExpanding();

    void OnPackageChanged(const DAVA::Any& package);

    void BindFields();

    QAction* addComponentAction = nullptr;
    QAction* addStylePropertyAction = nullptr;
    QAction* addStyleSelectorAction = nullptr;
    QAction* removeAction = nullptr;

    PropertiesModel* propertiesModel = nullptr;
    PropertiesTreeItemDelegate* propertiesItemsDelegate = nullptr;

    DAVA::Map<DAVA::String, bool> itemsState;

    SelectionContainer selectionContainer;

    DAVA::String lastTopIndexPath;
    PackageBaseNode* selectedNode = nullptr; //node used to build model

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
