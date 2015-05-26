#ifndef __DUMPVIEWERWIDGET_H__
#define __DUMPVIEWERWIDGET_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include <QWidget>

class QTabWidget;
class QTreeView;
class QListView;

class SymbolsTreeModel;
class SymbolsFilterModel;

class BranchTreeModel;
class BlockListModel;

class MemoryDump;
class MemorySnapshot;

class DumpViewerWidget : public QWidget
{
    Q_OBJECT

public:
    DumpViewerWidget(const MemorySnapshot& brief, QWidget* parent = nullptr);
    virtual ~DumpViewerWidget();

public slots:
    void SymbolView_OnBuldTree();

    void BranchView_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void BranchBlockView_DoubleClicked(const QModelIndex& current);

private:
    void Init();
    void InitSymbolsView();
    void InitBranchView();

    DAVA::Vector<const char*> GetSelectedSymbols();

private:
    const MemorySnapshot& dumpBrief;
    MemoryDump* memoryDump;

    SymbolsTreeModel* symbolsTreeModel = nullptr;
    SymbolsFilterModel* symbolsFilterModel = nullptr;

    BranchTreeModel* branchTreeModel = nullptr;
    BlockListModel* blockListModel = nullptr;

    QTabWidget* tab = nullptr;
    QTreeView* symbolsTree = nullptr;
    QTreeView* branchTree = nullptr;
    QListView* blockList = nullptr;
};

#endif  // __DUMPVIEWERWIDGET_H__
