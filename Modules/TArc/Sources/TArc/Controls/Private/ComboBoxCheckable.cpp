#include "TArc/Controls/ComboBoxCheckable.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Utils/ScopedValueGuard.h"
#include "TArc/DataProcessing/AnyQMetaType.h"

#include <QtTools/WarningGuard/QtWarningsHandler.h>

#include <Base/FastName.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectedMeta.h>

#include <QEvent>
#include <QPaintEvent>
#include <QListView>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QVariant>

#include <QStyle>
#include <QStylePainter>

namespace DAVA
{
namespace TArc
{
namespace ComboBoxCheckableDetails
{
class Model : public QStandardItemModel
{
public:
    Model(QObject* parent = nullptr)
        : QStandardItemModel(parent)
    {
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        return QStandardItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }
};
}

QString CreateTextComboCheckable(const Any& value, const EnumMap* enumMap)
{
    DVASSERT(enumMap != nullptr);

    QString result;
    if (value.IsEmpty() == true)
    {
        result = QString(MultipleValuesString);
    }
    else if (enumMap != nullptr)
    {
        QStringList list;

        int intValue = value.Cast<int>();

        int count = static_cast<int>(enumMap->GetCount());
        for (int i = count - 1; (i >= 0) && (intValue != 0); --i)
        {
            int iValue = 0;
            bool ok = enumMap->GetValue(i, iValue);
            DVASSERT(ok);

            if ((intValue & iValue) == iValue)
            {
                list.insert(0, enumMap->ToString(iValue));
                intValue -= iValue;
            }
        }

        if (list.isEmpty())
        {
            result = "<none>";
        }
        else
        {
            result = list.join(", ");
        }
    }

    return result;
}

ComboBoxCheckable::ComboBoxCheckable(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QComboBox>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ComboBoxCheckable::ComboBoxCheckable(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QComboBox>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void ComboBoxCheckable::SetupControl()
{
    setModel(new ComboBoxCheckableDetails::Model(this->ToWidgetCast()));
    setView(new QListView(this->ToWidgetCast()));

    view()->viewport()->installEventFilter(this);
}

void ComboBoxCheckable::UpdateControl(const ControlDescriptor& changedFields)
{
    bool readOnlyChanged = changedFields.IsChanged(Fields::IsReadOnly);
    bool valueChanged = changedFields.IsChanged(Fields::Value);
    if (readOnlyChanged || valueChanged)
    {
        bool readOnly = IsValueReadOnly(changedFields, Fields::Value, Fields::IsReadOnly);
        setEnabled(!readOnly);
    }

    Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Value));
    DVASSERT(fieldValue.IsValid());

    if (count() == 0)
    {
        CreateItems(fieldValue);
    }

    DVASSERT(count() != 0);

    SelectCurrentItems(fieldValue);
    UpdateText();
    update();
}

void ComboBoxCheckable::CreateItems(const Reflection& fieldValue)
{
    QSignalBlocker blockSignals(this);

    const M::Flags* enumFlags = fieldValue.GetMeta<M::Flags>();
    if (enumFlags != nullptr)
    {
        const EnumMap* enumMap = enumFlags->GetFlagsMap();
        int countInMap = static_cast<int>(enumMap->GetCount());

        for (int i = 0; i < countInMap; ++i)
        {
            int iValue = 0;
            bool ok = enumMap->GetValue(i, iValue);
            if (ok)
            {
                if (iValue != 0)
                {
                    addItem(enumMap->ToString(iValue), iValue);
                }
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
}

void ComboBoxCheckable::SelectCurrentItems(const Reflection& fieldValue)
{
    Any value = fieldValue.GetValue();
    if (value.IsEmpty() == false)
    {
        cachedValue = value.Cast<int>();
        UpdateCheckedState();
    }
    else
    {
        cachedValue = 0;

        QAbstractItemModel* abstractModel = QComboBox::model();

        int countInCombo = count();
        DVASSERT(countInCombo == abstractModel->rowCount());

        for (int i = 0; i < countInCombo; ++i)
        {
            const QModelIndex index = abstractModel->index(i, 0);
            abstractModel->setData(index, Qt::PartiallyChecked, Qt::CheckStateRole);
        }
    }
}

void ComboBoxCheckable::UpdateText()
{
    Reflection fieldValue = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(fieldValue.IsValid());

    const M::Flags* enumFlags = fieldValue.GetMeta<M::Flags>();
    if (enumFlags != nullptr)
    {
        const EnumMap* enumMap = enumFlags->GetFlagsMap();
        if (enumMap != nullptr)
        {
            text = CreateTextComboCheckable(cachedValue, enumMap);
        }
    }

    setToolTip(text);
}

void ComboBoxCheckable::UpdateCheckedState()
{
    QAbstractItemModel* abstractModel = QComboBox::model();

    int countInCombo = count();
    DVASSERT(countInCombo == abstractModel->rowCount());
    for (int i = 0; i < countInCombo; ++i)
    {
        int iAny = itemData(i).value<int>();
        bool flagEnabled = ((cachedValue & iAny) == iAny);
        Qt::CheckState state = flagEnabled ? Qt::Checked : Qt::Unchecked;

        const QModelIndex index = abstractModel->index(i, 0);
        abstractModel->setData(index, state, Qt::CheckStateRole);
    }
}

void ComboBoxCheckable::ApplyChanges()
{
    wrapper.SetFieldValue(GetFieldName(Fields::Value), cachedValue);
}

bool ComboBoxCheckable::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == view()->viewport())
    {
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
        {
            QAbstractItemView* v = view();
            QAbstractItemModel* abstractModel = v->model();
            QModelIndex index = v->currentIndex();

            int previousState = abstractModel->data(index, Qt::CheckStateRole).toInt();
            if (previousState == Qt::PartiallyChecked)
            {
                cachedValue = 0;
            }
            else
            {
                QVariant checkData = abstractModel->data(index, Qt::CheckStateRole);
                int iAny = itemData(index.row()).value<int>();

                if (previousState == Qt::Checked)
                {
                    cachedValue &= ~iAny; //remove flag
                }
                else //Unchecked
                {
                    cachedValue |= iAny; //add flag
                }
            }
            UpdateCheckedState();
            UpdateText();
            update();
        }
        break;
        case QEvent::MouseButtonRelease:
            return true;
        case QEvent::Hide:
            ApplyChanges();
            break;
        default:
            break;
        }
    }

    return QComboBox::eventFilter(obj, e);
}

void ComboBoxCheckable::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox option;
    initStyleOption(&option);

    painter.drawComplexControl(QStyle::CC_ComboBox, option);

    QRect textRect = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField);
    QFontMetrics metrics(font());
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, textRect.width());

    painter.drawText(textRect, Qt::AlignVCenter, elidedText);
}

} // namespace TArc
} // namespace DAVA
