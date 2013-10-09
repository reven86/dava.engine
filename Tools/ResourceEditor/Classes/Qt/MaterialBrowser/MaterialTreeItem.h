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



#ifndef __MATERIAL_ITEM_H__
#define __MATERIAL_ITEM_H__

#include <QList>
#include <QVariant>

#include "DAVAEngine.h"

class MaterialTreeItem : public DAVA::BaseObject
{
public:
	typedef enum ItemType
	{
		TYPE_MATERIAL,
		TYPE_FOLDER
	} ItemType;

	MaterialTreeItem(DAVA::Material *material, MaterialTreeItem *parent = NULL);
	MaterialTreeItem(QString folderName, MaterialTreeItem *parent = NULL);
	~MaterialTreeItem();

	void AddChild(MaterialTreeItem *child);

	MaterialTreeItem *Child(int row) const;
	MaterialTreeItem *Parent() const;
	int Row() const;

	int ChildCount() const;
	int ColumnCount() const;

	QVariant Data(int column) const;
	ItemType Type() const;

	const DAVA::Material* Material() const;

private:
	MaterialTreeItem *parentItem;
	QList<MaterialTreeItem*> childItems;

	QString name;
	const DAVA::Material *material;
	ItemType type;
};

#endif // __MATERIAL_ITEM_H__
