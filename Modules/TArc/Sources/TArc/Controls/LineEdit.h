#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"

#include <QLineEdit>

namespace DAVA
{
namespace TArc
{
class LineEdit final : public ControlProxy<QLineEdit>
{
public:
    struct FieldsDescriptor
    {
        Any valueFieldName;
    };

    LineEdit(const FieldsDescriptor& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    LineEdit(const FieldsDescriptor& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields) override;

    void SetupControl();
    void EditingFinished();

private:
    FieldsDescriptor fieldsDescr;
    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA