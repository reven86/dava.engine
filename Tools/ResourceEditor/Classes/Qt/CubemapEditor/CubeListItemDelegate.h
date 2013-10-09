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


#ifndef __CUBE_LIST_ITEM_DELEGATE_H__
#define __CUBE_LIST_ITEM_DELEGATE_H__

#include <QPainter>
#include <QAbstractItemDelegate>

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#define CUBELIST_DELEGATE_ITEMFULLPATH (Qt::UserRole)
#define CUBELIST_DELEGATE_ITEMFILENAME (Qt::UserRole + 1)

class CubeListItemDelegate : public QAbstractItemDelegate
{
	Q_OBJECT
	
protected:
	
	struct ListCacheItem
	{
		DAVA::Vector<QImage*> icons;
		DAVA::Vector<QSize> actualSize;
		bool valid;
	};
	
	QSize thumbnailSize;
	int itemHeight;
	std::map<std::string, ListCacheItem> itemCache;
	
private:
	
	int GetAdjustedTextHeight(int baseHeight) const;
	QRect GetCheckBoxRect(const QStyleOptionViewItem & option) const;
	
public:
	
	struct ListItemInfo
	{
		DAVA::FilePath path;
		DAVA::Vector<QImage*> icons;
		DAVA::Vector<QSize> actualSize;
		bool valid;
	};
	
public:
	
	CubeListItemDelegate(QSize thumbSize, QObject *parent = 0);
	virtual ~CubeListItemDelegate();
	
	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
		
	void ClearCache();
	void UpdateCache(DAVA::Vector<CubeListItemDelegate::ListItemInfo>& fileList);
	
signals:
	
	void OnEditCubemap(const QModelIndex &index);
	void OnItemCheckStateChanged(const QModelIndex &index);

};

#endif /* defined(__CUBE_LIST_ITEM_DELEGATE_H__) */
