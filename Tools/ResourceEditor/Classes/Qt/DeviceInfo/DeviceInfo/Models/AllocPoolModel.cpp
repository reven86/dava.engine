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

#include "AllocPoolModel.h"

#include "../ProfilingSession.h"
#include "DataFormat.h"

using namespace DAVA;

AllocPoolModel::AllocPoolModel(QObject* parent)
    : QAbstractTableModel(parent)
    , profileSession(nullptr)
    , timestamp(0)
{}

AllocPoolModel::~AllocPoolModel()
{}

int AllocPoolModel::rowCount(const QModelIndex& parent) const
{
    return profileSession != nullptr ? static_cast<int>(profileSession->AllocPoolCount())
                                     : 0;
}

int AllocPoolModel::columnCount(const QModelIndex& parent) const
{
    return NCOLUMNS;
}

QVariant AllocPoolModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role)
    {
        if (Qt::Horizontal == orientation)
        {
            static const char* headers[NCOLUMNS] = {
                "Allocation pool",
                "Allocation size",
                "Overhead",
                "Block count"
            };
            return QVariant(headers[section]);
        }
        else
        {
            return QVariant(section + 1);
        }
    }
    return QVariant();
}

QVariant AllocPoolModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && profileSession != nullptr)
    {
        int row = index.row();
        int clm = index.column();
        if (Qt::DisplayRole == role)
        {
            const AllocPoolStat& stat = curValues[row];
            switch (clm)
            {
            case CLM_NAME:
                return QVariant(profileSession->AllocPoolName(row).c_str());
            case CLM_ALLOC_APP:
                return FormatNumberWithDigitGroups(stat.allocByApp).c_str();
            case CLM_ALLOC_TOTAL:
                return FormatNumberWithDigitGroups(stat.allocTotal).c_str();
            case CLM_NBLOCKS:
                return FormatNumberWithDigitGroups(stat.blockCount).c_str();
            default:
                break;
            }
        }
        else if (Qt::BackgroundRole == role)
        {
            if (!poolColors.empty())
            {
                size_t index = static_cast<size_t>(row) % poolColors.size();
                return QVariant(poolColors[index]);
            }
        }
        else if (Qt::ForegroundRole == role)
        {
            if (!poolColors.empty())
                return QColor(Qt::white);
        }
    }
    return QVariant();
}

void AllocPoolModel::BeginNewProfileSession(ProfilingSession* profSession)
{
    beginResetModel();
    profileSession = profSession;
    curValues.clear();
    curValues.resize(profileSession->AllocPoolCount());
    endResetModel();
}

void AllocPoolModel::SetCurrentValues(const MemoryStatItem& item)
{
    Q_ASSERT(curValues.size() == item.PoolStat().size());
    timestamp = item.Timestamp();
    std::copy(item.PoolStat().begin(), item.PoolStat().end(), curValues.begin());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void AllocPoolModel::SetPoolColors(const DAVA::Vector<QColor>& colors)
{
    poolColors = colors;
}
