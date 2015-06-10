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


/*
#include "QtPropertyItem.h"
#include "QtPropertyData.h"
#include "QtPropertyModel.h"

QtPropertyItem::QtPropertyItem()
	: QStandardItem()
	, itemData(NULL)
	, parentName(NULL)
	, itemDataDeleteByParent(false)
{
}

QtPropertyItem::QtPropertyItem(QtPropertyData* data, QtPropertyItem *name)
	: QStandardItem()
	, itemData(data)
	, parentName(name)
	, itemDataDeleteByParent(false)
{
	if(NULL != data && NULL != parentName)
	{
		for (int i = 0; i < data->ChildCount(); ++i)
		{
			QPair<QString, QtPropertyData*> childData = data->ChildGet(i);
			ChildAdd(childData.first, childData.second);
		}

		ApplyDataFlags();
		ApplyNameStyle();

		QObject::connect(data, SIGNAL(ChildRemoving(const QString &, QtPropertyData *)), this, SLOT(DataChildRemoving(const QString &, QtPropertyData *)));
		QObject::connect(data, SIGNAL(ChildAdded(const QString &, QtPropertyData *)), this, SLOT(DataChildAdded(const QString &, QtPropertyData *)));
		QObject::connect(data, SIGNAL(ValueChanged(QtPropertyData::ValueChangeReason)), this, SLOT(DataValueChanged(QtPropertyData::ValueChangeReason)));
		QObject::connect(data, SIGNAL(FlagsChanged()), this, SLOT(DataFlagsChanged()));
	}
}

QtPropertyItem::QtPropertyItem(const QVariant &value)
	: QStandardItem()
	, itemData(NULL)
	, parentName(NULL)
	, itemDataDeleteByParent(false)
{
	itemData = new QtPropertyData(value);
}

QtPropertyItem::~QtPropertyItem()
{
	if(NULL != itemData && !itemDataDeleteByParent)
	{
		delete itemData;
		itemData = NULL;
	}
}

QtPropertyData* QtPropertyItem::GetPropertyData() const
{
	return itemData;
}

QtPropertyItem* QtPropertyItem::GetParentNameItem() const
{
	return parentName;
}

int QtPropertyItem::type() const
{
	return QStandardItem::UserType + 1;
}

QVariant QtPropertyItem::data(int role) const
{
	QVariant v;

	if(NULL != itemData)
	{
		switch(role)
		{
		case Qt::DecorationRole:
			v = itemData->GetIcon();
			break;
		case Qt::DisplayRole:
		case Qt::ToolTipRole:
		case Qt::EditRole:
			v = itemData->GetAlias();
			if(!v.isValid())
			{
				v = itemData->GetValue();
			}
			break;
		case PropertyDataRole:
			v = qVariantFromValue(GetPropertyData());
			break;
		default:
			break;
		}
	}
	
	if(v.isNull())
	{
		v = QStandardItem::data(role);
	}

	return v;
}

void QtPropertyItem::setData(const QVariant & value, int role)
{
	switch(role)
	{
	case Qt::EditRole:
		if(NULL != itemData)
		{
			itemData->SetValue(value, QtPropertyData::VALUE_EDITED);
		}
		break;
	default:
		QStandardItem::setData(value, role);
		break;
	}
}

bool QtPropertyItem::Update()
{
	bool ret = false;

	if(NULL != itemData)
	{
		return itemData->UpdateValue();
	}

	return ret;
}

void QtPropertyItem::ChildAdd(const QString &key, QtPropertyData* data)
{
	if(NULL != parentName)
	{
		QList<QStandardItem *> subItems;
		QtPropertyItem *subName = new QtPropertyItem(key);
		QtPropertyItem *subValue = new QtPropertyItem(data, subName);

		subValue->itemDataDeleteByParent = true;
		// subValue->setFont(QFont("Courier"));
		subName->setEditable(false);

		subItems.append(subName);
		subItems.append(subValue);

		parentName->appendRow(subItems);
	}
}

void QtPropertyItem::ChildRemove(QtPropertyData* data)
{
	if(NULL != parentName)
	{
		for(int i = 0; i < parentName->rowCount(); ++i)
		{
			QtPropertyItem* childItem = (QtPropertyItem*) parentName->child(i, 1);
			if(NULL != childItem && childItem->itemData == data)
			{
				childItem->itemData = NULL;
				parentName->removeRow(i);
			}
		}
	}
}

void QtPropertyItem::ApplyNameStyle()
{
	QFont curFont = parentName->font();

	// if there are childs, set bold font
	if(NULL != parentName && parentName->rowCount() > 0)
	{
		curFont.setBold(true);
	}
	else
	{
		curFont.setBold(false);
	}

	parentName->setFont(curFont);
}

void QtPropertyItem::ApplyDataFlags()
{
	if(NULL != itemData)
	{
		int dataFlags = itemData->GetFlags();

		// changing Checkable flag will cause model
		// to emit itemChanged signal, but we don't want this signal to be emited

		bool oldState = false;
		if(NULL != model())
		{
			oldState = model()->blockSignals(true);
		}
			
		setCheckable(dataFlags & QtPropertyData::FLAG_IS_CHECKABLE);
		if(dataFlags & QtPropertyData::FLAG_IS_CHECKABLE)
		{
			if(itemData->GetValue().toBool())
			{
				setCheckState(Qt::Checked);
			}
			else
			{
				setCheckState(Qt::Unchecked);
			}
		}

		if(NULL != model())
		{
			model()->blockSignals(oldState);
		}

		setEnabled(!(dataFlags & QtPropertyData::FLAG_IS_DISABLED));
		setEditable(!(dataFlags & QtPropertyData::FLAG_IS_NOT_EDITABLE));
	}
}

void QtPropertyItem::DataChildAdded(const QString &key, QtPropertyData *data)
{
	ChildAdd(key, data);
	ApplyNameStyle();
}

void QtPropertyItem::DataChildRemoving(const QString &key, QtPropertyData *data)
{
	ChildRemove(data);
	ApplyNameStyle();
}

void QtPropertyItem::DataFlagsChanged()
{
	ApplyDataFlags();
}

void QtPropertyItem::DataValueChanged(QtPropertyData::ValueChangeReason reason)
{
	ApplyDataFlags();

	if(reason == QtPropertyData::VALUE_EDITED)
	{
		QtPropertyModel *propModel = (QtPropertyModel *) model();
		propModel->EmitDataEdited(QtPropertyRow(this->parentName, this));
	}
	else
	{
		emitDataChanged();
	}
}
*/
