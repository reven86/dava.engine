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


#ifndef __TEXTURE_LIST_DELEGATE_H__
#define __TEXTURE_LIST_DELEGATE_H__

#include <QAbstractItemDelegate>
#include <QMap>
#include <QFont>
#include <QFontMetrics>

#include "TextureInfo.h"
#include "DAVAEngine.h"

class QPainter;

class TextureListDelegate : public QAbstractItemDelegate
{
	Q_OBJECT

public:
	enum DrawRule
	{
		DRAW_PREVIEW_BIG,
		DRAW_PREVIEW_SMALL
	};

	TextureListDelegate(QObject *parent = 0);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void setDrawRule(DrawRule rule);
    
    bool helpEvent(QHelpEvent *event,
                           QAbstractItemView *view,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) override;

signals:
    void textureDescriptorChanged(DAVA::TextureDescriptor* descriptor);

protected:
    bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index);

private slots:
	void textureReadyThumbnail(const DAVA::TextureDescriptor *descriptor,  const TextureInfo & images);
    void onOpenTexturePath();

    void onLoadPreset();
    void onSavePreset();

private:
	QFont nameFont;
	QFontMetrics nameFontMetrics;
	mutable QMap<DAVA::FilePath, QModelIndex> descriptorIndexes;
    DAVA::TextureDescriptor* lastSelectedTextureDescriptor = nullptr;

    DrawRule drawRule;

    void drawPreviewBig(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void drawPreviewSmall(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	int drawFormatInfo(QPainter *painter, QRect rect, const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor) const;
    
    QString CreateInfoString(const QModelIndex & index) const;
};

#endif // __TEXTURE_LIST_DELEGATE_H__
