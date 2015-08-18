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

#ifndef __MEMORYTOOL_MEMORYBLOCKSMODEL_H__
#define __MEMORYTOOL_MEMORYBLOCKSMODEL_H__

#include <functional>

#include "Base/BaseTypes.h"

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"

class ProfilingSession;
struct BlockLink;

class MemoryBlocksModel : public QAbstractTableModel
{
public:
    enum {
        ROLE_LINKITEM_POINTER = Qt::UserRole + 1
    };

public:
    MemoryBlocksModel(const ProfilingSession* session, QObject* parent = nullptr);
    virtual ~MemoryBlocksModel();

    void SetBlockLink(const BlockLink* blockLink);

    // reimplemented QAbstractTableModel methods
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    const ProfilingSession* session = nullptr;
    const BlockLink* blockLink = nullptr;
};

//////////////////////////////////////////////////////////////////////////
class MemoryBlocksFilterModel : public QSortFilterProxyModel
{
public:
    MemoryBlocksFilterModel(MemoryBlocksModel* model, QObject* parent = nullptr);
    virtual ~MemoryBlocksFilterModel();

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    // TODO: replace with DAVA::Function
    std::function<bool (const BlockLink::Item&, const BlockLink::Item&)> lessThanPredicate;
    std::function<bool (const BlockLink::Item&)> filterPredicate;

    bool dontFilter = true;
};

#endif  // __MEMORYTOOL_MEMORYBLOCKSMODEL_H__
