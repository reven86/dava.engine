
#include "TArc/Controls/ComboBox.h"
#include "TArc/Utils/ScopedValueGuard.h"
#include "TArc/DataProcessing/AnyQMetaType.h"

#include <Base/FastName.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectedMeta.h>

#include <QSignalBlocker>
#include <QVariant>

namespace DAVA
{
namespace TArc
{
namespace ComboBoxDetails
{
const M::Enum* GetEnumMeta(const Reflection& fieldValue, const Reflection& fieldEnumerator)
{
    const M::Enum* enumMeta = fieldValue.GetMeta<M::Enum>();
    if (enumMeta != nullptr)
    {
        return enumMeta;
    }

    Any value = fieldEnumerator.GetValue();
    if (fieldEnumerator.IsValid() && value.CanCast<const M::Enum*>())
    {
        return value.Cast<const M::Enum*>();
    }

    return nullptr;
}
}

ComboBox::ComboBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxy<QComboBox>(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ComboBox::ComboBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxy<QComboBox>(ControlDescriptor(fields), accessor, model, parent)
{
    SetupControl();
}

void ComboBox::SetupControl()
{
    connections.AddConnection(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), MakeFunction(this, &ComboBox::CurrentIndexChanged));

    installEventFilter(this);
}

void ComboBox::UpdateControl(const ControlDescriptor& changedFields)
{
    DVASSERT(updateControlProceed == false);
    ScopedValueGuard<bool> guard(updateControlProceed, true);

    Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Value));
    DVASSERT(fieldValue.IsValid());

    Reflection fieldEnumerator;
    const FastName& enumeratorName = changedFields.GetName(Fields::Enumerator);
    if (enumeratorName.IsValid())
    {
        fieldEnumerator = model.GetField(enumeratorName);
    }

    ProcessReadOnlyState(fieldValue, changedFields);

    int countInCombo = count();
    if (countInCombo == 0 || changedFields.IsChanged(Fields::Enumerator))
    {
        CreateItems(fieldValue, fieldEnumerator);
    }

    DVASSERT(count() != 0);

    int currentIndex = SelectCurrentItem(fieldValue, fieldEnumerator);
    setCurrentIndex(currentIndex);
}

void ComboBox::ProcessReadOnlyState(const Reflection& fieldValue, const ControlDescriptor& changedFields)
{
    bool readOnly = fieldValue.IsReadonly();
    readOnly |= fieldValue.GetMeta<M::ReadOnly>() != nullptr;
    if (changedFields.IsChanged(Fields::IsReadOnly) == true)
    {
        Reflection readOnlyField = model.GetField(changedFields.GetName(Fields::IsReadOnly));
        DVASSERT(readOnlyField.IsValid());
        readOnly |= readOnlyField.GetValue().Cast<bool>();
    }
    setEnabled(!readOnly);
}

void ComboBox::CreateItems(const Reflection& fieldValue, const Reflection& fieldEnumerator)
{
    QSignalBlocker blockSignals(this);

    if (count() != 0)
    {
        clear();
    }

    const M::Enum* enumMeta = ComboBoxDetails::GetEnumMeta(fieldValue, fieldEnumerator);
    if (enumMeta != nullptr)
    {
        const EnumMap* enumMap = enumMeta->GetEnumMap();
        int countInMap = static_cast<int>(enumMap->GetCount());
        for (int i = 0; i < countInMap; ++i)
        {
            int iValue = 0;
            bool ok = enumMap->GetValue(i, iValue);
            if (ok)
            {
                QVariant dataValue;
                dataValue.setValue(Any(iValue));

                addItem(enumMap->ToString(iValue), dataValue);
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
    else
    {
        DVASSERT(fieldEnumerator.IsValid() == true);

        Vector<Reflection::Field> fields = fieldEnumerator.GetFields();
        for (Reflection::Field& field : fields)
        {
            Any fieldDescr = field.ref.GetValue();

            QVariant dataValue;
            dataValue.setValue(field.key);

            if (fieldDescr.CanCast<QIcon>())
            {
                addItem(fieldDescr.Cast<QIcon>(), fieldDescr.Cast<String>().c_str(), dataValue);
            }
            else
            {
                addItem(fieldDescr.Cast<String>().c_str(), dataValue);
            }
        }
    }
}

int ComboBox::SelectCurrentItem(const Reflection& fieldValue, const Reflection& fieldEnumerator)
{
    const M::Enum* enumMeta = ComboBoxDetails::GetEnumMeta(fieldValue, fieldEnumerator);
    Any value = fieldValue.GetValue();
    if (value.IsEmpty() == false)
    {
        int countInCombo = count();
        for (int i = 0; i < countInCombo; ++i)
        {
            Any iAny = itemData(i).value<Any>();
            if (enumMeta != nullptr && value.Cast<int>() == iAny.Cast<int>())
            {
                return i;
            }
            else if (value == iAny)
            {
                return i;
            }
        }
    }

    return (-1);
}

void ComboBox::CurrentIndexChanged(int newCurrentItem)
{
    if (updateControlProceed)
    {
        // ignore reaction on control initialization
        return;
    }

    wrapper.SetFieldValue(GetFieldName(Fields::Value), itemData(newCurrentItem).value<Any>());
}

} // namespace TArc
} // namespace DAVA
