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
#include <QPainter>
#include <QImage>
#include <QDebug>
#include <QApplication>

#include "MaterialItem.h"
#include "MaterialModel.h"
#include "Main/QtUtils.h"
#include "TextureBrowser/TextureCache.h"

namespace
{
    const int MAX_MATERIAL_HEIGHT = 30;
}

MaterialItem::MaterialItem(DAVA::NMaterial* _material, bool dragEnabled, bool dropEnabled)
    : QObject(NULL)
    , QStandardItem()
    , material(_material)
    , curFlag(0)
    , isPreviewRequested(false)
    , lodIndex(-1)
    , switchIndex(-1)
{
	DVASSERT(material);

	static QIcon materialIcon(QString::fromUtf8(":/QtIcons/sphere.png"));
	static QIcon instanceIcon(QString::fromUtf8(":/QtIcons/3d.png"));
    static QIcon globalIcon(QString::fromUtf8(":/QtIcons/global.png"));
	
	setEditable(false);
    setData(QVariant::fromValue<DAVA::NMaterial *>(material));
    setDragEnabled(dragEnabled);
    setDropEnabled(dropEnabled);
    setSizeHint(QSize(MAX_MATERIAL_HEIGHT, MAX_MATERIAL_HEIGHT));

    setColumnCount(3);
}

MaterialItem::~MaterialItem()
{ }

QVariant MaterialItem::data(int role) const
{
	QVariant ret;

	switch(role)
	{
		case Qt::DisplayRole:
			ret = QString(material->GetMaterialName().c_str());
			break;
        case Qt::DecorationRole:
            const_cast< MaterialItem * >( this )->requestPreview();
            ret = QStandardItem::data(role);
            break;
		default:
			ret = QStandardItem::data(role);
			break;
	}

    return ret;
}

DAVA::NMaterial * MaterialItem::GetMaterial() const
{
	return material;
}

void MaterialItem::SetFlag(MaterialFlag flag, bool set)
{
    if ((set && !(curFlag & flag)) || (!set && (curFlag & flag)))
    {
        bool ok = true;
        switch (flag)
        {
        case IS_MARK_FOR_DELETE:
            break;

            case IS_PART_OF_SELECTION:
                if (material->GetChildren().size() > 0)
                {
                    QFont curFont = font();
                    curFont.setBold(set);
                    setFont(curFont);
                }
                break;

            default:
                ok = false;
                break;
            }

        if(ok)
        {
            if(set)
            {
                curFlag |= (int) flag;
            }
            else
            {
                curFlag &= ~(int) flag;
            }
            emitDataChanged();
        }
    }
}

bool MaterialItem::GetFlag(MaterialFlag flag) const
{
	return (bool) (curFlag & flag);
}

void MaterialItem::SetLodIndex(int index)
{
    if(index != lodIndex)
    {
        lodIndex = index;
        emitDataChanged();
    }
}

int MaterialItem::GetLodIndex() const
{
    return lodIndex;
}

void MaterialItem::SetSwitchIndex(int index)
{
    if(index != switchIndex)
    {
        switchIndex = index;
        emitDataChanged();
    }
}

int MaterialItem::GetSwitchIndex() const
{
    return switchIndex;
}

void MaterialItem::requestPreview()
{
    if(!isPreviewRequested)
    {
        isPreviewRequested = true;

        DAVA::Texture* t = material->GetEffectiveTexture(DAVA::NMaterialTextureName::TEXTURE_ALBEDO);
        if(t)
        {
            DAVA::TextureDescriptor *descriptor = t->GetDescriptor();
            QVariant itemRef = QString(descriptor->pathname.GetAbsolutePathname().c_str());
            TextureCache::Instance()->getThumbnail(descriptor, this, "onThumbnailReady", itemRef);
        }
    }
}

void MaterialItem::onThumbnailReady( QList<QImage> images, QVariant userData )
{
    if(images.size() > 0)
    {
        QImage img0 = images[0];
        QPainter p(&img0);
        QRect rc(0, 0, img0.width() - 1, img0.height() - 1);
        p.setPen(QColor(0, 0, 0, 0x30));
        p.drawRect(rc);

        setIcon(QIcon(QPixmap::fromImage(img0)));
    }
}
