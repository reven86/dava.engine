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


#include "LogFilterModel.h"

#include <QDebug>

#include "LogModel.h"


LogFilterModel::LogFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

LogFilterModel::~LogFilterModel()
{
}

void LogFilterModel::SetFilters(const QSet<int>& _filters)
{
    if (filters != _filters)
    {
        filters = _filters;
        invalidateFilter();
    }
}

void LogFilterModel::SetFilterString(const QString& _filter)
{
    if (_filter != filterText)
    {
        filterText = _filter;
        invalidateFilter();
    }
}

bool LogFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex source = sourceModel()->index(source_row, 0, source_parent);

    bool isAcceptedByText = true;
    if (!filterText.isEmpty())
    {
        const QString text = source.data(Qt::DisplayRole).toString();
        if (!text.contains(filterText, Qt::CaseInsensitive))
        {
            isAcceptedByText = false;
        }
    }

    bool wasSet = false;
    const int level = source.data(LogModel::LEVEL_ROLE).toInt(&wasSet);
    const bool isAcceptedByLevel = (wasSet && filters.contains(level));

    return isAcceptedByLevel && isAcceptedByText;
}