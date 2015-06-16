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

#ifndef __TAGMODEL_H__
#define __TAGMODEL_H__

#include <QColor>
#include <QAbstractTableModel>

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

class ProfilingSession;
class MemoryStatItem;

class TagModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum {
        CLM_NAME = 0,
        CLM_ALLOC_APP,
        CLM_NBLOCKS,
        NCOLUMNS = 3
    };

public:
    TagModel(QObject* parent = nullptr);
    virtual ~TagModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void BeginNewProfileSession(ProfilingSession* profSession);
    void SetCurrentValues(const MemoryStatItem& item);
    void SetTagColors(QColor colorActive, QColor colorInactive);

private:
    ProfilingSession* profileSession;

    DAVA::uint64 timestamp;
    DAVA::uint32 activeTags;
    DAVA::Vector<DAVA::TagAllocStat> curValues;
    QColor colors[2];
};

#endif  // __TAGMODEL_H__
