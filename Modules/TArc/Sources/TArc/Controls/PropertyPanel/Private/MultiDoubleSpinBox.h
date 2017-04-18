#pragma once
#include "TArc/Controls/ControlProxy.h"

#include <Base/BaseTypes.h>

#include <QtEvents>

namespace DAVA
{
namespace TArc
{
class MultiDoubleSpinBox : public ControlProxyImpl<QWidget>
{
    using TBase = ControlProxyImpl<QWidget>;

public:
    struct FieldDescriptor
    {
        String valueRole;
        String readOnlyRole;
        String rangeRole;
        String accuracyRole;

        bool operator==(const FieldDescriptor& other) const;
    };

    enum class Fields : uint32
    {
        FieldsList,
        FieldCount
    };

    MultiDoubleSpinBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    MultiDoubleSpinBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

protected:
    void focusInEvent(QFocusEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    template <typename T>
    void SetupControl(T* accessor);

    void ForceUpdate() override;
    void TearDown() override;

    void UpdateControl(const ControlDescriptor& descriptor) override;

    Vector<ControlProxy*> subControls;
    QWidget* lastFocusedItem = nullptr;
};
} // namespace TArc
} // namespace DAVA