#include "CallTreeModel.h"

#include "DataFormat.h"

using namespace DAVA;

CallTreeModel::CallTreeModel(const char* filename, bool back, QObject* parent)
    : QAbstractItemModel(parent)
{
    dumpSession.LoadDump(filename);
    if (back)
        rootBranch = dumpSession.CreateTreeBack();
    else
        rootBranch = dumpSession.CreateTreeFwd();
}

CallTreeModel::CallTreeModel(const char* filename, QObject* parent)
    : QAbstractItemModel(parent)
{
    dumpSession.LoadDump(filename);
    rootBranch = dumpSession.CreateTreeFwdEx();
}

CallTreeModel::~CallTreeModel()
{
    delete rootBranch;
}

QVariant CallTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        int row = index.row();
        int clm = index.column();
        DumpSession::Branch* branch = static_cast<DumpSession::Branch*>(index.internalPointer());
        if (Qt::DisplayRole == role)
        {
            if (clm == 0)
            {
                uint64 addr = branch->Addr();
                const String& name = dumpSession.Symbols().GetSymbol(addr);

                char buf[32];
                Snprintf(buf, COUNT_OF(buf), "%08llX", addr);

                return QString("%1 %2")
                    .arg(buf)
                    .arg(name.c_str());
            }
            else if (clm == 1)
            {
                uint32 a = branch->AllocSize();
                uint32 n = branch->NBlocks();
                return QString("%1; %2")
                    .arg(FormatNumberWithDigitGroups(a).c_str())
                    .arg(n);
            }
        }
    }
    return QVariant();
}

bool CallTreeModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int CallTreeModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

int CallTreeModel::rowCount(const QModelIndex& parent) const
{
    DumpSession::Branch* branch = rootBranch;
    if (parent.isValid())
    {
        branch = static_cast<DumpSession::Branch*>(parent.internalPointer());
    }
    return branch->ChildrenCount();
}

QModelIndex CallTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent))
    {
        DumpSession::Branch* parentBranch = rootBranch;
        if (parent.isValid())
        {
            parentBranch = static_cast<DumpSession::Branch*>(parent.internalPointer());
        }
        if (parentBranch->HasChildren())
        {
            return createIndex(row, column, parentBranch->Child(row));
        }
    }
    return QModelIndex();
}

QModelIndex CallTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        DumpSession::Branch* branch = static_cast<DumpSession::Branch*>(index.internalPointer());
        if (rootBranch != branch->Parent())
        {
            return createIndex(branch->Level() - 1, 0, branch->Parent());
        }
    }
    return QModelIndex();
}
