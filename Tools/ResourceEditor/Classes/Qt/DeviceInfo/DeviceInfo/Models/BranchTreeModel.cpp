#include <QColor>

#include "BranchTreeModel.h"
#include "DataFormat.h"

using namespace DAVA;

BranchTreeModel::BranchTreeModel(const MemoryDumpSession& session, bool diff, QObject* parent)
    : QAbstractItemModel(parent)
    , dumpSession(session)
    , rootBranch(new Branch(nullptr))
    , diffModel(diff)
    , ncolumns(diff ? 2 : 1)
{

}

BranchTreeModel::~BranchTreeModel()
{}

void BranchTreeModel::PrepareModel(const char* fromName)
{
    Q_ASSERT(fromName != nullptr);
    Q_ASSERT(!diffModel);

    beginResetModel();
    BuildTree(fromName);
    endResetModel();
}

void BranchTreeModel::PrepareDiffModel(const char* fromName)
{
    Q_ASSERT(fromName != nullptr);
    Q_ASSERT(diffModel);

    beginResetModel();
    BuildTree(fromName);
    endResetModel();
}

void BranchTreeModel::PrepareModel(const DAVA::Vector<const char*>& names)
{
    Q_ASSERT(!diffModel);

    beginResetModel();
    BuildTree(names);
    endResetModel();
}

void BranchTreeModel::PrepareDiffModel(const DAVA::Vector<const char*>& names)
{
    Q_ASSERT(diffModel);

    beginResetModel();
    BuildTree(names);
    endResetModel();
}

QModelIndex BranchTreeModel::Find(Branch* p, const QModelIndex& par) const
{
    int nrows = rowCount(par);
    for (int i = 0;i < nrows;++i)
    {
        QModelIndex tmp = index(i, 0, par);
        Branch* x = (Branch*)tmp.internalPointer();
        if (x == p)
            return tmp;
        tmp = Find(p, tmp);
        if (tmp.isValid())
            return tmp;
    }
    return QModelIndex();
}

void BranchTreeModel::Find(Branch* p, const QModelIndex& par, DAVA::Vector<QModelIndex>& v) const
{
    int nrows = rowCount(par);
    for (int i = 0;i < nrows;++i)
    {
        QModelIndex tmp = index(i, 0, par);
        Branch* x = (Branch*)tmp.internalPointer();
        if (x == p)
        {
            v.push_back(tmp);
            return ;
        }
        Find(p, tmp, v);
        if (tmp.isValid())
        {
            v.push_back(tmp);
            return ;
        }
    }
}

DAVA::Vector<QModelIndex> BranchTreeModel::Select2(const DAVA::MMBlock* block) const
{
    Vector<QModelIndex> v;

    auto& frames = dumpSession.SymbolTable().GetFrames(block->bktraceHash);
    Q_ASSERT(!frames.empty());

    QModelIndex idx = QModelIndex();
    int ich = 0;
    for (auto ch : rootBranch->childBranches)
    {
        auto iter = std::find(frames.begin(), frames.end(), ch->name);
        if (iter == frames.end())
            continue;
        idx = index(ich, 0, idx);
        v.push_back(idx);
        size_t startFrame = iter - frames.begin();
        while (startFrame > 0)
        {
            startFrame -= 1;
            const char8* curName = frames[startFrame];
            Branch* p = ch->FindInChildren(curName);
            if (p == nullptr)
                break;
            //int off = int(p - ch->childBranches[0]);
            int off = ch->ChildIndex(p);
            idx = index(off, 0, idx);
            v.push_back(idx);
            ch = p;
        }
        ich += 1;
    }

    /*Branch* p = dumpSession.FindPath(rootBranch, frames);
    if (p != nullptr)
    {
        Find(p, QModelIndex(), v);
    }*/
    return v;
}

QModelIndex BranchTreeModel::Select(const DAVA::MMBlock* block) const
{
    Q_ASSERT(block != nullptr);

    //QModelIndex i = index(0, 0, QModelIndex());
    //i = index(0, 0, i);
    //i = index(0, 0, i);
    //return i;
    auto& frames = dumpSession.SymbolTable().GetFrames(block->bktraceHash);
    Q_ASSERT(!frames.empty());

    Branch* p = dumpSession.FindPath(rootBranch, frames);
    /*if (p != nullptr)
    {
        return Find(p, QModelIndex());
    }*/
    if (p != nullptr)
    {
        return createIndex(p->level, 0, p);
        //return createIndex(0, 0, p);
    }
    return QModelIndex();
}

QVariant BranchTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        Branch* branch = static_cast<Branch*>(index.internalPointer());
        if (Qt::DisplayRole == role)
        {
            if (branch->name != nullptr)
            {
                if (diffModel)
                    return dataDiff(branch, index.row(), index.column());
                else
                    return dataBranch(branch, index.row(), index.column());
            }
            else
                return QVariant("Root");
        }
        else if (Qt::BackgroundColorRole == role)
        {
            return colorDiff(branch, index.row(), index.column());
        }
    }
    return QVariant();
}

QVariant BranchTreeModel::dataBranch(Branch* branch, int row, int clm) const
{
    return QString("[%1, %2] %3")
        .arg(FormatNumberWithDigitGroups(branch->allocByApp[0]).c_str())
        .arg(branch->blockCount[0])
        .arg(branch->name);
    //return QVariant(branch->name);
}

QVariant BranchTreeModel::dataDiff(Branch* branch, int row, int clm) const
{
    if (branch->diff[clm])
    {
        return QString("[%1, %2] %3")
            .arg(FormatNumberWithDigitGroups(branch->allocByApp[clm]).c_str())
            .arg(branch->blockCount[clm])
            .arg(branch->name);
        return QVariant(branch->name);
    }
    return QVariant();
}

QVariant BranchTreeModel::colorDiff(Branch* branch, int row, int clm) const
{
    /*if (branch->diff[0] != branch->diff[1])
    {
        if (branch->diff[clm] != 0)
            return QColor(255, 128, 128);
    }*/
    if (branch->diff[clm] == 0)
        return QColor(255, 128, 128);
    return QVariant();
}

bool BranchTreeModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int BranchTreeModel::columnCount(const QModelIndex& parent) const
{
    return ncolumns;
}

int BranchTreeModel::rowCount(const QModelIndex& parent) const
{
    Branch* branch = rootBranch;
    if (parent.isValid())
    {
        branch = static_cast<Branch*>(parent.internalPointer());
    }
    return static_cast<int>(branch->childBranches.size());
}

QModelIndex BranchTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent))
    {
        Branch* parentBranch = rootBranch;
        if (parent.isValid())
        {
            parentBranch = static_cast<Branch*>(parent.internalPointer());
        }
        if (!parentBranch->childBranches.empty())
        {
            return createIndex(row, column, parentBranch->childBranches[row]);
        }
    }
    return QModelIndex();
}

QModelIndex BranchTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        Branch* branch = static_cast<Branch*>(index.internalPointer());
        if (rootBranch != branch->parent)
        {
            //return createIndex(node->Index(), 0, branch->parent);
            return createIndex(branch->level - 1, 0, branch->parent);
        }
    }
    return QModelIndex();
}

void BranchTreeModel::BuildTree(const char* fromName)
{
    if (rootBranch != nullptr)
    {
        delete rootBranch;
    }
    if (diffModel)
        rootBranch = dumpSession.CreateBranchDiff(fromName);
    else
        rootBranch = dumpSession.CreateBranch(fromName, 0);
}

void BranchTreeModel::BuildTree(const DAVA::Vector<const char*>& names)
{
    if (rootBranch != nullptr)
    {
        delete rootBranch;
    }
    if (diffModel)
        rootBranch = dumpSession.CreateBranchDiff(names);
    else
        rootBranch = dumpSession.CreateBranch(names, 0);
}
