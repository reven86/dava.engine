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


#pragma once
#include <QtGui>
#include <QItemDelegate.h>
#include "MemoryManager/MemoryManagerTypes.h"

class MyDelegate : public QItemDelegate
{
public:
    MyDelegate(QObject* parent = 0)
        : QItemDelegate(parent)
    {}
    void paint(QPainter* painter, const QStyleOptionViewItem& opt, const QModelIndex& index) const
    {
        if (index.column() != 0) {
            QItemDelegate::paint(painter, opt, index);
            return;
        }
        // Draw a check box for the status column
        bool enabled = true;
        QStyleOptionViewItem option = opt;
        QRect crect = option.rect;
        crect.moveLeft(15);
        drawCheck(painter, option, crect, enabled ? Qt::Checked : Qt::Unchecked);
    }
};

class MemProfInfoModel : public QAbstractTableModel
{
public:
    MemProfInfoModel();
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const; 
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    virtual ~MemProfInfoModel();
    void addMoreData(const DAVA::MMStat * data);
    void setConfig(const DAVA::MMStatConfig* statConfig);
private:
    QString formatMemoryData(int memoryData);
    struct MemData
    {
        DAVA::uint32 allocByApp;
        DAVA::uint32 allocTotal;
    };
    using PoolsStat = DAVA::Vector < MemData > ;
    using TagsStatVector = DAVA::Vector < PoolsStat >;
    struct TagsStat
    {
        TagsStatVector statData;
        DAVA::Vector<int> tagNames;
    };
    
    QMap<int, TagsStat > timedData;
    DAVA::Vector<QString> tagNames;
    DAVA::Vector<QString> poolNames;
};

