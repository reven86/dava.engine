#include "FlagSelectorCombo.h"

#include <QMouseEvent>
#include <QStylePainter>

#include "Base/EnumMap.h"
#include "Debug/DVAssert.h"


FlagSelectorCombo::FlagSelectorCombo(QWidget* parent)
    : QComboBox(parent)
{
    setEditable(false);

    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
    DVASSERT(m);
    connect( m, SIGNAL( itemChanged(QStandardItem*) ), SLOT( onItemChanged(QStandardItem*) ) );

    QListView *v = new QListView();
    setView(v);

    view()->viewport()->installEventFilter(this);
}

FlagSelectorCombo::~FlagSelectorCombo()
{
}

void FlagSelectorCombo::AddFlagItem(int const value, QString const& hint)
{
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
    DVASSERT(m);

    QStandardItem *item = new QStandardItem();
    item->setCheckable(true);
    item->setData(value, ValueRole);
    item->setText(!hint.isEmpty() ? hint : QString::number(value));

    m->appendRow(item);
}

void FlagSelectorCombo::SetFlags(const int flags)
{
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
    DVASSERT(m);

    const int n = m->rowCount();
    for (int i = 0; i < n; i++)
    {
        QStandardItem *item = m->item(i);
        const int flag = item->data(ValueRole).toInt();
        const bool isChecked = ( (flag & flags) == flag );
        item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
    }

    updateText();
}

int FlagSelectorCombo::GetFlags() const
{
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
    DVASSERT(m);

    int flags = 0;
    const int n = m->rowCount();
    for (int i = 0; i < n; i++)
    {
        QStandardItem *item = m->item(i);
        if (item->checkState() == Qt::Checked)
        {
            const int flag = item->data(ValueRole).toInt();
            flags |= flag;
        }
    }

    return flags;
}

void FlagSelectorCombo::onItemChanged(QStandardItem* item)
{
    Q_UNUSED(item);
    updateText();
}

void FlagSelectorCombo::updateText()
{
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
    DVASSERT(m);

    QStringList hints;
    const int n = m->rowCount();
    for (int i = 0; i < n; i++)
    {
        QStandardItem *item = m->item(i);
        if (item->checkState() == Qt::Checked)
        {
            hints << item->text();
        }
    }

    text = hints.join( " | " );
    setToolTip(text);
    repaint();
}

bool FlagSelectorCombo::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == view()->viewport())
    {
        switch (e->type())
        {
        case QEvent::MouseButtonRelease:
            {
                QAbstractItemView *v = view();
                QAbstractItemModel *m = v->model();
                const QModelIndex index = v->currentIndex();
                const bool isChecked = ( m->data(index, Qt::CheckStateRole).toInt() == Qt::Checked );
                m->setData( index, isChecked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
            }
            return true;

        default:
            break;
        }
    }

    return QComboBox::eventFilter(obj, e);
}

void FlagSelectorCombo::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter p(this);
    p.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox option;
    initStyleOption(&option);

    p.drawComplexControl(QStyle::CC_ComboBox, option);

    const QRect textRect = style()->subControlRect( QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField );
    const QFontMetrics metrics(font());
    const QString elidedText = metrics.elidedText( text, Qt::ElideRight, textRect.width() );

    p.drawText(textRect, Qt::AlignVCenter, elidedText);
}
