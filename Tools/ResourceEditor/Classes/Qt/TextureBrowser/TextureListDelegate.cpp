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


#include "Base/BaseTypes.h"
#include "TextureListDelegate.h"
#include "TextureListModel.h"
#include "TextureCache.h"
#include "TextureConvertor.h"
#include "TextureBrowser.h"
#include <QPainter>
#include <QFileInfo>
#include <QEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QToolTip>
#include <QAbstractItemView>

#include "Main/QtUtils.h"

#include "Project/ProjectManager.h"

#define TEXTURE_PREVIEW_SIZE 80
#define TEXTURE_PREVIEW_SIZE_SMALL 24
#define BORDER_MARGIN 1
#define BORDER_COLOR QColor(0, 0, 0, 25)
#define SELECTION_BORDER_COLOR QColor(0, 0, 0, 50)
#define SELECTION_COLOR_ALPHA 100
#define INFO_TEXT_COLOR QColor(0, 0, 0, 100)
#define FORMAT_INFO_WIDTH 3
#define FORMAT_INFO_SPACING 1
#include <QDesktopServices>

TextureListDelegate::TextureListDelegate(QObject *parent /* = 0 */)
	: QAbstractItemDelegate(parent)
	, nameFont("Arial", 10, QFont::Bold)
	, nameFontMetrics(nameFont)
	, drawRule(DRAW_PREVIEW_BIG)
{
	QObject::connect(TextureCache::Instance(), SIGNAL(ThumbnailLoaded(const DAVA::TextureDescriptor *, const TextureInfo &)), this, SLOT(textureReadyThumbnail(const DAVA::TextureDescriptor *, const TextureInfo &)));
};

void TextureListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch(drawRule)
	{
	case DRAW_PREVIEW_SMALL:
		drawPreviewSmall(painter, option, index);
		break;

	case DRAW_PREVIEW_BIG:
	default:
		drawPreviewBig(painter, option, index);
		break;
	}
}

QSize TextureListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch(drawRule)
	{
	case DRAW_PREVIEW_SMALL:
		return QSize(option.rect.x(), TEXTURE_PREVIEW_SIZE_SMALL);
		break;

	case DRAW_PREVIEW_BIG:
	default:
		return QSize(option.rect.x(), TEXTURE_PREVIEW_SIZE);
		break;
	}
}

void TextureListDelegate::textureReadyThumbnail(const DAVA::TextureDescriptor *descriptor, const TextureInfo & images)
{
	if(nullptr != descriptor)
	{
		if(descriptorIndexes.contains(descriptor->pathname))
		{
			QModelIndex index = descriptorIndexes[descriptor->pathname];
			descriptorIndexes.remove(descriptor->pathname);
            
			// this will force item with given index to redraw
			emit sizeHintChanged(index);
		}
	}
}

void TextureListDelegate::setDrawRule(DrawRule rule)
{
	drawRule = rule;
}

void TextureListDelegate::drawPreviewBig(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const TextureListModel *curModel = (TextureListModel *) index.model();
	DAVA::TextureDescriptor *curTextureDescriptor = curModel->getDescriptor(index);

	if(nullptr != curTextureDescriptor)
	{
		DAVA::Texture *curTexture = curModel->getTexture(index);

		QString texturePath = curTextureDescriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str();
		QString textureName = QFileInfo(texturePath).fileName();
		QSize textureDimension = QSize();
		QString textureDataSize = 0;

		if(nullptr != curTexture)
		{
			textureDimension = QSize(curTexture->width, curTexture->height);
            textureDataSize = QString::fromStdString(SizeInBytesToString(TextureCache::Instance()->getThumbnailSize(curTextureDescriptor)));
		}

		painter->save();
		painter->setClipRect(option.rect);

		// draw border
		QRect borderRect = option.rect;
		borderRect.adjust(BORDER_MARGIN, BORDER_MARGIN, -BORDER_MARGIN, -BORDER_MARGIN);
		if(curModel->isHighlited(index))
		{
			// draw highlight
			painter->setBrush(option.palette.toolTipBase());
		}
		painter->setPen(BORDER_COLOR);
		painter->drawRect(borderRect);

		const QList<QImage>& images = TextureCache::Instance()->getThumbnail(curTextureDescriptor);
		if(images.size() > 0 &&
		   !images[0].isNull())
		{
			QSize imageSize = images[0].rect().size();
			int imageX =  option.rect.x() + (TEXTURE_PREVIEW_SIZE - imageSize.width())/2;
			int imageY =  option.rect.y() + (TEXTURE_PREVIEW_SIZE - imageSize.height())/2;
			painter->drawImage(QRect(QPoint(imageX, imageY), imageSize), images[0]);
		}
		else
		{
			// there is no image for this texture in cache
			// so load it async
            TextureConvertor::Instance()->GetThumbnail(curTextureDescriptor);
			descriptorIndexes.insert(curTextureDescriptor->pathname, index);
		}

		// draw formats info
		int infoLen = drawFormatInfo(painter, borderRect, curTexture, curTextureDescriptor);

		// draw text info
		{
			QRectF textRect = option.rect;
			textRect.adjust(TEXTURE_PREVIEW_SIZE, option.decorationSize.height() / 2, -infoLen, 0);

			QFont origFont = painter->font();
			painter->setPen(option.palette.text().color());
			painter->setFont(nameFont);
			painter->drawText(textRect, textureName);

			painter->setFont(origFont);
			painter->setPen(INFO_TEXT_COLOR);
			textRect.adjust(0, nameFontMetrics.height(), 0, 0);

            QString infoText = CreateInfoString(index);
			painter->drawText(textRect, infoText);
		}

		// draw selected item
		if(option.state & QStyle::State_Selected)
		{
			QBrush br = option.palette.highlight();
			QColor cl = br.color();
			cl.setAlpha(SELECTION_COLOR_ALPHA);
			br.setColor(cl);
			painter->setBrush(br);
			painter->setPen(SELECTION_BORDER_COLOR);
			painter->drawRect(borderRect);
		}

		painter->restore();
	}
}

bool TextureListDelegate::helpEvent(QHelpEvent *event,
               QAbstractItemView *view,
               const QStyleOptionViewItem &option,
               const QModelIndex &index)
{
    if (nullptr == event || nullptr == view)
    {
        return false;
    }
    
    if (event->type() == QEvent::ToolTip)
    {
        auto tooltip = index.data( Qt::DisplayRole );
        if ( tooltip.canConvert<QString>() )
        {
            QString infoText = CreateInfoString(index);
            if(!infoText.isEmpty())
            {
                QToolTip::showText( event->globalPos(), infoText, view);
                return true;
            }
        }
    }
    
    return QAbstractItemDelegate::helpEvent( event, view, option, index );
}

QString TextureListDelegate::CreateInfoString(const QModelIndex & index) const
{
    if(!index.isValid())
    {
        return QString();
    }
    
    auto curModel = static_cast<const TextureListModel *>(index.model());
    auto curTextureDescriptor = curModel->getDescriptor(index);
    
    if(nullptr != curTextureDescriptor)
    {
        auto curTexture = curModel->getTexture(index);
        if(nullptr != curTexture)
        {
            QString infoText;
            char dimen[64];
            
            sprintf(dimen, "Size: %dx%d", curTexture->width, curTexture->height);
            infoText += dimen;
            infoText += "\nData size: ";
            infoText += QString::fromStdString(SizeInBytesToString(TextureCache::Instance()->getThumbnailSize(curTextureDescriptor)));
            
            auto dataSourcePath = ProjectManager::Instance()->CurProjectDataSourcePath();
            
            infoText += "\nPath: ";
            infoText += curTextureDescriptor->pathname.GetRelativePathname(dataSourcePath).c_str();
            
            return infoText;
        }
    }
 
    return QString();
}


void TextureListDelegate::drawPreviewSmall(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const TextureListModel *curModel = (TextureListModel *) index.model();
	DAVA::TextureDescriptor *curTextureDescriptor = curModel->getDescriptor(index);
	DAVA::Texture *curTexture = curModel->getTexture(index);

	if(nullptr != curTextureDescriptor)
	{
		painter->save();
		painter->setClipRect(option.rect);


		// draw border
		QRect borderRect = option.rect;
		borderRect.adjust(BORDER_MARGIN, BORDER_MARGIN, -BORDER_MARGIN, -BORDER_MARGIN);
		if(curModel->isHighlited(index))
		{
			// draw highlight
			painter->setBrush(QBrush(QColor(255, 255, 200)));
		}
		painter->setPen(BORDER_COLOR);
		painter->drawRect(borderRect);

		// draw preview
		QRect previewRect = borderRect;
		previewRect.adjust(BORDER_MARGIN, BORDER_MARGIN, -BORDER_MARGIN*2, -BORDER_MARGIN*2);
		previewRect.setWidth(previewRect.height());
		painter->setBrush(QBrush(QColor(100, 100, 100)));
		painter->drawRect(previewRect);

		// draw formats info
		int infoLen = drawFormatInfo(painter, borderRect, curTexture, curTextureDescriptor);

		// draw text
		QRectF textRect = option.rect;
		textRect.adjust(TEXTURE_PREVIEW_SIZE_SMALL, (option.rect.height() - option.fontMetrics.height())/2, -infoLen, 0);
		painter->setPen(option.palette.text().color());
		painter->drawText(textRect, curModel->data(index).toString());

		// draw selected item
		if(option.state & QStyle::State_Selected)
		{
			QBrush br = option.palette.highlight();
			QColor cl = br.color();
			cl.setAlpha(SELECTION_COLOR_ALPHA);
			br.setColor(cl);
			painter->setBrush(br);
			painter->setPen(SELECTION_BORDER_COLOR);
			painter->drawRect(borderRect);
		}

		painter->restore();
	}
}

int TextureListDelegate::drawFormatInfo(QPainter *painter, QRect rect, const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor) const
{
	static QIcon errorIcon = QIcon(":/QtIcons/error.png");

	int ret = 0;
	QRect r = rect;

	if(nullptr != descriptor && nullptr != texture)
	{
		r.adjust(FORMAT_INFO_SPACING, FORMAT_INFO_SPACING, -FORMAT_INFO_SPACING, -FORMAT_INFO_SPACING);
		r.setX(rect.x() + rect.width());
		r.setWidth(FORMAT_INFO_WIDTH);

		QColor gpuInfoColors[DAVA::GPU_DEVICE_COUNT];
		gpuInfoColors[DAVA::GPU_POWERVR_IOS] = TextureBrowser::gpuColor_PVR_ISO;
		gpuInfoColors[DAVA::GPU_POWERVR_ANDROID] = TextureBrowser::gpuColor_PVR_Android;
		gpuInfoColors[DAVA::GPU_TEGRA] = TextureBrowser::gpuColor_Tegra;
		gpuInfoColors[DAVA::GPU_MALI] = TextureBrowser::gpuColor_MALI;
		gpuInfoColors[DAVA::GPU_ADRENO] = TextureBrowser::gpuColor_Adreno;
        gpuInfoColors[DAVA::GPU_DX11] = TextureBrowser::gpuColor_DX11;

        // format lines
		for(int i = (DAVA::GPU_DEVICE_COUNT - 1); i >= 0; --i)
		{
			r.moveLeft(r.x() - FORMAT_INFO_WIDTH);

			if(descriptor->compression[i].format != DAVA::FORMAT_INVALID)
			{
				QColor c = gpuInfoColors[i];

				painter->setPen(Qt::NoPen);
				painter->setBrush(c);
				painter->drawRect(r);
			}

			r.moveLeft(r.x() - FORMAT_INFO_SPACING);
		}

		// error icon
		if(texture->width != texture->height)
		{
			r.moveLeft(r.x() - 16);
			errorIcon.paint(painter, r.x(), r.y(), 16, 16);
		}

		ret = rect.width() - (r.x() - rect.x());
	}

	return ret;
}

bool TextureListDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    switch ( event->type() )
    {
    case QEvent::MouseButtonRelease:
        {
            if ( !index.isValid() )
            {
                break;
            }

            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() != Qt::RightButton)
            {
                break;
            }

            const TextureListModel *curModel = qobject_cast<const TextureListModel *>(index.model());
            DVASSERT(curModel);
            DAVA::TextureDescriptor *curTextureDescriptor = curModel->getDescriptor(index);
            if (curTextureDescriptor == NULL)
            {
                break;
            }

            lastSelectedTextureFolder = curTextureDescriptor->pathname.GetAbsolutePathname().c_str();
            QMenu menu;
            QAction *act = menu.addAction("Open texture folder");
            connect(act, SIGNAL( triggered() ), SLOT( onOpenTexturePath() ));
            menu.exec( QCursor::pos() );
        }
        break;

    default:
        break;
    }

    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}

void TextureListDelegate::onOpenTexturePath()
{
    ShowFileInExplorer(lastSelectedTextureFolder);
    lastSelectedTextureFolder.clear();
}
