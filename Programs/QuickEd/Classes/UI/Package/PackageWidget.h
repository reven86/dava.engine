#ifndef __UI_EDITOR_UI_PACKAGE_WIDGET__
#define __UI_EDITOR_UI_PACKAGE_WIDGET__

#include "EditorSystems/SelectionContainer.h"
#include "Base/BaseTypes.h"
#include "ui_PackageWidget.h"
#include <QWidget>
#include <QDockWidget>
#include <QModelIndex>
#include <QStack>
#include <QPointer>

class Document;
class ControlNode;
class StyleSheetNode;
class PackageNode;
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

    PackageModel* GetPackageModel() const;
    using ExpandedIndexes = QModelIndexList;

signals:
    void SelectedNodesChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void CurrentIndexChanged(PackageBaseNode* node);

public slots:
    void OnDocumentChanged(Document* context);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnCopy();
    void OnPaste();
    void OnCut();
    void OnDelete();
    void OnImport();

private slots:
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
    void SetSelectedNodes(const SelectedNodes& selected, const SelectedNodes& deselected);
    void CollectExpandedIndexes(PackageBaseNode* node);
    void MoveNodeUpDown(bool up);
    void MoveNodeImpl(PackageBaseNode* node, PackageBaseNode* dest, DAVA::uint32 destIndex);
    QAction* CreateAction(const QString& name, void (PackageWidget::*callback)(void), const QKeySequence& sequence = QKeySequence());
    void CreateActions();
    void PlaceActions();
    void LoadContext();
    void SaveContext();
    void RefreshActions();

    void DeselectNodeImpl(PackageBaseNode* node);
    void SelectNodeImpl(PackageBaseNode* node);
    void CollectSelectedControls(DAVA::Vector<ControlNode*>& nodes, bool forCopy, bool forRemove);
    void CollectSelectedStyles(DAVA::Vector<StyleSheetNode*>& nodes, bool forCopy, bool forRemove);
    void CollectSelectedImportedPackages(DAVA::Vector<PackageNode*>& nodes, bool forCopy, bool forRemove);
    void CopyNodesToClipboard(const DAVA::Vector<ControlNode*>& controls, const DAVA::Vector<StyleSheetNode*>& styles);

    ExpandedIndexes GetExpandedIndexes() const;
    void RestoreExpandedIndexes(const ExpandedIndexes& indexes);

    QPointer<Document> document;
    QAction* importPackageAction = nullptr;
    QAction* copyAction = nullptr;
    QAction* pasteAction = nullptr;
    QAction* cutAction = nullptr;
    QAction* delAction = nullptr;
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
};

#endif // __UI_EDITOR_UI_PACKAGE_WIDGET__
