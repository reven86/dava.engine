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


#include "FilteredPackageModel.h"
#include <QColor>
#include "DAVAEngine.h"

FilteredPackageModel::FilteredPackageModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    
}

FilteredPackageModel::~FilteredPackageModel()
{
    
}

bool FilteredPackageModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
    {
        return true;
    }
    return hasAcceptedChildren(sourceRow, sourceParent);
}

QVariant FilteredPackageModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextColorRole)
    {
        QRegExp regExp = filterRegExp();
        if (!regExp.isEmpty())
        {
            QModelIndex srcIndex = mapToSource(index);
            if (!srcIndex.data(filterRole()).toString().contains(regExp))
                return QColor(Qt::lightGray);
        }
    }
    
    return QSortFilterProxyModel::data(index, role);
}

bool FilteredPackageModel::hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex item = sourceModel()->index(sourceRow, 0, sourceParent);
    int rowCount = sourceModel()->rowCount(item);
    
    if (rowCount == 0)
        return false;
    
    for (int row = 0; row < rowCount; ++row)
    {
        if (filterAcceptsRow(row, item))
            return true;
    }
    
    return false;
}
