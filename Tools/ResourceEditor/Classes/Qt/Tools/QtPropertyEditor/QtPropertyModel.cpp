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



#include "QtPropertyModel.h"
#include "QtPropertyItem.h"

QtPropertyModel::QtPropertyModel(QObject* parent /* = 0 */)
	: QStandardItemModel(parent)
{
	QStringList headerLabels;

	headerLabels.append("Name");
	headerLabels.append("Value");

	setColumnCount(2);
	setHorizontalHeaderLabels(headerLabels);

	QObject::connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(OnItemChanged(QStandardItem *)));
}

QtPropertyModel::~QtPropertyModel()
{ }

QPair<QtPropertyItem*, QtPropertyItem*> QtPropertyModel::AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent /*= NULL*/)
{
	QList<QStandardItem *> items;
	QStandardItem* root = (QStandardItem *) parent;

	QtPropertyItem *newPropertyName = new QtPropertyItem(name);
	QtPropertyItem *newPropertyValue = new QtPropertyItem(data, newPropertyName);

	newPropertyName->setEditable(false);

	items.append(newPropertyName);
	items.append(newPropertyValue);

	if(NULL == root)
	{
		root = invisibleRootItem();
	}

	root->appendRow(items);

	return QPair<QtPropertyItem*, QtPropertyItem*>(newPropertyName, newPropertyValue);
}

QPair<QtPropertyItem*, QtPropertyItem*> QtPropertyModel::GetProperty(const QString &name, QtPropertyItem* parent/* = NULL*/)
{
	QPair<QtPropertyItem*, QtPropertyItem*> ret(NULL, NULL);

    QStandardItem* root = (QStandardItem *) parent;
	if(NULL == root)
	{
		root = invisibleRootItem();
	}

    for(DAVA::int32 r = 0; r < root->rowCount(); ++r)
    {
        QtPropertyItem *keyItem = (QtPropertyItem *) root->child(r, 0);
        if(keyItem->GetPropertyData()->GetValue().toString() == name)
        {
            QtPropertyItem *dataItem = (QtPropertyItem *) root->child(r, 1);

			ret.first = keyItem;
			ret.second = dataItem;

			break;
        }
    }
    
    return ret;
}

void QtPropertyModel::RemoveProperty(QtPropertyItem* item)
{
	removeRow(indexFromItem(item).row());
}

void QtPropertyModel::RemovePropertyAll()
{
	removeRows(0, rowCount());
}

void QtPropertyModel::RefreshAll()
{
	emit dataChanged(index(0, 0), index(rowCount(), 1));
}

void QtPropertyModel::OnItemChanged(QStandardItem* item)
{
	QtPropertyItem *propItem = (QtPropertyItem *) item;

	if(NULL != propItem)
	{
		if(propItem->isCheckable())
		{
			propItem->GetPropertyData()->SetValue(QVariant(propItem->checkState() == Qt::Checked));
		}
	}
}
