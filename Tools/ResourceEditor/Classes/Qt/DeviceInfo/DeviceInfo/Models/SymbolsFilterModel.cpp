#include "SymbolsFilterModel.h"
#include "SymbolsTreeModel.h"

using namespace DAVA;

SymbolsFilterModel::SymbolsFilterModel()
    : hideStd(false)
{

}

void SymbolsFilterModel::SetFilterString(const QString& filterString)
{
    String s = filterString.toStdString();
    if (s != filter)
    {
        filter.swap(s);
        invalidateFilter();
    }
}

void SymbolsFilterModel::ToggleStd()
{
    hideStd = !hideStd;
    invalidateFilter();
}

bool SymbolsFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    GenericTreeNode* pl = static_cast<GenericTreeNode*>(left.internalPointer());
    GenericTreeNode* pr = static_cast<GenericTreeNode*>(right.internalPointer());
    int ltype = pl->Type();
    int rtype = pr->Type();
    Q_ASSERT(ltype == rtype);

    if (ltype == SymbolsTreeModel::TYPE_ADDR)
    {
        SymbolsTreeModel::AddrNode* l = static_cast<SymbolsTreeModel::AddrNode*>(pl);
        SymbolsTreeModel::AddrNode* r = static_cast<SymbolsTreeModel::AddrNode*>(pr);

        return l->Address() < r->Address();
    }
    else if (ltype == SymbolsTreeModel::TYPE_NAME)
    {
        SymbolsTreeModel::NameNode* l = static_cast<SymbolsTreeModel::NameNode*>(pl);
        SymbolsTreeModel::NameNode* r = static_cast<SymbolsTreeModel::NameNode*>(pr);

        return l->Name() < r->Name();
    }
    return false;
}

bool SymbolsFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (hideStd)
    {
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        GenericTreeNode* p = static_cast<GenericTreeNode*>(index.internalPointer());
        int type = p->Type();
        if (p->Type() == SymbolsTreeModel::TYPE_NAME)
        {
            SymbolsTreeModel::NameNode* n = static_cast<SymbolsTreeModel::NameNode*>(p);
            const String& name = n->Name();
            if (name.find("std::") == 0)
                return false;
        }
    }

    if (!filter.empty())
    {
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        GenericTreeNode* p = static_cast<GenericTreeNode*>(index.internalPointer());
        int type = p->Type();
        if (p->Type() == SymbolsTreeModel::TYPE_NAME)
        {
            SymbolsTreeModel::NameNode* n = static_cast<SymbolsTreeModel::NameNode*>(p);
            const String& name = n->Name();
            if (hideStd)
            {
                if (name.find("std::") == 0)
                    return false;
            }
            return name.find(filter) != String::npos;
        }
    }
    return true;
}
