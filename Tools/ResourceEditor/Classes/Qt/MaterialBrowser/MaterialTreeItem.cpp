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



#include "MaterialBrowser/MaterialTreeItem.h"

MaterialTreeItem::MaterialTreeItem(DAVA::Material *material, MaterialTreeItem *parent /* = NULL */)
	: parentItem(parent)
	, material(material)
{
	type = TYPE_MATERIAL;
	if(NULL != material)
	{
		name = material->GetName().c_str();
	}

	if(NULL != parent)
	{
		parent->AddChild(this);
	}
}

MaterialTreeItem::MaterialTreeItem(QString folderName, MaterialTreeItem *parent /* = NULL */)
	: parentItem(parent)
	, material(NULL)
{
	type = TYPE_FOLDER;
	name = folderName;

	if(NULL != parent)
	{
		parent->AddChild(this);
	}
}

MaterialTreeItem::~MaterialTreeItem()
{
	qDeleteAll(childItems);
}

void MaterialTreeItem::AddChild(MaterialTreeItem *child)
{
	childItems.append(child);

	if(NULL != child)
	{
		child->parentItem = this;
	}
}

MaterialTreeItem* MaterialTreeItem::Child(int row) const
{
	MaterialTreeItem* item = NULL;

	if(row < childItems.count())
	{
		item = childItems.value(row);
	}

	return item;
}

MaterialTreeItem* MaterialTreeItem::Parent() const
{
	return parentItem;
}

int MaterialTreeItem::Row() const
{
	int row = 0;

	if (parentItem)
	{
		row = parentItem->childItems.indexOf(const_cast<MaterialTreeItem*>(this));
	}

	return 0;
}

int MaterialTreeItem::ChildCount() const
{
	return childItems.count();
}

int MaterialTreeItem::ColumnCount() const
{
	// TODO: 
	return 1;
}

QVariant MaterialTreeItem::Data(int column) const
{
	return QVariant(name);
}

const DAVA::Material* MaterialTreeItem::Material() const
{
	return material;
}
