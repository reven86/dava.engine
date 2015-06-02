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


#include "QtPropertyModelFiltering.h"
#include "QtPropertyData.h"

QtPropertyModelFiltering::QtPropertyModelFiltering(QtPropertyModel *_propModel, QObject *parent /* = NULL */)
: QSortFilterProxyModel(parent)
, propModel(_propModel)
{
	setSourceModel(propModel);

	QObject::connect(propModel, SIGNAL(PropertyEdited(const QModelIndex&)), this, SLOT(OnPropertyEdited(const QModelIndex&)));
	QObject::connect(propModel, SIGNAL(PropertyChanged(const QModelIndex&)), this, SLOT(OnPropertyChanged(const QModelIndex&)));
	QObject::connect(propModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(OnRowsInserted(const QModelIndex &, int, int)));
	QObject::connect(propModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(OnRowsRemoved(const QModelIndex &, int, int)));
}

bool QtPropertyModelFiltering::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if(NULL != propModel)
	{
		// check self accept
		if(selfAcceptRow(sourceRow, sourceParent))
		{
			return true;
		}

		//accept if any of the parents is accepted
		QModelIndex parent = sourceParent;
		while(parent.isValid())
		{
			if(selfAcceptRow(parent.row(), parent.parent()))
			{
				return true;
			}

			parent = parent.parent();
		}

		// accept if any child is accepted
		if(childrenAcceptRow(sourceRow, sourceParent))
		{
			return true;
		}
	}

	return false;
}

bool QtPropertyModelFiltering::selfAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool QtPropertyModelFiltering::childrenAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool ret = false;

	QModelIndex index = propModel->index(sourceRow, 0, sourceParent);
	if(propModel->rowCount(index) > 0)
	{
		for(int i = 0; i < propModel->rowCount(index); i++)
		{
			if(selfAcceptRow(i, index) || childrenAcceptRow(i, index))
			{
				ret = true;
				break;
			}
		}
	}

	return ret;
}

void QtPropertyModelFiltering::OnPropertyEdited(const QModelIndex &index)
{
	emit PropertyEdited(mapFromSource(index));
}

void QtPropertyModelFiltering::OnPropertyChanged(const QModelIndex &index)
{
	emit PropertyChanged(mapFromSource(index));
}

void QtPropertyModelFiltering::OnRowsInserted(const QModelIndex &, int, int)
{
	invalidate();
}

void QtPropertyModelFiltering::OnRowsRemoved(const QModelIndex &, int, int)
{
	invalidate();
}
