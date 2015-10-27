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


#include "TileTexturePreviewWidget.h"
#include "TextureBrowser/TextureConvertor.h"
#include "../../Main/QtUtils.h"
#include "StringConstants.h"
#include "ImageTools/ImageTools.h"

#include "TileTexturePreviewWidgetItemDelegate.h"
#include "Tools/ColorPicker/ColorPicker.h"

#include <QHeaderView>
#include <QLabel>
#include <QEvent>

TileTexturePreviewWidget::TileTexturePreviewWidget(QWidget* parent)
:	QTreeWidget(parent)
,	selectedTexture(0)
,	mode(MODES_COUNT)
,	validator(NULL)
{
	colors.reserve(DEF_TILE_TEXTURES_COUNT);
	images.reserve(DEF_TILE_TEXTURES_COUNT);
	labels.reserve(DEF_TILE_TEXTURES_COUNT);

	SetMode(MODE_WITH_COLORS);
	ConnectToSignals();

	validator = new QRegExpValidator();
	validator->setRegExp(QRegExp(TileTexturePreviewWidgetItemDelegate::TILE_COLOR_VALIDATE_REGEXP, Qt::CaseInsensitive));
}

TileTexturePreviewWidget::~TileTexturePreviewWidget()
{
	Clear();

	SafeDelete(validator);
}

void TileTexturePreviewWidget::Clear()
{
	clear();

	for (int32 i = 0; i < (int32)images.size(); ++i)
	{
		SafeRelease(images[i]);
	}

	colors.clear();
	images.clear();
	labels.clear();
}

void TileTexturePreviewWidget::AddTexture(Image* previewTexture, const Color& color /*  = Color::White */)
{
    DVASSERT(previewTexture->GetPixelFormat() == FORMAT_RGBA8888);

    bool blocked = signalsBlocked();
	blockSignals(true);

	images.push_back(SafeRetain(previewTexture));

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setCheckState(0, Qt::Unchecked);
	addTopLevelItem(item);

	if (mode == MODE_WITHOUT_COLORS)
	{
		item->setFlags((item->flags() | Qt::ItemIsUserCheckable) & ~(Qt::ItemIsSelectable | Qt::ItemIsEditable));
	}
	else
	{
		item->setFlags((item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable) & ~(Qt::ItemIsSelectable));

		QLabel* label = new QLabel();
		label->setMinimumWidth(26);
		label->setFrameShape(QFrame::Box);
		label->setAutoFillBackground(true);
		setItemWidget(item, COLOR_PREVIEW_COLUMN, label);
		label->setMinimumHeight(TEXTURE_PREVIEW_HEIGHT);
		labels.push_back(label);
		label->installEventFilter(this);
		label->setToolTip(ResourceEditor::TILE_TEXTURE_PREVIEW_CHANGE_COLOR_TOOLTIP.c_str());
		label->setCursor(Qt::PointingHandCursor);

		colors.push_back(color);

		UpdateColor(images.size() - 1);

		this->setItemDelegate(new TileTexturePreviewWidgetItemDelegate());
	}

	UpdateImage(images.size() - 1);
	UpdateSelection();

	blockSignals(blocked);
}

void TileTexturePreviewWidget::InitWithColors()
{
	bool blocked = signalsBlocked();
	blockSignals(true);

	Clear();

	setHeaderHidden(true);
	setColumnCount(2);
	setRootIsDecorated(false);
	header()->setStretchLastSection(false);
	header()->setSectionResizeMode(0, QHeaderView::Stretch);
	header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	setIconSize(QSize(TEXTURE_PREVIEW_WIDTH_SMALL, TEXTURE_PREVIEW_HEIGHT));

	blockSignals(blocked);
}

void TileTexturePreviewWidget::InitWithoutColors()
{
	bool blocked = signalsBlocked();
	blockSignals(true);

	Clear();

	setHeaderHidden(true);
	setColumnCount(1);
	setRootIsDecorated(false);
	setIconSize(QSize(TEXTURE_PREVIEW_WIDTH, TEXTURE_PREVIEW_HEIGHT));

	blockSignals(blocked);
}

void TileTexturePreviewWidget::ConnectToSignals()
{
	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
			this, SLOT(OnCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnItemChanged(QTreeWidgetItem*, int)));
}

int32 TileTexturePreviewWidget::GetSelectedTexture()
{
	return selectedTexture;
}

void TileTexturePreviewWidget::SetSelectedTexture(int32 number)
{
	if (number < 0 || number >= (int32)images.size())
	{
		return;
	}

	selectedTexture = number;
	UpdateSelection();

	emit SelectionChanged(selectedTexture);
}

void TileTexturePreviewWidget::UpdateImage(int32 number)
{
	DVASSERT(number >= 0 && number < (int32)images.size());

	QTreeWidgetItem* item = topLevelItem(number);

	Image* image;
	if (mode == MODE_WITH_COLORS)
	{
		image = MultiplyImageWithColor(images[number], colors[number]);
	}
	else
	{
		image = SafeRetain(images[number]);
	}

	QImage qimg = ImageTools::FromDavaImage(image);
	SafeRelease(image);

	QSize size = QSize(TEXTURE_PREVIEW_WIDTH, TEXTURE_PREVIEW_HEIGHT);
	if (mode == MODE_WITH_COLORS)
	{
		size.setWidth(TEXTURE_PREVIEW_WIDTH_SMALL);
	}

	QImage previewImage = qimg.copy(0, 0, size.width(), size.height());
	QIcon icon = QIcon(QPixmap::fromImage(previewImage));

	item->setIcon(0, icon);
}

void TileTexturePreviewWidget::UpdateColor(int32 number)
{
	DVASSERT(number >= 0 && number < (int32)images.size());
    
    bool blocked = blockSignals(true);

    
	QTreeWidgetItem* item = topLevelItem(number);
	QColor color = ColorToQColor(colors[number]);
    color.setAlpha(255);

    QPalette palette = labels[number]->palette();
	palette.setColor(labels[number]->backgroundRole(), color);
	labels[number]->setPalette(palette);

	QString str;
	str.sprintf("#%02x%02x%02x", color.red(), color.green(), color.blue());
	item->setText(0, str);

	UpdateImage(number);
    
    blockSignals(blocked);
}

void TileTexturePreviewWidget::UpdateSelection()
{
	for (int32 i = 0; i < (int32)images.size(); ++i)
	{
		QTreeWidgetItem* item = topLevelItem(i);
		item->setCheckState(0, (i == selectedTexture ? Qt::Checked : Qt::Unchecked));
	}
}

void TileTexturePreviewWidget::OnCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	QModelIndex index = currentIndex();
	selectedTexture = index.row();
	UpdateSelection();

	emit SelectionChanged(selectedTexture);
}

void TileTexturePreviewWidget::OnItemChanged(QTreeWidgetItem* item, int column)
{
	int32 index = indexOfTopLevelItem(item);

	if (mode == MODE_WITH_COLORS)
	{
		int32 len = 0;
		QString str = item->text(0);
		QValidator::State state = validator->validate(str, len);

		if (state == QValidator::Acceptable)
		{
			QString colorString = item->text(0);
			if (!colorString.startsWith("#"))
			{
				colorString = "#" + colorString;
			}

			QColor color = QColor(colorString);
			if (color.isValid())
			{
				Color c = QColorToColor(color);
				if (c != colors[index])
				{
					SetColor(index, c);
				}
			}
		}
	}

	if (item->checkState(0) == Qt::Checked)
	{
		SetSelectedTexture(index);
	}
	else
	{
		UpdateSelection();
	}
}

bool TileTexturePreviewWidget::eventFilter(QObject *obj, QEvent *ev)
{
	for (int32 i = 0; i < (int32)labels.size(); ++i)
	{
		if (obj == labels[i])
		{
			if (ev->type() == QEvent::MouseButtonRelease)
			{
                const Color oldColor = colors[i];
                ColorPicker cp(this);
                cp.setWindowTitle("Tile color");
                cp.SetDavaColor(oldColor);
                const bool result = cp.Exec();
                const Color newColor = cp.GetDavaColor();

				if (result && newColor != oldColor)
				{
					SetColor(i, newColor);
				}

				return true;
			}
			else if (ev->type() == QEvent::MouseButtonPress)
			{
				return true;
			}
		}
	}

	return QObject::eventFilter(obj, ev);
}

void TileTexturePreviewWidget::SetColor(int32 number, const Color& color)
{
	colors[number] = color;
	emit TileColorChanged(number, colors[number]);
	UpdateColor(number);
}

void TileTexturePreviewWidget::SetMode(TileTexturePreviewWidget::eWidgetModes mode)
{
	if (mode == this->mode)
	{
		return;
	}

	if (mode == MODE_WITH_COLORS)
	{
		InitWithColors();
	}
	else if (mode == MODE_WITHOUT_COLORS)
	{
		InitWithoutColors();
	}
	this->mode = mode;
}

Image* TileTexturePreviewWidget::MultiplyImageWithColor(DAVA::Image *image, const DAVA::Color &color)
{
    DVASSERT(image->GetPixelFormat() == FORMAT_RGBA8888);

    Image* newImage = Image::Create(image->GetWidth(), image->GetHeight(), image->GetPixelFormat());

    uint32* imageData = (uint32*)image->GetData();
    uint32* newImageData = (uint32*)newImage->GetData();

    int32 pixelsCount = image->dataSize / sizeof(uint32);

    for (int32 i = 0; i < pixelsCount; ++i)
    {
        uint8* pixelData = (uint8*)imageData;
        uint8* newPixelData = (uint8*)newImageData;

        newPixelData[0] = (uint8)floorf(pixelData[0] * color.r);
        newPixelData[1] = (uint8)floorf(pixelData[1] * color.g);
        newPixelData[2] = (uint8)floorf(pixelData[2] * color.b);
        newPixelData[3] = 255;

        ++imageData;
        ++newImageData;
    }

    return newImage;
}

void TileTexturePreviewWidget::UpdateColor(int32 index, const Color& color)
{
	if (index < 0 || index >= (int32)colors.size())
	{
		return;
	}

	if (colors[index] != color)
	{
		SetColor(index, color);
	}
}
