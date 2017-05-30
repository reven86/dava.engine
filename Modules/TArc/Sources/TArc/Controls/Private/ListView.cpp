#include "TArc/Controls/ListView.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include <QAbstractListModel>
#include <QItemSelectionModel>
#include <QVariant>

namespace DAVA
{
namespace TArc
{
namespace ListViewDetails
{
class ListModel : public QAbstractListModel
{
public:
    int rowCount(const QModelIndex& parent) const override
    {
        return static_cast<int>(values.size());
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (role == Qt::DisplayRole)
        {
            int row = index.row();
            DVASSERT(row < static_cast<int>(values.size()));
            return values[row].second.Cast<QString>();
        }

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled);
    }

    void BeginReset()
    {
        beginResetModel();
    }

    void EndReset()
    {
        endResetModel();
    }

    Vector<std::pair<Any, Any>> values;
};
}

ListView::ListView(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ListView::ListView(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void ListView::SetupControl()
{
    setUniformItemSizes(true);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);

    listModel = new ListViewDetails::ListModel();
    setModel(listModel);

    QItemSelectionModel* selectModel = new QItemSelectionModel(listModel, this);
    connections.AddConnection(selectModel, &QItemSelectionModel::selectionChanged, MakeFunction(this, &ListView::OnSelectionChanged));
    setSelectionModel(selectModel);
}

void ListView::UpdateControl(const ControlDescriptor& fields)
{
    SCOPED_VALUE_GUARD(bool, updateGuard, true, void());

    ListViewDetails::ListModel* m = static_cast<ListViewDetails::ListModel*>(listModel);
    if (fields.IsChanged(Fields::ValueList))
    {
        m->BeginReset();
        m->values.clear();

        Reflection r = model.GetField(fields.GetName(Fields::ValueList));
        DVASSERT(r.IsValid());
        Vector<Reflection::Field> fields = r.GetFields();
        m->values.reserve(fields.size());
        for (const Reflection::Field& f : fields)
        {
            m->values.emplace_back(f.key, f.ref.GetValue());
        }

        m->EndReset();
    }

    if (fields.IsChanged(Fields::CurrentValue) || fields.IsChanged(Fields::ValueList))
    {
        Any currentValue = GetFieldValue(Fields::CurrentValue, Any());
        for (size_t i = 0; i < m->values.size(); ++i)
        {
            if (m->values[i].first == currentValue)
            {
                QModelIndex index = m->index(static_cast<int>(i), 0, QModelIndex());
                QItemSelectionModel* selectModel = selectionModel();
                selectModel->clearCurrentIndex();
                selectModel->select(index, QItemSelectionModel::ClearAndSelect);
                break;
            }
        }
    }

    setEnabled(!IsValueReadOnly(fields, Fields::CurrentValue, Fields::IsReadOnly));
}

void ListView::OnSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection)
{
    SCOPED_VALUE_GUARD(bool, updateGuard, true, void());

    QModelIndexList indexList = newSelection.indexes();
    DVASSERT(indexList.size() == 1);
    const QModelIndex& index = indexList.front();
    ListViewDetails::ListModel* m = static_cast<ListViewDetails::ListModel*>(listModel);
    DVASSERT(index.row() < m->values.size());
    wrapper.SetFieldValue(GetFieldName(Fields::CurrentValue), m->values[index.row()].first);
}

} // namespace TArc
} // namespace DAVA
