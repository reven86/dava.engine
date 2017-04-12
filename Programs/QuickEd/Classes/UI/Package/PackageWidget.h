#pragma once

#include "ui_PackageWidget.h"

#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataWrapper.h>

#include <Base/BaseTypes.h>

#include <QWidget>
#include <QDockWidget>
#include <QModelIndex>
#include <QStack>
#include <QPointer>

namespace DAVA
{
class Any;
namespace TArc
{
class ContextAccessor;
}
}

struct PackageContext;
class ControlNode;
class StyleSheetNode;
class PackageBaseNode;
class FilteredPackageModel;
class PackageModel;
class PackageNode;
class QItemSelection;
class QtModelPackageCommandExecutor;

class PackageWidget : public QDockWidget, public Ui::PackageWidget
{
    Q_OBJECT
public:
    explicit PackageWidget(QWidget* parent = 0);
    ~PackageWidget();

    void SetAccessor(DAVA::TArc::ContextAccessor* accessor);

    PackageModel* GetPackageModel() const;
    using ExpandedIndexes = QModelIndexList;

    void OnSelectionChanged(const DAVA::Any& selection);
    void OnPackageChanged(PackageContext* context, PackageNode* node);

signals:
    void SelectedNodesChanged(const SelectedNodes& selection);
    void CurrentIndexChanged(PackageBaseNode* package);

public slots:
    void OnCopy();
    void OnPaste();
    void OnCut();
    void OnDelete();
    void OnDuplicate();
    void OnImport();

    void OnSelectionChangedFromView(const QItemSelection& proxySelected, const QItemSelection& proxyDeselected);
    void OnFilterTextChanged(const QString&);
    void OnRename();
    void OnAddStyle();
    void OnCopyControlPath();
    void OnMoveUp();
    void OnMoveDown();
    void OnMoveLeft();
    void OnMoveRight();
    void OnBeforeProcessNodes(const SelectedNodes& nodes);
    void OnAfterProcessNodes(const SelectedNodes& nodes);
    void OnCurrentIndexChanged(const QModelIndex& index, const QModelIndex& previous);

private:
    void SetSelectedNodes(const SelectedNodes& selection);
    void CollectExpandedIndexes(PackageBaseNode* node);
    void MoveNodeUpDown(bool up);
    void MoveNodeImpl(PackageBaseNode* node, PackageBaseNode* dest, DAVA::uint32 destIndex);
    QAction* CreateAction(const QString& name, void (PackageWidget::*callback)(void), const QKeySequence& sequence = QKeySequence());
    void CreateActions();
    void PlaceActions();
    void RefreshActions();
    void LoadContext();
    void SaveContext();
    void Paste(PackageBaseNode* target, int index);

    void DeselectNodeImpl(PackageBaseNode* node);
    void SelectNodeImpl(PackageBaseNode* node);
    void CollectSelectedControls(DAVA::Vector<ControlNode*>& nodes, bool forCopy, bool forRemove);
    void CollectSelectedStyles(DAVA::Vector<StyleSheetNode*>& nodes, bool forCopy, bool forRemove);
    void CollectSelectedImportedPackages(DAVA::Vector<PackageNode*>& nodes, bool forCopy, bool forRemove);
    void CopyNodesToClipboard(const DAVA::Vector<ControlNode*>& controls, const DAVA::Vector<StyleSheetNode*>& styles);

    ExpandedIndexes GetExpandedIndexes() const;
    void RestoreExpandedIndexes(const ExpandedIndexes& indexes);

    QAction* importPackageAction = nullptr;
    QAction* copyAction = nullptr;
    QAction* pasteAction = nullptr;
    QAction* cutAction = nullptr;
    QAction* delAction = nullptr;
    QAction* duplicateControlsAction = nullptr;

    QAction* renameAction = nullptr;
    QAction* addStyleAction = nullptr;
    QAction* copyControlPathAction = nullptr;

    QAction* moveUpAction = nullptr;
    QAction* moveDownAction = nullptr;
    QAction* moveLeftAction = nullptr;
    QAction* moveRightAction = nullptr;

    FilteredPackageModel* filteredPackageModel = nullptr;
    PackageModel* packageModel = nullptr;

    SelectionContainer selectionContainer;
    SelectedNodes expandedNodes;
    //source indexes
    std::list<QPersistentModelIndex> currentIndexes;
    bool lastFilterTextEmpty = true;
    PackageContext* currentContext = nullptr;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::DataWrapper dataWrapper;
};

struct PackageContext
{
    PackageWidget::ExpandedIndexes expandedIndexes;
    QString filterString;
};
