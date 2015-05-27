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

#include "SnapshotListModel.h"

#include "../ProfilingSession.h"

using namespace DAVA;

SnapshotListModel::SnapshotListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

SnapshotListModel::~SnapshotListModel()
{}

int SnapshotListModel::rowCount(const QModelIndex& parent) const
{
    return profileSession != nullptr ? static_cast<int>(profileSession->SnapshotCount())
                                     : 0;
}

QVariant SnapshotListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role)
    {
        if (Qt::Horizontal == orientation)
        {
            return QVariant("Memory snapshots");
        }
        else
        {
            return QVariant(section + 1);
        }
    }
    return QVariant();
}

QVariant SnapshotListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && profileSession != nullptr)
    {
        int row = index.row();
        const MemorySnapshot& snapshot = profileSession->Snapshot(row);
        if (Qt::DisplayRole == role)
        {
            String filename = snapshot.FileName().GetFilename();
            return QString("%1 - %2")
                .arg(snapshot.Timestamp() / 1000)
                .arg(filename.c_str());
        }
        else if (Qt::BackgroundRole == role)
        {
            // TODO: maybe colorize loaded snapshots
        }
    }
    return QVariant();
}

void SnapshotListModel::BeginNewProfileSession(ProfilingSession* profSession)
{
    beginResetModel();
    profileSession = profSession;
    endResetModel();
}

void SnapshotListModel::NewSnapshotArrived()
{
    beginResetModel();
    endResetModel();
}
