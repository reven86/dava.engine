#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Utils/QtConnections.h"

#include <QListView>
#include <QAbstractItemModel>

namespace DAVA
{
namespace TArc
{
class ListView : public ControlProxyImpl<QListView>
{
    using TBase = ControlProxyImpl<QListView>;

public:
    enum Fields : uint32
    {
        CurrentValue,
        ValueList,
        IsReadOnly,
        FieldCount
    };

    ListView(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ListView(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& fields) override;

    void OnSelectionChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection);

    QAbstractItemModel* listModel = nullptr;
    QtConnections connections;
    bool updateGuard = false;
};
} // namespace TArc
} // namespace DAVA
