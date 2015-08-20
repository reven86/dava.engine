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

#include "Debug/DVAssert.h"

#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/FilterAndSortBar.h"

#include "QtTools/ComboBox/CheckableComboBox.h"

#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QStandardItem>

using namespace DAVA;

FilterAndSortBar::FilterAndSortBar(const ProfilingSession* session_, int32 flags, QWidget* parent)
    : QWidget(parent)
    , session(session_)
{
    DVASSERT((flags & FLAG_ENABLE_ALL) != 0);
    Init(flags);
}

FilterAndSortBar::~FilterAndSortBar() = default;

void FilterAndSortBar::Init(int32 flags)
{
    QHBoxLayout* layout = new QHBoxLayout;
    if (flags & FLAG_ENABLE_SORTING)
    {
        layout->addWidget(CreateSortCombo());
    }
    if (flags & FLAG_ENABLE_FILTER_BY_POOL)
    {
        layout->addWidget(CreateFilterPoolCombo());
    }
    if (flags & FLAG_ENABLE_FILTER_BY_TAG)
    {
        layout->addWidget(CreateFilterTagCombo());
    }
    if (flags & FLAG_ENABLE_HIDE_SAME)
    {
        layout->addWidget(CreateHideTheSameCheck());
    }
    setLayout(layout);
}

QComboBox* FilterAndSortBar::CreateSortCombo()
{
    std::pair<QString, int> items[] = {
        {"Sort by order", SORT_BY_ORDER},
        {"Sort by size", SORT_BY_SIZE},
        {"Sort by pool", SORT_BY_POOL},
        {"Sort by backtrace", SORT_BY_BACKTRACE}
    };

    int nrows = static_cast<int>(sizeof(items) / sizeof(items[0]));
    sortComboModel.reset(new QStandardItemModel(nrows, 1));
    for (int i = 0;i < nrows;++i)
    {
        QStandardItem* item = new QStandardItem(QString(items[i].first));
        item->setData(items[i].second, Qt::UserRole + 1);

        sortComboModel->setItem(i, 0, item);
    }

    QComboBox* widget = new QComboBox;
    connect(widget, SIGNAL(currentIndexChanged(int)), this, SLOT(SortOrderCombo_CurrentIndexChanged(int)));

    widget->setModel(sortComboModel.get());
    widget->setCurrentIndex(0);
    return widget;
}

CheckableComboBox* FilterAndSortBar::CreateFilterPoolCombo()
{
    CheckableComboBox* widget = new CheckableComboBox;

    int nrows = static_cast<int>(session->AllocPoolCount());
    for (int i = 0;i < nrows;++i)
    {
        const String& name = session->AllocPoolName(i);
        widget->addItem(name.c_str(), 1 << i);
    }

    QAbstractItemModel* model = widget->model();
    for (int i = 0;i < nrows;++i)
    {
        QModelIndex index = model->index(i, 0);
        model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }

    connect(widget, &CheckableComboBox::selectedUserDataChanged, this, &FilterAndSortBar::FilterPoolCombo_DataChanged);
    return widget;
}

CheckableComboBox* FilterAndSortBar::CreateFilterTagCombo()
{
    CheckableComboBox* widget = new CheckableComboBox;

    int nrows = static_cast<int>(session->TagCount());
    for (int i = 0;i < nrows;++i)
    {
        const String& name = session->TagName(i);
        widget->addItem(name.c_str(), 1 << i);
    }

    QAbstractItemModel* model = widget->model();
    for (int i = 0;i < nrows;++i)
    {
        QModelIndex index = model->index(i, 0);
        model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }

    connect(widget, &CheckableComboBox::selectedUserDataChanged, this, &FilterAndSortBar::FilterTagCombo_DataChanged);
    return widget;
}

QCheckBox* FilterAndSortBar::CreateHideTheSameCheck()
{
    QCheckBox* widget = new QCheckBox("Hide same blocks");
    widget->setTristate(false);

    connect(widget, &QCheckBox::stateChanged, this, &FilterAndSortBar::HideTheSameCheck_StateChanges);

    return widget;
}

void FilterAndSortBar::SortOrderCombo_CurrentIndexChanged(int index)
{
    QModelIndex modelIndex = sortComboModel->index(index, 0);
    if (modelIndex.isValid())
    {
        int v = sortComboModel->data(modelIndex, Qt::UserRole + 1).toInt();
        emit SortingOrderChanged(v);
    }
}

void FilterAndSortBar::FilterPoolCombo_DataChanged(const QVariantList& data)
{
    filterPoolMask = 0;
    for (const QVariant& v : data)
    {
        filterPoolMask |= v.toUInt();
    }
    emit FilterChanged(filterPoolMask, filterTagMask);
}

void FilterAndSortBar::FilterTagCombo_DataChanged(const QVariantList& data)
{
    filterTagMask = 0;
    for (const QVariant& v : data)
    {
        filterTagMask |= v.toUInt();
    }
    emit FilterChanged(filterPoolMask, filterTagMask);
}

void FilterAndSortBar::HideTheSameCheck_StateChanges(int state)
{
    hideTheSame = state == Qt::Checked;
    emit HideTheSameChanged(hideTheSame);
}
