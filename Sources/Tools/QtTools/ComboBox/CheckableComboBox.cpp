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


#include "CheckableComboBox.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QListView>
#include <QStylePainter>
#include <QDebug>


CheckableComboBox::CheckableComboBox(QWidget* parent)
    : QComboBox(parent)
{
    connect( model(), &QAbstractItemModel::rowsInserted, this, &CheckableComboBox::onRowsInserted );
    connect( this, &CheckableComboBox::done, this, &CheckableComboBox::updateTextHints );

    auto v = new QListView();
    setView(v);

    installEventFilter(this);
    view()->viewport()->installEventFilter(this);
}

CheckableComboBox::~CheckableComboBox()
{
}

QStringList CheckableComboBox::selectedItems() const
{
    const auto& indexes = checkedIndexes();
    QStringList list;

    for ( auto i = 0; i < indexes.size(); i++)
    {
        list << indexes[i].data(Qt::DisplayRole).toString();
    }

    return list;
}

QList<QVariant> CheckableComboBox::selectedUserData() const
{
    const auto& indexes = checkedIndexes();
    QVariantList list;

    for ( auto i = 0; i < indexes.size(); i++)
    {
        list << indexes[i].data(Qt::UserRole);
    }

    return list;
}

void CheckableComboBox::selectUserData(const QList<QVariant>& dataList)
{
    auto m = model();
    const auto n = m->rowCount();
    for ( auto i = 0; i < n; i++)
    {
        const auto index = m->index(i, 0, QModelIndex());
        const auto checkState = dataList.contains(index.data(Qt::UserRole)) ? Qt::Checked : Qt::Unchecked;
        m->setData(index, checkState, Qt::CheckStateRole);
    }

    updateTextHints();
}

void CheckableComboBox::onRowsInserted(const QModelIndex& parent, int start, int end)
{
    QStandardItemModel* m = qobject_cast<QStandardItemModel *>(model());
    if (m == nullptr)
        return ;

    for ( auto i = start; i <= end; i++)
    {
        auto item = m->item(i);
        item->setCheckable(true);
    }

    if ( isVisible() )
    {
        updateTextHints();
    }
}

void CheckableComboBox::updateTextHints()
{
    const auto& items = selectedItems();

    textHint = items.join(", ");
    const auto toolTip = items.join("\n");
    setToolTip(toolTip);

    update();
}

QModelIndexList CheckableComboBox::checkedIndexes() const
{
    QModelIndexList list;

    auto m = model();
    const auto n = m->rowCount();
    for ( auto i = 0; i < n; i++)
    {
        const auto index = m->index(i, 0, QModelIndex());
        const auto isChecked = (index.data(Qt::CheckStateRole).toInt() == Qt::Checked);
        if (isChecked)
        {
            list << index;
        }
    }

    return list;
}

bool CheckableComboBox::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == view()->viewport())
    {
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
            {
                auto v = view();
                auto m = v->model();
                const auto index = v->currentIndex();
                const auto isChecked = (m->data(index, Qt::CheckStateRole).toInt() == Qt::Checked);
                m->setData(index, isChecked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
            }
            break;
        case QEvent::MouseButtonRelease:
            return true;

        case QEvent::Show:
            updateTextHints();
            break;

        case QEvent::Hide:
            emit done();
            break;

        default:
            break;
        }
    }

    if ( obj == this )
    {
        switch(e->type())
        {
        case QEvent::Show:
            updateTextHints();
            break;

        default:
            break;
        }
    }

    return QComboBox::eventFilter(obj, e);
}

void CheckableComboBox::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter p(this);
    p.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox option;
    initStyleOption(&option);

    p.drawComplexControl(QStyle::CC_ComboBox, option);

    const auto textRect = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField);
    const QFontMetrics metrics(font());
    const auto elidedText = metrics.elidedText(textHint, Qt::ElideRight, textRect.width());

    p.drawText(textRect, Qt::AlignVCenter, elidedText);
}
