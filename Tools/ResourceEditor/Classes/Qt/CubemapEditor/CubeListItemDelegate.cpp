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


#include "CubemapEditor/CubeListItemDelegate.h"
#include "CubemapEditor/CubemapUtils.h"
#include "Render/Texture.h"
#include "Render/TextureDescriptor.h"

#include <QApplication>
#include <QMouseEvent>

const int FACE_IMAGE_BORDER = 5;
const int LIST_ITEM_OFFSET = 1;
const int CHECKBOX_CONTROL_X_OFFSET = 5;
const int CHECKBOX_WIDTH = 16;
const int CHECKBOX_HEIGHT = 16;
const int CHECKBOX_X_OFFSET = CHECKBOX_WIDTH + CHECKBOX_CONTROL_X_OFFSET + 2;

const QColor SELECTION_BORDER_COLOR = QColor(0, 0, 0, 50);
const int SELECTION_COLOR_ALPHA = 100;

const QString FONT_NAMES[] = {QString("Arial"), QString("Arial"), QString("Arial")};
static int FONT_SIZES[] = {16, 11, 11};
const int FONT_STYLES[] = {QFont::Bold, QFont::Normal, QFont::Normal};
const int FONT_ATTRIBUTE_COUNT = 3;

const int TITLE_ITEM_DATA = 0;
const int DESCRIPTION_ITEM_DATA = 1;
const int FACE_SIZE_ITEM_DATA = 2;

const int TARGET_TEXT_SIZE_PX[] = {17, 12, 12};

CubeListItemDelegate::CubeListItemDelegate(QSize thumbSize, QObject *parent)
{
	thumbnailSize = thumbSize;
	
	//HACK: use this hack in order to achieve consistent text size on screens with different DPI
	for(int i = 0; i < FONT_ATTRIBUTE_COUNT; ++i)
	{
		QFontMetrics fontMetrics = QFontMetrics(QFont(FONT_NAMES[i], FONT_SIZES[i], FONT_STYLES[i]));
		FONT_SIZES[i] = (FONT_SIZES[i] * TARGET_TEXT_SIZE_PX[i]) / fontMetrics.height();;
	}

	int textTotalHeight = 0;
	for(int i = 0; i < FONT_ATTRIBUTE_COUNT; ++i)
	{
		QFontMetrics fontMetrics = QFontMetrics(QFont(FONT_NAMES[i], FONT_SIZES[i], FONT_STYLES[i]));
		textTotalHeight += GetAdjustedTextHeight(fontMetrics.height());
	}
	
	itemHeight = std::max(textTotalHeight, thumbnailSize.height() + 2 * FACE_IMAGE_BORDER + 2 * LIST_ITEM_OFFSET + 1);
}

CubeListItemDelegate::~CubeListItemDelegate()
{
	ClearCache();
}

int CubeListItemDelegate::GetAdjustedTextHeight(int baseHeight) const
{
	return baseHeight + FACE_IMAGE_BORDER;
}

void CubeListItemDelegate::ClearCache()
{
	for(std::map<std::string, ListCacheItem>::iterator i = itemCache.begin();
		i != itemCache.end();
		++i)
	{
		ListCacheItem& cacheItem = i->second;
		for(size_t iconIndex = 0; iconIndex < cacheItem.icons.size(); ++iconIndex)
		{
			delete cacheItem.icons[iconIndex];
		}
	}
	
	itemCache.clear();
}

void CubeListItemDelegate::UpdateCache(DAVA::Vector<CubeListItemDelegate::ListItemInfo>& fileList)
{
	ClearCache();
	
	for(size_t i = 0; i < fileList.size(); ++i)
	{
		ListItemInfo& itemInfo = fileList.at(i);
		
		ListCacheItem cacheItem;
		
		cacheItem.icons.insert(cacheItem.icons.end(), itemInfo.icons.begin(), itemInfo.icons.end());
		cacheItem.actualSize.insert(cacheItem.actualSize.end(), itemInfo.actualSize.begin(), itemInfo.actualSize.end());
		cacheItem.valid = itemInfo.valid;
		itemCache[itemInfo.path.GetAbsolutePathname()] = cacheItem;
	}
}

void CubeListItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	int y = option.rect.y();
	int totalImageStripWidth = (thumbnailSize.width() + FACE_IMAGE_BORDER) * CubemapUtils::GetMaxFaces();
	int faceStartX = option.rect.width() - totalImageStripWidth - FACE_IMAGE_BORDER;

	QRect r = option.rect;

	QPen fontPen(QColor::fromRgb(51, 51, 51), 1, Qt::SolidLine);
	
	//BACKGROUND ALTERNATING COLORS
	bool selected = index.data(Qt::CheckStateRole).toBool();
	QString displayText = index.data(Qt::DisplayRole).toString();
	QString title = index.data(CUBELIST_DELEGATE_ITEMFILENAME).toString();
	QString description = index.data(CUBELIST_DELEGATE_ITEMFULLPATH).toString();
	
	std::map<std::string, ListCacheItem>::const_iterator itemIter = itemCache.find(description.toStdString());
	
	QColor regularColor = (index.row() % 2) ? Qt::white : QColor(240, 240, 240);
	regularColor = (selected) ? QColor(255, 255, 224) : regularColor;
	
	if(itemIter != itemCache.end())
	{
		regularColor = (itemIter->second.valid) ? regularColor : QColor(249, 204, 202);
	}
	else
	{
		regularColor = QColor(249, 204, 202);
	}

	painter->setBrush(regularColor);
	painter->drawRect(r);
	
	painter->setPen(fontPen);
	
	int textStartY = y;
	//TITLE
	painter->setFont(QFont(FONT_NAMES[TITLE_ITEM_DATA], FONT_SIZES[TITLE_ITEM_DATA], FONT_STYLES[TITLE_ITEM_DATA]));
	QFontMetrics fontMetrics = painter->fontMetrics();
	QRect textRect = fontMetrics.boundingRect(title);
	painter->drawText(FACE_IMAGE_BORDER + CHECKBOX_X_OFFSET, textStartY + GetAdjustedTextHeight(textRect.height()), title);
	textStartY += GetAdjustedTextHeight(textRect.height());
	
	//DESCRIPTION
	painter->setFont(QFont(FONT_NAMES[DESCRIPTION_ITEM_DATA], FONT_SIZES[DESCRIPTION_ITEM_DATA], FONT_STYLES[DESCRIPTION_ITEM_DATA]));
	fontMetrics = painter->fontMetrics();
	textRect = fontMetrics.boundingRect(description);
	painter->drawText(FACE_IMAGE_BORDER + CHECKBOX_X_OFFSET, textStartY + GetAdjustedTextHeight(textRect.height()), description);
	textStartY += GetAdjustedTextHeight(textRect.height());
	
	int imageWidth = 0;
	int imageHeight = 0;
	if(itemIter != itemCache.end())
	{
		const ListCacheItem& cacheItem = itemIter->second;
		for(size_t iconIndex = 0; iconIndex < cacheItem.icons.size(); ++iconIndex)
		{
			painter->drawImage(QPoint(faceStartX + iconIndex * (thumbnailSize.width() + FACE_IMAGE_BORDER), y + FACE_IMAGE_BORDER),
							   *cacheItem.icons[iconIndex]);
		}
		
		if(cacheItem.actualSize.size() > 0)
		{
			imageWidth = cacheItem.actualSize[0].width();
			imageHeight = cacheItem.actualSize[0].height();
		}
	}
		
	//FACE SIZE
	QString sizeInfo = QString("%1x%2").arg(QString().setNum(imageWidth), QString().setNum(imageHeight));
	painter->setFont(QFont(FONT_NAMES[FACE_SIZE_ITEM_DATA], FONT_SIZES[FACE_SIZE_ITEM_DATA], FONT_STYLES[FACE_SIZE_ITEM_DATA]));
	fontMetrics = painter->fontMetrics();
	textRect = fontMetrics.boundingRect(description);
	painter->drawText(FACE_IMAGE_BORDER + CHECKBOX_X_OFFSET, textStartY + GetAdjustedTextHeight(textRect.height()), sizeInfo);
	
	//draw checkbox
	QStyleOptionButton checkboxButton;
	checkboxButton.state |= QStyle::State_Enabled;
	QStyle::StateFlag checkState = (index.data(Qt::CheckStateRole).toBool()) ? QStyle::State_On : QStyle::State_Off;
	checkboxButton.state |= checkState;
	checkboxButton.rect = GetCheckBoxRect(option);
	
	QApplication::style()->drawControl(QStyle::CE_CheckBox,
									   &checkboxButton,
									   painter);
	
	// draw selected item
	if(option.state & QStyle::State_Selected)
	{
		QBrush br = option.palette.highlight();
		QColor cl = br.color();
		cl.setAlpha(SELECTION_COLOR_ALPHA);
		br.setColor(cl);
		painter->setBrush(br);
		painter->setPen(SELECTION_BORDER_COLOR);
		painter->drawRect(r);
	}
}

QSize CubeListItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	return QSize(option.rect.width(), itemHeight);
}

bool CubeListItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	bool result = false;
	const QEvent::Type eventType = event->type();
	if((eventType == QEvent::MouseButtonRelease))
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if(mouseEvent->button() == Qt::LeftButton &&
			GetCheckBoxRect(option).contains(mouseEvent->pos()))
		{
			bool checked = model->data(index, Qt::CheckStateRole).toBool();
			result = model->setData(index, !checked, Qt::CheckStateRole);
			
			if(result)
			{
				emit OnItemCheckStateChanged(index);
			}
		}
	}
	else if(eventType == QEvent::MouseButtonDblClick)
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if(!GetCheckBoxRect(option).contains(mouseEvent->pos()))
		{
			emit OnEditCubemap(index);
		}
	}

	return result;
}

QRect CubeListItemDelegate::GetCheckBoxRect(const QStyleOptionViewItem & option) const
{
	return QRect(option.rect.left() + CHECKBOX_CONTROL_X_OFFSET, option.rect.top() + option.rect.height() / 2 - CHECKBOX_HEIGHT / 2, CHECKBOX_WIDTH, CHECKBOX_HEIGHT);
}