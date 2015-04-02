#include <QColor>

#include "FileSystem/Logger.h"

#include "BlockListModel.h"

using namespace DAVA;

BlockListModel::BlockListModel(bool diff, QObject* parent)
    : QAbstractItemModel(parent)
    , diffModel(diff)
    , ncolumns(diff ? 2 : 3)
{
    this->diff.ntotal = 0;
}

BlockListModel::~BlockListModel()
{

}

void BlockListModel::PrepareModel(DAVA::Vector<const DAVA::MMBlock*>& v1)
{
    Q_ASSERT(!diffModel);
    beginResetModel();

    v.clear();
    v.swap(v1);
    endResetModel();
}

template<typename Cmp, typename Act>
void extract(Vector<const MMBlock*>& v1, Vector<const MMBlock*>& v2, /*Vector<const MMBlock*>& dst,*/ Cmp cmp, Act act)
{
    size_t i1 = 0;
    size_t i2 = 0;
    size_t n1 = v1.size();
    size_t n2 = v2.size();

    while (i1 < n1 && i2 < n2)
    {
        //if (v1[i1]->orderNo < v2[i2]->orderNo)
        if (cmp(v1[i1], v2[i2]))
        {
            i1 += 1;
        }
        else
        {
            //if (v1[i1]->orderNo == v2[i2]->orderNo)
            if (!cmp(v1[i1], v2[i2]))
            {
                //dst.push_back(v1[i1]);
                act(v1[i1], v2[i2]);
                v1[i1] = nullptr;
                v2[i2] = nullptr;
                i1 += 1;
            }
            i2 += 1;
        }
    }

    auto rm = [](const MMBlock* p) -> bool { return p == nullptr; };
    v1.erase(std::remove_if(v1.begin(), v1.end(), rm), v1.end());
    v2.erase(std::remove_if(v2.begin(), v2.end(), rm), v2.end());
}

void BlockListModel::PrepareDiffModel(bool cmpCallstack, DAVA::Vector<const DAVA::MMBlock*>& v1, DAVA::Vector<const DAVA::MMBlock*>& v2)
{
    Q_ASSERT(diffModel);
    beginResetModel();

    diff.ntotal = 0;
    for (auto& x : diff.v)
        x.clear();
    //diff.same.clear();
    //diff.same2.clear();
    //diff.left.clear();
    //diff.right.clear();

    auto debug = [&v1, &v2](const char* s)->void {
        Logger::Debug("%s", s);
        Logger::Debug("v1");
        for (auto x : v1)
            Logger::Debug("%08X %5u %10u", x->bktraceHash, x->allocByApp, x->orderNo);
        Logger::Debug("v2");
        for (auto x : v2)
            Logger::Debug("%08X %5u %10u", x->bktraceHash, x->allocByApp, x->orderNo);
    };

    //debug("initial");

    {
        auto less_order = [](const MMBlock* l, const MMBlock* r) -> bool {
            return l->orderNo < r->orderNo;
        };
        auto act = [this](const MMBlock* l, const MMBlock* r) -> void {
            diff.v[DiffStruct::D_SAME].push_back(std::make_pair(l, r));
        };
        std::sort(v1.begin(), v1.end(), less_order);
        std::sort(v2.begin(), v2.end(), less_order);
        extract(v1, v2, /*diff.same,*/ less_order, act);

        //debug("after remove same");
    }
    {
        auto less_bk = [](const MMBlock* l, const MMBlock* r) -> bool {
            return l->bktraceHash < r->bktraceHash;
        };
        auto act = [this](const MMBlock* l, const MMBlock* r) -> void {
            diff.v[DiffStruct::D_HASH].push_back(std::make_pair(l, r));
        };
        std::sort(v1.begin(), v1.end(), less_bk);
        std::sort(v2.begin(), v2.end(), less_bk);
        extract(v1, v2, /*diff.same2,*/ less_bk, act);

        //debug("after remove same backtrace");
    }

    {
        size_t nmax = std::max(v1.size(), v2.size());
        diff.v[DiffStruct::D_DIFF].resize(nmax);
        for (size_t i = 0;i < nmax;++i)
        {
            if (i < v1.size())
                diff.v[DiffStruct::D_DIFF][i].first = v1[i];
            if (i < v2.size())
                diff.v[DiffStruct::D_DIFF][i].second = v2[i];
        }
    }

    diff.r[DiffStruct::D_SAME] = std::make_pair(0, (int)diff.v[DiffStruct::D_SAME].size());
    diff.r[DiffStruct::D_HASH] = std::make_pair(diff.r[DiffStruct::D_SAME].second, diff.r[DiffStruct::D_SAME].second + (int)diff.v[DiffStruct::D_HASH].size());
    diff.r[DiffStruct::D_DIFF] = std::make_pair(diff.r[DiffStruct::D_HASH].second, diff.r[DiffStruct::D_HASH].second + (int)diff.v[DiffStruct::D_DIFF].size());
    diff.ntotal = int(diff.v[DiffStruct::D_SAME].size() + diff.v[DiffStruct::D_HASH].size() + diff.v[DiffStruct::D_DIFF].size());

    /*Logger::Debug("Diff vectors");
    Logger::Debug("same");
    for (auto x : diff.v[DiffStruct::D_SAME])
        Logger::Debug("%08X %5u %10u", x.first->backtraceHash, x.first->allocByApp, x.first->orderNo);
    Logger::Debug("same2");
    for (auto x : diff.same2)
        Logger::Debug("%08X %5u %10u", x->backtraceHash, x->allocByApp, x->orderNo);
    Logger::Debug("left");
    for (auto x : diff.left)
        Logger::Debug("%08X %5u %10u", x->backtraceHash, x->allocByApp, x->orderNo);
    Logger::Debug("rigth");
    for (auto x : diff.right)
        Logger::Debug("%08X %5u %10u", x->backtraceHash, x->allocByApp, x->orderNo);*/

    endResetModel();
}

const DAVA::MMBlock* BlockListModel::GetBlock(const QModelIndex& index) const
{
    if (index.isValid())
    {
        if (diffModel)
        {
            int row = index.row();
            int clm = index.column();
            int r = diff.in_range(row);
            int off = diff.r[r].first;
            Q_ASSERT(r >= 0);

            DiffStruct::pair p = diff.v[r][row - off];
            const DAVA::MMBlock* arr[2] = {
                p.first,
                p.second
            };
            if (arr[clm])
                return arr[clm];
            if (clm == 0)
                clm = 1;
            else
                clm = 0;
            return arr[clm];
        }
        else
        {
            size_t row = index.row();
            if (row < v.size())
                return v[row];
        }
    }
    return nullptr;
}

QVariant BlockListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        char buf[32];
        int row = index.row();
        int clm = index.column();
        if (Qt::DisplayRole == role)
        {
            if (diffModel)
            {
                int r = diff.in_range(row);
                int off = diff.r[r].first;
                Q_ASSERT(r >= 0);
                const MMBlock* p = nullptr;
                if (clm == 0)
                    p = diff.v[r][row - off].first;
                else
                    p = diff.v[r][row - off].second;
                if (p != nullptr)
                {
                    Snprintf(buf, COUNT_OF(buf), "%08X", p->bktraceHash);
                    return QString("hash=%1, size=%2, order=%3")
                        .arg(buf)
                        .arg(p->allocByApp)
                        .arg(p->orderNo);
                }
            }
            else
            {
                const MMBlock* block = v[row];
                switch (clm)
                {
                case 0:
                    return QVariant(row + 1);
                case 1:
                    return QString("%1").arg(block->orderNo);
                case 2:
                    return QString("%1").arg(block->allocByApp);
                }
            }
        }
        else if (Qt::BackgroundColorRole == role && diffModel)
        {
            int r = diff.in_range(row);
            int off = diff.r[r].first;
            Q_ASSERT(r >= 0);
            const MMBlock* p = nullptr;
            if (clm == 0)
                p = diff.v[r][row - off].first;
            else
                p = diff.v[r][row - off].second;

            if (p != nullptr)
            {
                if (r == DiffStruct::D_SAME)
                {

                }
                else if (r == DiffStruct::D_HASH)
                {
                    return QColor(128, 128, 0);
                }
                else if (r == DiffStruct::D_DIFF)
                {
                    if (clm == 0)
                        return QColor(255, 128, 128);
                    return QColor(128, 255, 128);
                }
            }
        }
    }
    return QVariant();
}

int BlockListModel::columnCount(const QModelIndex& parent) const
{
    return ncolumns;
}

bool BlockListModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int BlockListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        if (diffModel)
        {
            return diff.ntotal;
        }
        else
            return static_cast<int>(v.size());
    }
    return 0;
}

QModelIndex BlockListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent))
    {
        return createIndex(row, column, nullptr);
    }
    return QModelIndex();
}

QModelIndex BlockListModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}
