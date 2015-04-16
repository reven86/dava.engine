#ifndef __DIFFVIEWERWIDGET_H__
#define __DIFFVIEWERWIDGET_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include <QWidget>

class QTabWidget;
class QTreeView;
class QListView;

class SymbolsTreeModel;
class SymbolsFilterModel;

class BranchDiffTreeModel;
class BlockListModel;

class MemoryDump;
class DumpBrief;

class DiffViewerWidget : public QWidget
{
    Q_OBJECT

public:
    DiffViewerWidget(const DumpBrief& brief1, const DumpBrief& brief2, QWidget* parent = nullptr);
    virtual ~DiffViewerWidget();

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
    const DumpBrief& dumpBrief1;
    const DumpBrief& dumpBrief2;
    MemoryDump* memoryDump1;
    MemoryDump* memoryDump2;

    SymbolsTreeModel* symbolsTreeModel = nullptr;
    SymbolsFilterModel* symbolsFilterModel = nullptr;

    BranchDiffTreeModel* branchTreeModel = nullptr;
    BlockListModel* blockListModel1 = nullptr;
    BlockListModel* blockListModel2 = nullptr;

    QTabWidget* tab = nullptr;
    QTreeView* symbolsTree = nullptr;
    QTreeView* branchTree = nullptr;
    QListView* blockList1 = nullptr;
    QListView* blockList2 = nullptr;
};

#endif  // __DIFFVIEWERWIDGET_H__
