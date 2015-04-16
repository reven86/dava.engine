#include "SymbolsFilterModel.h"
#include "SymbolsTreeModel.h"
#include "GenericTreeNode.h"

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

    if (ltype == SymbolsTreeModel::TYPE_NAME)
    {
        SymbolsTreeModel::NameNode* l = static_cast<SymbolsTreeModel::NameNode*>(pl);
        SymbolsTreeModel::NameNode* r = static_cast<SymbolsTreeModel::NameNode*>(pr);

        return strcmp(l->Name(), r->Name()) < 0;
    }
    return false;
}

bool SymbolsFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    GenericTreeNode* p = static_cast<GenericTreeNode*>(index.internalPointer());
    if (p->Type() == SymbolsTreeModel::TYPE_NAME)
    {
        SymbolsTreeModel::NameNode* node = static_cast<SymbolsTreeModel::NameNode*>(p);
        String name = node->Name();
        if (hideStd && (name.find("std::") == 0 || name.find("#_") == 0))
        {
            return false;
        }
        return name.find(filter) != String::npos;
    }
    return true;
}
