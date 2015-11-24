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

#include "Qt/DeviceInfo/MemoryTool/Models/BlockGroupModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/DataFormat.h"

using namespace DAVA;

BlockGroupModel::BlockGroupModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

BlockGroupModel::~BlockGroupModel() = default;

void BlockGroupModel::SetBlockGroups(const DAVA::Vector<BlockGroup>* groups)
{
    beginResetModel();
    this->groups = groups;
    endResetModel();
}

int BlockGroupModel::rowCount(const QModelIndex& parent) const
{
    return groups != nullptr ? static_cast<int>(groups->size()) : 0;
}

int BlockGroupModel::columnCount(const QModelIndex& parent) const
{
    if (groups != nullptr && !groups->empty())
    {
        return static_cast<int>(groups->operator[](0).blockLink.linkCount);
    }
    return 0;
}

QVariant BlockGroupModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        const BlockGroup& curGroup = groups->operator[](index.row());
        if (Qt::DisplayRole == role)
        {
            int column = index.column();
            QString s = QString("size=%1;nblocks=%2")
                        .arg(FormatNumberWithDigitGroups(curGroup.blockLink.allocSize[column]).c_str())
                        .arg(FormatNumberWithDigitGroups(curGroup.blockLink.blockCount[column]).c_str());
            if (0 == column)
            {
                return QString("%1;%2")
                .arg(curGroup.title.c_str())
                .arg(s);
            }
            else
                return s;
        }
        else if (ROLE_GROUP_POINTER == role)
        {
            return QVariant::fromValue(const_cast<void*>(static_cast<const void*>(&curGroup)));
        }
    }
    return QVariant();
}

QVariant BlockGroupModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && Qt::Horizontal == orientation)
    {
        return QString("Block grouping");
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}
