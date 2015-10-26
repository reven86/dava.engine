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

#ifndef __MEMORYTOOL_MEMORYBLOCKSWIDGET_H__
#define __MEMORYTOOL_MEMORYBLOCKSWIDGET_H__

#include "Base/BaseTypes.h"

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"
#include "Qt/DeviceInfo/MemoryTool/BlockGroup.h"

#include <QWidget>

class QTableView;
class QListView;
class QModelIndex;

class ProfilingSession;
class MemorySnapshot;
class MemoryBlocksModel;
class MemoryBlocksFilterModel;
class BlockGroupModel;
class BacktraceListModel;
struct BlockLink;

class MemoryBlocksWidget : public QWidget
{
    Q_OBJECT

public:
    MemoryBlocksWidget(const ProfilingSession* session, const BlockLink* blockLink, bool showBacktrace = true, bool enableGrouping = true, QWidget* parent = nullptr);
    virtual ~MemoryBlocksWidget();

    void SetBlockLink(const BlockLink* blockLink);

signals:
    void MemoryBlockDoubleClicked(const BlockLink::Item& item);

private slots:
    void TableWidget_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void TableWidget_DoubleClicked(const QModelIndex& index);

    void GroupWidget_SelectionChanged(const QModelIndex& current, const QModelIndex& previous);

    void FilterBar_GroupOrderChanged(int order);
    void FilterBar_SortingOrderChanged(int order);
    void FilterBar_FilterChanged(DAVA::uint32 poolMask, DAVA::uint32 tagMask);
    void FilterBar_HideTheSameChanged(bool hide);
    void FilterBar_HideDifferentChanged(bool hide);
    void FilterBar_BlockOrderChanged(DAVA::uint32 minBlockOrder);

private:
    void Init();
    bool Filter(const BlockLink::Item& item);
    void GroupBy();

private:
    const ProfilingSession* session = nullptr;
    const BlockLink* blockLink = nullptr;

    QTableView* tableWidget = nullptr;
    QTableView* groupsWidget = nullptr;
    QListView* backtraceWidget = nullptr;
    bool showBacktrace = true;
    bool enableGrouping = false;

    std::unique_ptr<MemoryBlocksModel> memoryBlocksModel;
    std::unique_ptr<MemoryBlocksFilterModel> memoryBlocksFilterModel;
    std::unique_ptr<BlockGroupModel> blockGroupModel;
    std::unique_ptr<BacktraceListModel> backtraceListModel;

    DAVA::int32 groupOrder = 0;
    DAVA::uint32 filterPoolMask = 0;
    DAVA::uint32 filterTagMask = 0;
    bool hideTheSame = false;
    bool hideDifferent = false;
    DAVA::uint32 minBlockOrder = 0;

    DAVA::Vector<BlockGroup> groups;
};

#endif  // __MEMORYTOOL_MEMORYBLOCKSWIDGET_H__
