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


#include <QSet>

#include "MaterialItem.h"
#include "MaterialModel.h"

MaterialItem::MaterialItem(DAVA::NMaterial * _material)
    : QStandardItem()
    , material(_material)
{
	static QIcon qualityMaterialIcon(QString::fromUtf8(":/QtLibraryIcons/lodmaterial.png"));
	static QIcon userMaretialIcon(QString::fromUtf8(":/QtIcons/materialeditor.png"));
	static QIcon instanceMaretialIcon(QString::fromUtf8(":/QtIcons/materialeditor.png"));

	DVASSERT(material);
    
	setEditable(false);
	setText(material->GetMaterialName().c_str());
    setData(QVariant::fromValue<DAVA::NMaterial *>(material));
    
    if(material->IsSwitchable())
    {
		setIcon(qualityMaterialIcon);
    }
    else
    {
		setIcon(userMaretialIcon);
    }

	Sync();
}

MaterialItem::~MaterialItem()
{ }

QVariant MaterialItem::data(int role) const
{
    return QStandardItem::data(role);
}

DAVA::NMaterial * MaterialItem::GetMaterial() const
{
	return material;
}

void MaterialItem::Sync()
{
	QSet<DAVA::NMaterial *> materialsSet;

	// remember all entity childs
	for(int i = 0; i < material->GetChildrenCount(); ++i)
	{
		materialsSet.insert(material->GetChild(i));
	}

	// remove items, that are not in set
	for(int i = 0; i < rowCount(); ++i)
	{
		MaterialItem *childItem = (MaterialItem *) child(i);
		if(!materialsSet.contains(childItem->GetMaterial()))
		{
			removeRow(i);
			i--;
		}
	}

	materialsSet.clear();

	// add items, that are not in childs
	for(int row = 0, i = 0; i < material->GetChildrenCount(); ++i)
	{
		bool repeatStep;
		DAVA::NMaterial *childMaterial = material->GetChild(i);

		do
		{
			MaterialItem *item = (MaterialItem *) child(i);
			DAVA::NMaterial *itemMaterial = NULL;
			
			if(NULL != item)
			{
				item->GetMaterial();
			}

			repeatStep = false;

			// remove items that we already add
			while(materialsSet.contains(itemMaterial))
			{
				removeRow(row);

				item = (MaterialItem *) child(row);
				itemMaterial = item->GetMaterial();
			}

			// append entity that isn't in child items list
			if(NULL == item)
			{
				appendRow(new MaterialItem(childMaterial));
			}
			else if(childMaterial != itemMaterial)
			{
				// now we should decide what to do: remove item or insert it

				// calculate len until itemMaterial will be found in real entity childs
				int lenUntilRealEntity = 0;
				for(int j = i; j < material->GetChildrenCount(); ++j)
				{
					if(material->GetChild(j) == itemMaterial)
					{
						lenUntilRealEntity = j - i;
						break;
					}
				}

				// calculate len until current real entity child will be found in current item childs
				int lenUntilItem = 0;
				for(int j = i; j < rowCount(); ++j)
				{
					MaterialItem *itm = (MaterialItem *) child(j);

					if(NULL != itm && childMaterial == itm->GetMaterial())
					{
						lenUntilItem = j - i;
						break;
					}
				}

				if(lenUntilRealEntity >= lenUntilItem)
				{
					removeRow(row);
					repeatStep = true;
				}
				else
				{
					insertRow(row, new MaterialItem(childMaterial));
				}
			}
			else
			{
				item->Sync();
			}
		} 
		while(repeatStep);

		// remember that we add that entity
		materialsSet.insert(childMaterial);
		row++;
	}

	if(material->IsConfigMaterial())
	{
		setEnabled(rowCount() > 0);
	}
}




