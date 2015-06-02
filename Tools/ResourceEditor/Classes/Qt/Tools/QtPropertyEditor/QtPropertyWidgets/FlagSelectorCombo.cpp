/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "FlagSelectorCombo.h"

#include <QMouseEvent>
#include <QStylePainter>

#include "Base/EnumMap.h"
#include "Debug/DVAssert.h"


FlagSelectorCombo::FlagSelectorCombo(QWidget* parent)
    : QComboBox(parent)
    , extraMask(0)
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

void FlagSelectorCombo::AddFlagItem(const quint64 value, const QString& hint)
{
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
    DVASSERT(m);

    const QString numValue = QString::number(value);
    const QString toolTip = hint.isEmpty() ? numValue : QString("(%1) %2").arg(value).arg(hint);

    QStandardItem *item = new QStandardItem();
    item->setCheckable(true);
    item->setData(value, ValueRole);
    item->setText(!hint.isEmpty() ? hint : numValue);
    item->setToolTip(toolTip);

    m->appendRow(item);
}

void FlagSelectorCombo::SetFlags(const quint64 flags)
{
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
    DVASSERT(m);

    int knownFlags = 0;
    const int n = m->rowCount();
    for (int i = 0; i < n; i++)
    {
        QStandardItem *item = m->item(i);
        const quint64 flag = item->data(ValueRole).toULongLong();
        const bool isChecked = ( (flag & flags) == flag );
        item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        knownFlags |= flag;
    }
    extraMask = flags & ~knownFlags;

    updateText();
}

quint64 FlagSelectorCombo::GetFlags() const
{
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
    DVASSERT(m);

    quint64 flags = 0;
    const int n = m->rowCount();
    for (int i = 0; i < n; i++)
    {
        QStandardItem *item = m->item(i);
        if (item->checkState() == Qt::Checked)
        {
            const quint64 flag = item->data(ValueRole).toULongLong();
            flags |= flag;
        }
    }
    flags |= extraMask;

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
    const QString toolTip = hints.join( "\n" );
    setToolTip(toolTip);
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

        case QEvent::Hide:
            {
                const quint64 flags = GetFlags();
                emit done(flags);
            }
            break;

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
