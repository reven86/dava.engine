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
#include "QtTools/WidgetHelpers/SharedIcon.h"

namespace MaterialItemLocal
{
const DAVA::int32 MAX_MATERIAL_HEIGHT = 30;
}

MaterialItem::MaterialItem(DAVA::NMaterial* _material, bool dragEnabled, bool dropEnabled)
    : QObject(nullptr)
    , QStandardItem()
    , material(_material)
{
    DVASSERT(material);

    setEditable(false);
    setData(QVariant::fromValue<DAVA::NMaterial*>(material));
    setDragEnabled(dragEnabled);
    setDropEnabled(dropEnabled);
    setSizeHint(QSize(MaterialItemLocal::MAX_MATERIAL_HEIGHT, MaterialItemLocal::MAX_MATERIAL_HEIGHT));

    setColumnCount(3);
}

MaterialItem::~MaterialItem()
{
}

QVariant MaterialItem::data(int role) const
{
    QVariant ret;

    switch (role)
    {
    case Qt::DisplayRole:
        ret = QString(material->GetMaterialName().c_str());
        break;
    case Qt::DecorationRole:
        const_cast<MaterialItem*>(this)->requestPreview();
        ret = QStandardItem::data(role);
        break;
    case Qt::BackgroundRole:
    {
        if (GetFlag(MaterialItem::IS_MARK_FOR_DELETE))
        {
            ret = QBrush(QColor(255, 0, 0, 20));
        }
        else if (GetMaterial()->GetConfigCount() > 1)
        {
            ret = QBrush(QColor(0, 0, 255, 40));
        }
    }
    break;
    case Qt::FontRole:
    {
        ret = QStandardItem::data(role);
        if (GetFlag(MaterialItem::IS_PART_OF_SELECTION))
        {
            QFont font = ret.value<QFont>();
            font.setBold(true);
            ret = font;
        }
    }
    break;
    default:
        ret = QStandardItem::data(role);
        break;
    }

    return ret;
}

DAVA::NMaterial* MaterialItem::GetMaterial() const
{
    return material;
}

void MaterialItem::SetFlag(MaterialFlag flag, bool enable)
{
    DVASSERT((flag == IS_PART_OF_SELECTION) || (flag == IS_MARK_FOR_DELETE));

    if (enable == GetFlag(flag))
        return;

    if (flag == IS_PART_OF_SELECTION)
    {
        QFont curFont = font();
        curFont.setBold(enable);
        setFont(curFont);
    }

    if (enable)
    {
        curFlag |= flag;
    }
    else
    {
        curFlag &= ~flag;
    }
    emitDataChanged();
}

bool MaterialItem::GetFlag(MaterialFlag flag) const
{
    return (curFlag & flag) == flag;
}

void MaterialItem::SetLodIndex(DAVA::int32 index)
{
    if (index != lodIndex)
    {
        lodIndex = index;
        emitDataChanged();
    }
}

int MaterialItem::GetLodIndex() const
{
    return lodIndex;
}

void MaterialItem::SetSwitchIndex(DAVA::int32 index)
{
    if (index != switchIndex)
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
    if (isPreviewRequested)
        return;

    isPreviewRequested = true;

    DAVA::Texture* albedoTexture = material->GetEffectiveTexture(DAVA::NMaterialTextureName::TEXTURE_ALBEDO);
    if (albedoTexture != nullptr)
    {
        DAVA::TextureDescriptor* descriptor = albedoTexture->GetDescriptor();
        QVariant itemRef = QString(descriptor->pathname.GetAbsolutePathname().c_str());
        TextureCache::Instance()->getThumbnail(descriptor, this, "onThumbnailReady", itemRef);
    }
}

void MaterialItem::onThumbnailReady(const QList<QImage>& images, QVariant userData)
{
    if (images.empty())
        return;

    QImage firstImage = images[0];
    QPainter painter(&firstImage);
    painter.setPen(QColor(0, 0, 0, 0x30));
    painter.drawRect(QRect(0, 0, firstImage.width() - 1, firstImage.height() - 1));
    setIcon(QIcon(QPixmap::fromImage(firstImage)));
}
