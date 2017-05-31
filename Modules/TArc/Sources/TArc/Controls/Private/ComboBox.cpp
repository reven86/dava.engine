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
ComboBox::ComboBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QComboBox>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ComboBox::ComboBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QComboBox>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void ComboBox::SetupControl()
{
    connections.AddConnection(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), MakeFunction(this, &ComboBox::CurrentIndexChanged));
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

void ComboBox::UpdateControl(const ControlDescriptor& changedFields)
{
    DVASSERT(updateControlProceed == false);
    ScopedValueGuard<bool> guard(updateControlProceed, true);

    bool readOnlyChanged = changedFields.IsChanged(Fields::IsReadOnly);
    bool valueChanged = changedFields.IsChanged(Fields::Value);
    if (readOnlyChanged || valueChanged)
    {
        bool readOnly = IsValueReadOnly(changedFields, Fields::Value, Fields::IsReadOnly);
        setEnabled(!readOnly);
    }

    Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Value));
    DVASSERT(fieldValue.IsValid());

    Reflection fieldEnumerator;
    const FastName& enumeratorName = changedFields.GetName(Fields::Enumerator);
    if (enumeratorName.IsValid())
    {
        fieldEnumerator = model.GetField(enumeratorName);
    }

    int countInCombo = count();
    if (countInCombo == 0 || changedFields.IsChanged(Fields::Enumerator))
    {
        CreateItems(fieldValue, fieldEnumerator);
    }

    DVASSERT(count() != 0);

    int currentIndex = SelectCurrentItem(fieldValue, fieldEnumerator);
    setCurrentIndex(currentIndex);
}

void ComboBox::CreateItems(const Reflection& fieldValue, const Reflection& fieldEnumerator)
{
    QSignalBlocker blockSignals(this);

    if (count() != 0)
    {
        clear();
    }

    const M::Enum* enumMeta = fieldValue.GetMeta<M::Enum>();
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

            addItem(fieldDescr.Cast<QIcon>(QIcon()), fieldDescr.Cast<String>().c_str(), dataValue);
        }
    }
}

int ComboBox::SelectCurrentItem(const Reflection& fieldValue, const Reflection& fieldEnumerator)
{
    Any value = fieldValue.GetValue();
    if (value.IsEmpty() == false)
    {
        int countInCombo = count();
        for (int i = 0; i < countInCombo; ++i)
        {
            Any iAny = itemData(i).value<Any>();
            if (value == iAny)
            {
                return i;
            }
            else if (iAny.CanCast<int>() && value.CanCast<int>() &&
                     iAny.Cast<int>() == value.Cast<int>())
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
