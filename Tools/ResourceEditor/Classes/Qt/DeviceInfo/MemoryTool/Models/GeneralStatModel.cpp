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

#include "GeneralStatModel.h"

#include "../ProfilingSession.h"
#include "DataFormat.h"

using namespace DAVA;

GeneralStatModel::GeneralStatModel(QObject* parent)
    : QAbstractTableModel(parent)
    , profileSession(nullptr)
    , timestamp(0)
    , curValues()
{}

GeneralStatModel::~GeneralStatModel()
{}

int GeneralStatModel::rowCount(const QModelIndex& parent) const
{
    return NROWS;
}

int GeneralStatModel::columnCount(const QModelIndex& parent) const
{
    return NCOLUMNS;
}

QVariant GeneralStatModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role)
    {
        if (Qt::Horizontal == orientation)
        {
            static const char* headers[NCOLUMNS] = {
                "Value"
            };
            return QVariant(headers[section]);
        }
        else
        {
            static const char* headers[NROWS] = {
                "Internal allocation size",
                "Total internal allocation size",
                "Internal block count",
                "Ghost allocation size",
                "Ghost block count",
                "Total allocation count"
            };
            return QVariant(headers[section]);
        }
    }
    return QVariant();
}

QVariant GeneralStatModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && profileSession != nullptr)
    {
        int row = index.row();
        int clm = index.column();
        if (Qt::DisplayRole == role)
        {
            switch (row)
            {
            case ROW_ALLOC_INTERNAL:
                return FormatNumberWithDigitGroups(curValues.allocInternal).c_str();
            case ROW_ALLOC_INTERNAL_TOTAL:
                    return FormatNumberWithDigitGroups(curValues.allocInternalTotal).c_str();
            case ROW_NBLOCKS_INTERNAL:
                return FormatNumberWithDigitGroups(curValues.internalBlockCount).c_str();
            case ROW_ALLOC_GHOST:
                return FormatNumberWithDigitGroups(curValues.ghostSize).c_str();
            case ROW_NBLOCKS_GHOST:
                return FormatNumberWithDigitGroups(curValues.ghostBlockCount).c_str();
            case ROW_TOTAL_ALLOC_COUNT:
                return FormatNumberWithDigitGroups(curValues.nextBlockNo).c_str();
            default:
                break;
            }
        }
    }
    return QVariant();
}

void GeneralStatModel::BeginNewProfileSession(ProfilingSession* profSession)
{
    beginResetModel();
    profileSession = profSession;
    curValues = GeneralAllocStat();
    endResetModel();
}

void GeneralStatModel::SetCurrentValues(const MemoryStatItem& item)
{
    timestamp = item.Timestamp();
    curValues = item.GeneralStat();
    emit dataChanged(QModelIndex(), QModelIndex());
}
