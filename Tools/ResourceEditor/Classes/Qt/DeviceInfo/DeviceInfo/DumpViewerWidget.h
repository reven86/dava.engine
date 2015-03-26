#ifndef __DUMPVIEWERWIDGET_H__
#define __DUMPVIEWERWIDGET_H__

#include <QWidget>

#include "MemoryManager/MemoryManagerTypes.h"

#include "MemoryDumpSession.h"

class QTabWidget;
class QTreeView;
class QListView;

class SymbolsTreeModel;
class SymbolsFilterModel;

class BranchTreeModel;
class BlockListModel;

class DumpViewerWidget : public QWidget
{
    Q_OBJECT

public:
    DumpViewerWidget(const char* filename, QWidget* parent = nullptr);
    virtual ~DumpViewerWidget();

public slots:
    void SymbolView_OnBuldTree();
    void SymbolView_OnBuldDiff();
    void SymbolView_OnLoadAnotherDump();

    void BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void DiffView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void BranchBlockView_DoubleClicked(const QModelIndex& current);
    void DiffBlockView_DoubleClicked(const QModelIndex& current);

private:
    void Init();
    void InitSymbolsView();
    void InitBranchView();
    void InitDiffView();

    DAVA::Vector<const char*> GetSelectedSymbols();

private:
    MemoryDumpSession dumpSession;

    SymbolsTreeModel* symbolsTreeModel;
    SymbolsFilterModel* symbolsFilterModel;

    BranchTreeModel* branchTreeModel;
    BlockListModel* blockListModel;
    BranchTreeModel* diffTreeModel;
    BlockListModel* blockDiffListModel;

    QTabWidget* tab;
    QTreeView* symbolsTree;
    QTreeView* branchTree;
    //QListView* blockList;
    QTreeView* blockList;
    QTreeView* diffTree;
    //QListView* diffList;
    QTreeView* diffList;
};

#endif  // __DUMPVIEWERWIDGET_H__
