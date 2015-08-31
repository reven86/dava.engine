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
#include "MemoryManager/MemoryManagerTypes.h"

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"
#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/Models/MemoryBlocksModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/BacktraceListModel.h"

#include "Qt/DeviceInfo/MemoryTool/Widgets/FilterAndSortBar.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/MemoryBlocksWidget.h"

#include <QTableView>
#include <QHeaderView>
#include <QListView>
#include <QVBoxLayout>
#include <QItemSelectionModel>
#include <QSplitter>
#include <QFrame>

using namespace DAVA;

MemoryBlocksWidget::MemoryBlocksWidget(const ProfilingSession* session_, const BlockLink* blockLink_, bool showBacktrace_, QWidget* parent)
    : QWidget(parent)
    , session(session_)
    , blockLink(blockLink_)
    , showBacktrace(showBacktrace_)
{
    DVASSERT(session != nullptr && blockLink != nullptr);
    Init();
}

MemoryBlocksWidget::~MemoryBlocksWidget() = default;

void MemoryBlocksWidget::SetBlockLink(const BlockLink* blockLink_)
{
    DVASSERT(blockLink_ != nullptr);
    blockLink = blockLink_;
    memoryBlocksModel->SetBlockLink(blockLink);
}

void MemoryBlocksWidget::TableWidget_SelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    QVariant v = tableWidget->model()->data(current, MemoryBlocksModel::ROLE_LINKITEM_POINTER);
    const BlockLink::Item* item = static_cast<const BlockLink::Item*>(v.value<void*>());
    if (item != nullptr)
    {
        uint32 hash = BlockLink::AnyBlock(*item)->bktraceHash;
        backtraceListModel->Update(hash);
    }
    else
    {
        backtraceListModel->Clear();
    }
}

void MemoryBlocksWidget::TableWidget_DoubleClicked(const QModelIndex& index)
{
    QVariant v = tableWidget->model()->data(index, MemoryBlocksModel::ROLE_LINKITEM_POINTER);
    const BlockLink::Item* item = static_cast<const BlockLink::Item*>(v.value<void*>());
    if (item != nullptr)
    {
        emit MemoryBlockDoubleClicked(*item);
    }
}

void MemoryBlocksWidget::FilterBar_SortingOrderChanged(int order)
{
    std::function<bool(const BlockLink::Item&, const BlockLink::Item&)> fn;
    switch (order)
    {
    case FilterAndSortBar::SORT_BY_ORDER:
        fn = [](const BlockLink::Item& l, const BlockLink::Item& r) -> bool
        {
            return BlockLink::AnyBlock(l)->orderNo < BlockLink::AnyBlock(r)->orderNo;
        };
        break;
    case FilterAndSortBar::SORT_BY_SIZE:
        fn = [](const BlockLink::Item& l, const BlockLink::Item& r) -> bool
        {
            const MMBlock* left = BlockLink::AnyBlock(l);
            const MMBlock* right = BlockLink::AnyBlock(r);
            if (left->allocByApp == right->allocByApp)
            {
                return left->orderNo < right->orderNo;
            }
            return left->allocByApp > right->allocByApp;
        };
        break;
    case FilterAndSortBar::SORT_BY_POOL:
        fn = [](const BlockLink::Item& l, const BlockLink::Item& r) -> bool
        {
            const MMBlock* left = BlockLink::AnyBlock(l);
            const MMBlock* right = BlockLink::AnyBlock(r);
            if (left->pool == right->pool)
            {
                if (left->allocByApp == right->allocByApp)
                {
                    return left->orderNo < right->orderNo;
                }
                return left->allocByApp > right->allocByApp;
            }
            return left->pool < right->pool;
        };
        break;
    case FilterAndSortBar::SORT_BY_BACKTRACE:
        fn = [](const BlockLink::Item& l, const BlockLink::Item& r) -> bool
        {
            const MMBlock* left = BlockLink::AnyBlock(l);
            const MMBlock* right = BlockLink::AnyBlock(r);
            if (left->bktraceHash == right->bktraceHash)
            {
                if (left->allocByApp == right->allocByApp)
                {
                    return left->orderNo < right->orderNo;
                }
                return left->allocByApp > right->allocByApp;
            }
            return left->bktraceHash < right->bktraceHash;
        };
        break;
    default:
        DVASSERT(0 && "Invalid sort order! Something goes wrong");
        break;
    }
    memoryBlocksFilterModel->SortBy(fn);
}

void MemoryBlocksWidget::FilterBar_FilterChanged(uint32 poolMask, uint32 tagMask)
{
    filterPoolMask = poolMask;
    filterTagMask = tagMask;
    if (filterTagMask != 0 || filterPoolMask != 0)
    {
        memoryBlocksFilterModel->SetFilter([this](const BlockLink::Item& item) ->bool { return Filter(item); });
    }
    else
    {
        memoryBlocksFilterModel->ClearFilter();
    }
}

void MemoryBlocksWidget::FilterBar_HideTheSameChanged(bool hide)
{
    hideTheSame = hide;
    memoryBlocksFilterModel->SetFilter([this](const BlockLink::Item& item) ->bool { return Filter(item); });
}

void MemoryBlocksWidget::Init()
{
    memoryBlocksModel.reset(new MemoryBlocksModel(session));
    memoryBlocksModel->SetBlockLink(blockLink);
    memoryBlocksFilterModel.reset(new MemoryBlocksFilterModel(memoryBlocksModel.get()));

    backtraceListModel.reset(new BacktraceListModel(session->SymbolTable()));

    tableWidget = new QTableView;
    tableWidget->setFont(QFont("Consolas", 10, 500));
    int fontHeight = QFontMetrics(tableWidget->font()).height();
    tableWidget->verticalHeader()->setDefaultSectionSize(fontHeight + 6);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setModel(memoryBlocksFilterModel.get());

    const int32 flags = 1 == blockLink->linkCount ? FilterAndSortBar::FLAG_ENABLE_ALL_FOR_SINGLE : FilterAndSortBar::FLAG_ENABLE_ALL_FOR_DIFF;
    FilterAndSortBar* filterBar = new FilterAndSortBar(session, flags);
    connect(filterBar, &FilterAndSortBar::SortingOrderChanged, this, &MemoryBlocksWidget::FilterBar_SortingOrderChanged);
    connect(filterBar, &FilterAndSortBar::FilterChanged, this, &MemoryBlocksWidget::FilterBar_FilterChanged);
    connect(filterBar, &FilterAndSortBar::HideTheSameChanged, this, &MemoryBlocksWidget::FilterBar_HideTheSameChanged);

    QVBoxLayout* layout1 = new QVBoxLayout;
    layout1->addWidget(filterBar);
    layout1->addWidget(tableWidget);

    if (showBacktrace)
    {
        backtraceWidget = new QListView;
        backtraceWidget->setFont(QFont("Consolas", 10, 500));
        backtraceWidget->setModel(backtraceListModel.get());

        QItemSelectionModel* tableSelectionModel = tableWidget->selectionModel();
        connect(tableSelectionModel, &QItemSelectionModel::currentChanged, this, &MemoryBlocksWidget::TableWidget_SelectionChanged);

        QVBoxLayout* layout2 = new QVBoxLayout;
        layout2->addWidget(backtraceWidget);

        QFrame* frame1 = new QFrame;
        frame1->setLayout(layout1);

        QFrame* frame2 = new QFrame;
        frame2->setLayout(layout2);

        QSplitter* splitter = new QSplitter(Qt::Vertical);
        splitter->addWidget(frame1);
        splitter->addWidget(frame2);

        QVBoxLayout* splitter_layout = new QVBoxLayout;
        splitter_layout->addWidget(splitter);
        setLayout(splitter_layout);
    }
    else
    {
        setLayout(layout1);
    }
}

bool MemoryBlocksWidget::Filter(const BlockLink::Item& item)
{
    if (hideTheSame && (item.first != nullptr && item.second != nullptr))
    {
        return false;
    }

    bool accept = true;
    const MMBlock* block = BlockLink::AnyBlock(item);
    if (filterPoolMask != 0)
    {
        accept = (block->pool & filterPoolMask) != 0;
    }
    if (filterTagMask != 0)
    {
        accept &= (block->tags & filterTagMask) != 0;
    }
    return accept;
}
