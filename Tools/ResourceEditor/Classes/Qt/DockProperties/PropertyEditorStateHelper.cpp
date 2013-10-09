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


#include "PropertyEditorStateHelper.h"

PropertyEditorStateHelper::PropertyEditorStateHelper(QTreeView* treeView, QtPropertyModel* model) :
	QTreeViewStateHelper(treeView)
{
	this->model = model;
}

void PropertyEditorStateHelper::SaveTreeViewState(bool needCleanupStorage)
{
	// Need to cleanup the full paths cache before the save.
	this->fullPathsCache.clear();
	DAVA::QTreeViewStateHelper<QString>::SaveTreeViewState(needCleanupStorage);
}

QString PropertyEditorStateHelper::GetPersistentDataForModelIndex(const QModelIndex &modelIndex)
{
	if (!this->model)
	{
		return QString();
	}

	// Calculate the full path up to the root. An optimization is required here - since this
	// method is called recursively, we must already know the full path to the parent, just
	// append the child name to it.
	QString fullPath;
	QStandardItem* item = model->itemFromIndex(modelIndex);
	if (!item)
	{
		return fullPath;
	}

	if (item->parent())
	{
		DAVA::Map<QStandardItem*, QString>::iterator parentIter = fullPathsCache.find(item->parent());
		if (parentIter != fullPathsCache.end())
		{
			fullPath = parentIter->second;
		}
	}

	// Append the current node name and store the full path to it in the cache.
	fullPath = fullPath + "//" + item->data(Qt::DisplayRole).toString();
	fullPathsCache[item] = fullPath;

	return fullPath;
}
