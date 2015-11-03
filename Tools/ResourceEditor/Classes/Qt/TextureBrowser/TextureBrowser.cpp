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


#include "TextureBrowser/TextureBrowser.h"
#include "TextureBrowser/TextureListModel.h"
#include "TextureBrowser/TextureListDelegate.h"
#include "TextureBrowser/TextureConvertor.h"
#include "TextureBrowser/TextureCache.h"
#include "Main/QtUtils.h"
#include "Main/mainwindow.h"
#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/LibDdsHelper.h"
#include "Qt/Settings/SettingsManager.h"
#include "Scene/SceneHelper.h"
#include "CubemapEditor/CubemapUtils.h"

#include "Classes/Constants.h"

#include "ui_texturebrowser.h"

#include <QComboBox>
#include <QAbstractItemModel>
#include <QStatusBar>
#include <QToolButton>
#include <QFileInfo>
#include <QList>
#include <QMessageBox>
#include <QProgressBar>


QColor TextureBrowser::gpuColor_PVR_ISO = QColor(0, 200, 0, 255);
QColor TextureBrowser::gpuColor_PVR_Android = QColor(0, 0, 200, 255);
QColor TextureBrowser::gpuColor_Tegra = QColor(0, 200, 200, 255);
QColor TextureBrowser::gpuColor_MALI = QColor(200, 200, 0, 255);
QColor TextureBrowser::gpuColor_Adreno = QColor(200, 0, 200, 255);
QColor TextureBrowser::gpuColor_DX11 = QColor(200, 200, 200, 255);
QColor TextureBrowser::errorColor = QColor(255, 0, 0, 255);

TextureBrowser::TextureBrowser(QWidget *parent)
    : QDialog(parent)
	, ui(new Ui::TextureBrowser)
	, curScene(NULL)
	, curTextureView(DAVA::GPU_POWERVR_IOS)
	, curTexture(NULL)
	, curDescriptor(NULL)
{
	ui->setupUi(this);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);
	
	textureListModel = new TextureListModel();
	textureListImagesDelegate = new TextureListDelegate();

	textureListSortModes["File size"] = TextureListModel::SortByFileSize;
	textureListSortModes["Data size"] = TextureListModel::SortByDataSize;
	textureListSortModes["Image size"] = TextureListModel::SortByImageSize;
	textureListSortModes["Name"] = TextureListModel::SortByName;

	// global scene manager signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(sceneDeactivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT(sceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));

	// convertor signals
	QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyOriginal(const DAVA::TextureDescriptor *, const TextureInfo &)), this, SLOT(textureReadyOriginal(const DAVA::TextureDescriptor *, const TextureInfo &)));
	QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyConverted(const DAVA::TextureDescriptor *, const DAVA::eGPUFamily, const TextureInfo &)), this, SLOT(textureReadyConverted(const DAVA::TextureDescriptor *, const DAVA::eGPUFamily, const TextureInfo &)));

	setupStatusBar();
	setupTexturesList();
	setupImagesScrollAreas();
	setupTextureListToolbar();
	setupTextureToolbar();
	setupTextureListFilter();
	setupTextureProperties();
	setupTextureViewTabBar();

	resetTextureInfo();

	// let textures list show images-view by default
	ui->actionViewImagesList->trigger();

	// let textures view show border by default
	// ui->actionShowBorder->trigger();

	// set initial empty texture
	setTexture(curTexture, curDescriptor);
	setTextureView(curTextureView);

	// ui->splitter->setSizes(QList<int>() << 60 << 0 << 40);

	posSaver.Attach(this);
    new QtPosSaver( ui->splitterMain );
}

TextureBrowser::~TextureBrowser()
{
	Close();

	delete textureListImagesDelegate;
	delete textureListModel;
    delete ui;
}

void TextureBrowser::Close()
{
    hide();

    TextureConvertor::Instance()->CancelConvert();
	TextureConvertor::Instance()->WaitConvertedAll();

    setScene(nullptr);

    // clear cache
    TextureCache::Instance()->clearInsteadThumbnails();

    ui->textureAreaConverted->warningShow(false);
}

void TextureBrowser::Update()
{
	setScene(curScene);
}

void TextureBrowser::closeEvent(QCloseEvent * e)
{
	QDialog::closeEvent(e);
	Close();
}

void TextureBrowser::setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor)
{
	curTexture = texture;
	curDescriptor = descriptor;

	// color channels to default value
	ui->actionColorR->setChecked(true);
	ui->actionColorG->setChecked(true);
	ui->actionColorB->setChecked(true);
	ui->actionColorA->setChecked(true);
	ui->textureAreaOriginal->setColorChannel(TextureScrollArea::ChannelAll);
	ui->textureAreaConverted->setColorChannel(TextureScrollArea::ChannelAll);

	ui->textureAreaOriginal->resetTexturePosZoom();
	ui->textureAreaConverted->resetTexturePosZoom();
	toolbarZoomSlider->setValue(0);

	// disable texture views by default
	ui->textureAreaOriginal->setEnabled(false);
	ui->textureAreaConverted->setEnabled(false);
	ui->convertToolButton->setEnabled(false);

	// set texture to properties control.
	// this should be done as a first step
	ui->textureProperties->setTextureDescriptor(curDescriptor);
	ui->textureProperties->setTextureGPU(curTextureView);

	updatePropertiesWarning();

	// if texture is ok - set it and enable texture views
	if(NULL != curTexture)
	{
		// enable convert button
		ui->convertToolButton->setEnabled(true);

		// load original image
		// check if image is in cache
		TextureInfo info;
		info.images = TextureCache::Instance()->getOriginal(curDescriptor);
		if(info.images.size() > 0 &&
		   !info.images[0].isNull())
		{
			// image is in cache, so set it immediately (by calling image-ready slot)
			textureReadyOriginal(curDescriptor, info);
		}
		else
		{
			// set empty info
			QList<QImage> emptyImages;
			emptyImages.push_back(QImage());
			updateInfoOriginal(emptyImages);

			// there is no image in cache - start loading it in different thread. image-ready slot will be called 
			ui->textureAreaOriginal->setImage(QImage());
			TextureConvertor::Instance()->GetOriginal(curDescriptor);

			// show loading bar
			ui->textureAreaOriginal->waitbarShow(true);
		}
	}
	else
	{
		// no texture - set empty images to original/PVR views
		ui->textureAreaOriginal->setImage(QImage());
		ui->textureAreaConverted->setImage(QImage());
	}
}

void TextureBrowser::setTextureView(DAVA::eGPUFamily view, eTextureConvertMode convertMode /* = CONVERT_NOT_EXISTENT */)
{
	bool infoConvertedIsUpToDate = false;
    bool cacheCleared = false;

	// if force convert or modified - clear cached images
	if(convertMode == CONVERT_FORCE ||
       (convertMode == CONVERT_MODIFIED && curDescriptor && !curDescriptor->IsCompressedTextureActual(view)))
	{
		TextureCache::Instance()->clearConverted(curDescriptor, view);
        cacheCleared = true;
	}

	curTextureView = view;

	// second set texture view to appropriate state
	if(NULL != curTexture && NULL != curDescriptor)
	{
		bool needConvert = true;

		// set empty image to converted image view. it will be visible until
		// conversion done (signal by textureConvertor).
		ui->textureAreaConverted->setImage(QImage());
		ui->textureAreaConverted->waitbarShow(true);

		// set current tab
		ui->viewTabBar->setCurrentIndex(curTextureView);
		ui->textureProperties->setTextureGPU(curTextureView);

		if(!cacheCleared)
		{
			// try to find image in cache
			const QList<QImage>& images = TextureCache::Instance()->getConverted(curDescriptor, view);

			if(images.size() > 0 &&
			   !images[0].isNull())
			{
				// image already in cache, just draw it
				updateConvertedImageAndInfo(images, *curDescriptor);

                needConvert = false;
				infoConvertedIsUpToDate = true;
			}
		}

		if(needConvert)
		{
			// Start convert. Signal will be emited when conversion done
			TextureConvertor::Instance()->GetConverted(curDescriptor, view, convertMode);
		}
	}

	if(infoConvertedIsUpToDate)
	{
		updateInfoConverted();
	}
}

void TextureBrowser::updatePropertiesWarning()
{
	if(NULL != curDescriptor && NULL != curTexture)
	{
		QString warningText = "";

		if(curTexture->width != curTexture->height)
		{
			warningText += "WARNING: Not square texture.\n";
		}

		ui->warningLabel->setText(warningText);
	}
}

void TextureBrowser::updateConvertedImageAndInfo(const QList<QImage> &images, DAVA::TextureDescriptor& descriptor)
{
	if(!descriptor.IsCubeMap())
	{
		ui->textureAreaConverted->setImage(images[0]);
	}
	else
	{
		ui->textureAreaConverted->setImage(images, descriptor.dataSettings.cubefaceFlags);
	}
	
	ui->textureAreaConverted->setEnabled(true);
	ui->textureAreaConverted->waitbarShow(false);

	// set info about converted image
	updateInfoConverted();
}

void TextureBrowser::updateInfoColor(QLabel *label, const QColor &color /* = QColor() */)
{
	if(NULL != label)
	{
		char tmp[1024];

		sprintf(tmp, "R: %02X\nG: %02X\nB: %02X\nA: %02X",
			color.red(), color.green(), color.blue(), color.alpha());

		label->setText(tmp);
	}
}

void TextureBrowser::updateInfoPos(QLabel *label, const QPoint &pos /* = QPoint() */)
{
	if(NULL != label)
	{
		char tmp[1024];

		sprintf(tmp, "X : %d\nY : %d",
			pos.x(), pos.y());

		label->setText(tmp);
	}
}

void TextureBrowser::updateInfoOriginal(const QList<QImage> &images)
{
	if(NULL != curTexture && NULL != curDescriptor)
	{
		char tmp[1024];

		const char *formatStr = DAVA::PixelFormatDescriptor::GetPixelFormatString(DAVA::FORMAT_RGBA8888);

		int datasize = TextureCache::Instance()->getOriginalSize(curDescriptor);
		int filesize = TextureCache::Instance()->getOriginalFileSize(curDescriptor);

		sprintf(tmp, "Format: %s\nSize: %dx%d\nData size: %s\nFile size: %s", formatStr, images[0].width(), images[0].height(),
			 SizeInBytesToString(datasize).c_str(),
			 SizeInBytesToString(filesize).c_str());

		ui->labelOriginalFormat->setText(tmp);
	}
	else
	{
		ui->labelOriginalFormat->setText("");
	}
}

void TextureBrowser::updateInfoConverted()
{
	if(curTexture != nullptr && 
       curDescriptor != nullptr && 
       curDescriptor->HasCompressionFor(curTextureView))
	{
		char tmp[1024];
		const char *formatStr = "Unknown";

		int datasize = TextureCache::Instance()->getConvertedSize(curDescriptor, curTextureView);
		int filesize = TextureCache::Instance()->getConvertedFileSize(curDescriptor, curTextureView);
		QSize imgSize = TextureCache::Instance()->getConvertedImageSize(curDescriptor, curTextureView);

        bool isUpToDate = curDescriptor->IsCompressedTextureActual(curTextureView);
        
        bool isFormatValid = curDescriptor->compression[curTextureView].format != DAVA::FORMAT_INVALID;
		if(isFormatValid)
		{
			formatStr = GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(curDescriptor->compression[curTextureView].format);
		}
        ui->convertToolButton->setEnabled(isFormatValid);
        
		sprintf(tmp, "Format: %s\nSize: %dx%d\nData size: %s\nFile size: %s", formatStr, imgSize.width(), imgSize.height(),
			SizeInBytesToString(datasize).c_str(),
			SizeInBytesToString(filesize).c_str());

		ui->labelConvertedFormat->setText(tmp);
        ui->textureAreaConverted->warningShow(!isUpToDate);
	}
	else
	{
		ui->labelConvertedFormat->setText("");
        ui->textureAreaConverted->warningShow(false);
	}
}

void TextureBrowser::setupStatusBar()
{
	statusBarProgress = new QProgressBar();
	statusBarProgress->setMaximumSize(150, 14);
	statusBarProgress->setTextVisible(false);
	statusBarProgress->setVisible(false);

	statusQueueLabel = new QLabel();

	statusBar = new QStatusBar(this);
	statusBar->setContentsMargins(0, 0, 0, 0);
	statusBar->addPermanentWidget(statusQueueLabel);
	statusBar->addPermanentWidget(statusBarProgress);
	statusBar->setStyleSheet("QStatusBar::item {border: none;}");
	statusBar->layout()->setMargin(0);
	statusBar->layout()->setSpacing(1);
	statusBar->layout()->setContentsMargins(0, 0, 0, 0);
	statusBar->setMaximumHeight(20);

	ui->mainLayout->addWidget(statusBar);

	QObject::connect(TextureConvertor::Instance(), SIGNAL(ConvertStatusImg(const QString&, int)), this, SLOT(convertStatusImg(const QString&, int)));
	QObject::connect(TextureConvertor::Instance(), SIGNAL(ConvertStatusQueue(int, int)), this, SLOT(convertStatusQueue(int, int)));
}

void TextureBrowser::setupTexturesList()
{
	QObject::connect(ui->listViewTextures, SIGNAL(selected(const QModelIndex &)), this, SLOT(texturePressed(const QModelIndex &)));
	QObject::connect(ui->clearFilterButton, SIGNAL(released()), this, SLOT(clearFilter()));

	ui->listViewTextures->setItemDelegate(textureListImagesDelegate);
	ui->listViewTextures->setModel(textureListModel);
}

void TextureBrowser::setupImagesScrollAreas()
{
	// pos change
	QObject::connect(ui->textureAreaOriginal, SIGNAL(texturePosChanged(const QPoint &)), ui->textureAreaConverted, SLOT(setTexturePos(const QPoint &)));
	QObject::connect(ui->textureAreaConverted, SIGNAL(texturePosChanged(const QPoint &)), ui->textureAreaOriginal, SLOT(setTexturePos(const QPoint &)));

	// mouse over pixel
	QObject::connect(ui->textureAreaOriginal, SIGNAL(mouseOverPixel(const QPoint &)), this, SLOT(texturePixelOver(const QPoint &)));
	QObject::connect(ui->textureAreaConverted, SIGNAL(mouseOverPixel(const QPoint &)), this, SLOT(texturePixelOver(const QPoint &)));

	// mouse wheel
	QObject::connect(ui->textureAreaOriginal, SIGNAL(mouseWheel(int)), this, SLOT(textureAreaWheel(int)));
	QObject::connect(ui->textureAreaConverted, SIGNAL(mouseWheel(int)), this, SLOT(textureAreaWheel(int)));

    ui->textureAreaConverted->warningSetText("Not relevant");
}

void TextureBrowser::setupTextureToolbar()
{
	QLabel *textureZoomLabel = new QLabel("Zoom: ");

	toolbarZoomSlider = new QSlider();
	toolbarZoomSliderValue = new QLabel();
	toolbarZoomSlider->setOrientation(Qt::Horizontal);
	toolbarZoomSlider->setMaximumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_TEXT);
	toolbarZoomSlider->setTracking(true);
	toolbarZoomSlider->setRange(-90, 90);
	toolbarZoomSlider->setTickPosition(QSlider::TicksBelow);
	toolbarZoomSlider->setTickInterval(15);
	toolbarZoomSlider->setSingleStep(5);
	textureZoomSlide(0);

	ui->textureToolbar->addWidget(textureZoomLabel);
	ui->textureToolbar->addWidget(toolbarZoomSlider);
	ui->textureToolbar->addWidget(toolbarZoomSliderValue);

	QObject::connect(ui->actionColorA, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorB, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorG, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorR, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));

	QObject::connect(ui->actionShowBorder, SIGNAL(triggered(bool)), this, SLOT(textureBorderPressed(bool)));
	QObject::connect(ui->actionShowBgMask, SIGNAL(triggered(bool)), this, SLOT(textureBgMaskPressed(bool)));

	QObject::connect(ui->actionZoom100, SIGNAL(triggered(bool)), this, SLOT(textureZoom100(bool)));
	QObject::connect(ui->actionZoomFit, SIGNAL(triggered(bool)), this, SLOT(textureZoomFit(bool)));

	QObject::connect(toolbarZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(textureZoomSlide(int)));

	QObject::connect(ui->actionConvert, SIGNAL(triggered(bool)), this, SLOT(textureConver(bool)));
	QObject::connect(ui->actionConvertAll, SIGNAL(triggered(bool)), this, SLOT(textureConverAll(bool)));
    QObject::connect(ui->actionConvertModified, SIGNAL(triggered(bool)), this, SLOT(ConvertModifiedTextures(bool)));
}

void TextureBrowser::setupTextureListToolbar()
{
	QComboBox *texturesSortCombo = new QComboBox();
	QLabel *texturesSortComboLabel = new QLabel();

	QMapIterator<QString, int> i(textureListSortModes);
	while(i.hasNext())
	{
		i.next();
		texturesSortCombo->addItem(i.key());
	}

	texturesSortComboLabel->setText("Sort by: ");

	// insert blank widget to align convert actions right
	QWidget *spacerWidget = new QWidget(this);
	spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	spacerWidget->setVisible(true);

	ui->textureListToolbar->insertWidget(ui->actionConvertAll, spacerWidget);
	ui->textureListToolbar->insertSeparator(ui->actionConvertAll);

	// TODO: mainwindow
	//QtMainWindow::Instance()->ShowActionWithText(ui->textureListToolbar, ui->actionConvertAll, true);

	ui->textureListSortToolbar->addWidget(texturesSortComboLabel);
	ui->textureListSortToolbar->addWidget(texturesSortCombo);

	QObject::connect(texturesSortCombo, SIGNAL(activated(const QString &)), this, SLOT(textureListSortChanged(const QString &)));
	QObject::connect(ui->actionViewTextList, SIGNAL(triggered(bool)), this, SLOT(textureListViewText(bool)));
	QObject::connect(ui->actionViewImagesList, SIGNAL(triggered(bool)), this, SLOT(textureListViewImages(bool)));
	QObject::connect(ui->actionFilterSelectedNode, SIGNAL(triggered(bool)), this, SLOT(textureListFilterSelectedNodeChanged(bool)));

    // set sorting type appropriate to current value in combobox
    textureListSortChanged(texturesSortCombo->itemText(texturesSortCombo->currentIndex()));
}

void TextureBrowser::setupTextureListFilter()
{
	QObject::connect(ui->textureFilterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textureListFilterChanged(const QString&)));
}

void TextureBrowser::setupTextureProperties()
{
	QObject::connect(ui->textureProperties, SIGNAL(PropertyChanged(int)), this, SLOT(texturePropertyChanged(int)));

	ui->convertToolButton->setDefaultAction(ui->actionConvert);

	QPalette palette = ui->warningLabel->palette();
	palette.setColor(ui->warningLabel->foregroundRole(), Qt::red);
	ui->warningLabel->setPalette(palette);
}

void TextureBrowser::setupTextureViewTabBar()
{
	ui->viewTabBar->setTabsClosable(false);
	ui->viewTabBar->setMovable(false);
	ui->viewTabBar->setUsesScrollButtons(false);
	ui->viewTabBar->setExpanding(false);

	QPixmap pix(16,16);
	QPainter p(&pix);
	int tabIndex;
	
	p.setBrush(QBrush(gpuColor_PVR_ISO));
	p.setPen(QColor(64, 64, 64, 255));
	p.drawRect(QRect(0,0,15,15));
	
	tabIndex = ui->viewTabBar->addTab("PVR iOS");
	ui->viewTabBar->setTabData(tabIndex, GPU_POWERVR_IOS);
	ui->viewTabBar->setTabIcon(tabIndex, QIcon(pix));

	p.setBrush(QBrush(gpuColor_PVR_Android));
	p.drawRect(QRect(0,0,15,15));

	tabIndex = ui->viewTabBar->addTab("PVR Android");
	ui->viewTabBar->setTabData(tabIndex, GPU_POWERVR_ANDROID);
	ui->viewTabBar->setTabIcon(tabIndex, QIcon(pix));

	p.setBrush(QBrush(gpuColor_Tegra));
	p.drawRect(QRect(0,0,15,15));

	tabIndex = ui->viewTabBar->addTab("Tegra");
	ui->viewTabBar->setTabData(tabIndex, GPU_TEGRA);
	ui->viewTabBar->setTabIcon(tabIndex, QIcon(pix));

	p.setBrush(QBrush(gpuColor_MALI));
	p.drawRect(QRect(0,0,15,15));

	tabIndex = ui->viewTabBar->addTab("MALI");
	ui->viewTabBar->setTabData(tabIndex, GPU_MALI);
	ui->viewTabBar->setTabIcon(tabIndex, QIcon(pix));

	p.setBrush(QBrush(gpuColor_Adreno));
	p.drawRect(QRect(0,0,15,15));

	tabIndex = ui->viewTabBar->addTab("Adreno");
	ui->viewTabBar->setTabData(tabIndex, GPU_ADRENO);
	ui->viewTabBar->setTabIcon(tabIndex, QIcon(pix));

    p.setBrush(QBrush(gpuColor_DX11));
    p.drawRect(QRect(0, 0, 15, 15));
    tabIndex = ui->viewTabBar->addTab("DX11");
    ui->viewTabBar->setTabData(tabIndex, GPU_DX11);
    ui->viewTabBar->setTabIcon(tabIndex, QIcon(pix));

    QObject::connect(ui->viewTabBar, SIGNAL(currentChanged(int)), this, SLOT(textureViewChanged(int)));
}

void TextureBrowser::resetTextureInfo()
{
	updateInfoColor(ui->labelOriginalRGBA);
	updateInfoPos(ui->labelOriginalXY);

	updateInfoColor(ui->labelConvertedRGBA);
	updateInfoPos(ui->labelConvertedXY);

	QList<QImage> emptyImages;
	emptyImages.push_back(QImage());
	updateInfoOriginal(emptyImages);
	updateInfoConverted();
}

void TextureBrowser::reloadTextureToScene(DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu)
{
	if(NULL != descriptor && NULL != texture)
	{
		DAVA::eGPUFamily curEditorImageGPUForTextures = QtMainWindow::Instance()->GetGPUFormat();

		// reload only when editor view format is the same as given texture format
		// or if given texture format if not a file (will happened if some common texture params changed - mipmap/filtering etc.)
		if(!GPUFamilyDescriptor::IsGPUForDevice(gpu) || gpu == curEditorImageGPUForTextures)
		{
			texture->ReloadAs(curEditorImageGPUForTextures);
		}
	}
}

void TextureBrowser::texturePressed(const QModelIndex & index)
{
	setTexture(textureListModel->getTexture(index), textureListModel->getDescriptor(index));
	setTextureView(curTextureView);
}

void TextureBrowser::textureListViewText(bool checked)
{
	ui->actionViewImagesList->setChecked(false);
	ui->actionViewTextList->setChecked(true);

	textureListImagesDelegate->setDrawRule(TextureListDelegate::DRAW_PREVIEW_SMALL);
	ui->listViewTextures->reset();
}

void TextureBrowser::textureListViewImages(bool checked)
{
	ui->actionViewTextList->setChecked(false);
	ui->actionViewImagesList->setChecked(true);

	textureListImagesDelegate->setDrawRule(TextureListDelegate::DRAW_PREVIEW_BIG);
	ui->listViewTextures->reset();
}

void TextureBrowser::textureListFilterChanged(const QString &text)
{
	textureListModel->setFilter(text);
}

void TextureBrowser::textureListFilterSelectedNodeChanged(bool checked)
{
	textureListModel->setFilterBySelectedNode(checked);
}

void TextureBrowser::textureListSortChanged(const QString &text)
{
	textureListModel->setSortMode((TextureListModel::TextureListSortMode) textureListSortModes[text]);
}

void TextureBrowser::textureColorChannelPressed(bool checked)
{
	int channelsMask = 0;

	if(ui->actionColorA->isChecked()) channelsMask |= TextureScrollArea::ChannelA;
	if(ui->actionColorB->isChecked()) channelsMask |= TextureScrollArea::ChannelB;
	if(ui->actionColorG->isChecked()) channelsMask |= TextureScrollArea::ChannelG;
	if(ui->actionColorR->isChecked()) channelsMask |= TextureScrollArea::ChannelR;

	ui->textureAreaOriginal->setColorChannel(channelsMask);
	ui->textureAreaConverted->setColorChannel(channelsMask);
}

void TextureBrowser::textureBorderPressed(bool checked)
{
	ui->textureAreaConverted->borderShow(checked);
	ui->textureAreaOriginal->borderShow(checked);
}

void TextureBrowser::textureBgMaskPressed(bool checked)
{
	ui->textureAreaConverted->bgmaskShow(checked);
	ui->textureAreaOriginal->bgmaskShow(checked);
}

void TextureBrowser::texturePropertyChanged(int type)
{
	// settings that need texture to reconvert
	if( type == TextureProperties::PROP_FORMAT ||
		type == TextureProperties::PROP_MIPMAP ||
        type == TextureProperties::PROP_NORMALMAP||
		type == TextureProperties::PROP_SIZE)
	{
		// set current Texture view and force texture convertion
		// new texture will be applyed to scene after conversion (by signal)
		setTextureView(curTextureView, CONVERT_FORCE);
	}
	// other settings don't need texture to reconvert
	else
	{
        const DAVA::TextureDescriptor* descriptor = ui->textureProperties->getTextureDescriptor();
        descriptor->Save();
        // new texture can be applied to scene immediately
        reloadTextureToScene(curTexture, descriptor, DAVA::GPU_ORIGIN);
    }

    // update warning message
    updatePropertiesWarning();
}

void TextureBrowser::textureReadyOriginal(const DAVA::TextureDescriptor *descriptor, const TextureInfo & images)
{
	if(NULL != descriptor)
	{
		if(curDescriptor == descriptor)
		{
			if(descriptor->IsCubeMap())
			{
				ui->textureAreaOriginal->setImage(images.images, descriptor->dataSettings.cubefaceFlags);
			}
			else
			{
				ui->textureAreaOriginal->setImage(images.images[0]);
			}
			ui->textureAreaOriginal->setEnabled(true);
			ui->textureAreaOriginal->waitbarShow(false);

			// set info about original image size info texture properties
			ui->textureProperties->setOriginalImageSize(images.images[0].size());

			// set info about loaded image
			updateInfoOriginal(images.images);

            // zoom fit selected texture
            textureZoomFit(true);
		}
	}
}

void TextureBrowser::textureReadyConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const TextureInfo & images)
{
	if(NULL != descriptor)
	{
        descriptor->Save();

        if (curDescriptor == descriptor && curTextureView == gpu)
        {
            updateConvertedImageAndInfo(images.images, *curDescriptor);
        }

        DAVA::Texture* texture = textureListModel->getTexture(descriptor);
        if (NULL != texture)
        {
            // reload this texture into scene
            reloadTextureToScene(texture, descriptor, gpu);
        }
    }
}

void TextureBrowser::texturePixelOver(const QPoint &pos)
{
	if(ui->textureAreaOriginal->sceneRect().contains(QPointF(pos.x(), pos.y())))
	{
		QColor origColor = ui->textureAreaOriginal->getPixelColor(pos);
		QColor convColor = ui->textureAreaConverted->getPixelColor(pos);

		updateInfoPos(ui->labelOriginalXY, pos);
		updateInfoPos(ui->labelConvertedXY, pos);

		updateInfoColor(ui->labelOriginalRGBA, origColor);
		updateInfoColor(ui->labelConvertedRGBA, convColor);
	}
	else
	{
		// default pos
		updateInfoPos(ui->labelOriginalXY);
		updateInfoPos(ui->labelConvertedXY);

		// default color
		updateInfoColor(ui->labelOriginalRGBA);
		updateInfoColor(ui->labelConvertedRGBA);
	}
}

void TextureBrowser::textureZoomSlide(int value)
{
	float zoom;
	int v;

	if(value > 0)
	{
		value *= 10;
	}

	zoom = 1.0 + (float) value / 100.0;
	v = 100 + value;

	ui->textureAreaOriginal->setTextureZoom(zoom);
	ui->textureAreaConverted->setTextureZoom(zoom);

	toolbarZoomSliderValue->setText(QString("%1%").arg(v));
}

void TextureBrowser::textureZoomFit(bool checked)
{
	if(NULL != curTexture)
	{
		int w = 0;
		int h = 0;

		if(curTexture->IsPinkPlaceholder())
		{
			QImage img = ui->textureAreaOriginal->getImage();
			w = img.width();
			h = img.height();
		}
		else
		{
            if (rhi::TEXTURE_TYPE_CUBE == curTexture->textureType)
            {
                QSize size = ui->textureAreaOriginal->getContentSize();
				w = size.width();
				h = size.height();
			}
			else
			{
				w = curTexture->width;
				h = curTexture->height;
			}
		}

		if(0 != w && 0 != h)
		{
			int v = 0;
			float needWidth = (float) ui->textureAreaOriginal->width();
			float needHeight = (float) ui->textureAreaOriginal->height();
			float scaleX = needWidth / (float) w;
			float scaleY = needHeight / (float) h;
			float scale = DAVA::Min(scaleX, scaleY);

			v = (int) ((scale - 1.0) * 100.0);

			if(v >= 0)
			{
				v = v / 10;
				v -= v % toolbarZoomSlider->singleStep();
			}
			else
			{
				v -= toolbarZoomSlider->singleStep();
				v -= v % toolbarZoomSlider->singleStep();
			}

			toolbarZoomSlider->setValue(v);
		}
	}
}

void TextureBrowser::textureZoom100(bool checked)
{
	toolbarZoomSlider->setValue(0);
}

void TextureBrowser::textureAreaWheel(int delta)
{
	int v = toolbarZoomSlider->value();
	v += delta / 20;
	v -= v % toolbarZoomSlider->singleStep();
	toolbarZoomSlider->setValue(v);
}

void TextureBrowser::textureConver(bool checked)
{
	// re-set current texture view and force conversion
	setTextureView(curTextureView, CONVERT_FORCE);
}

void TextureBrowser::textureConverAll(bool checked)
{
    ConvertMultipleTextures(CONVERT_FORCE);
}

void TextureBrowser::ConvertModifiedTextures(bool)
{
    ConvertMultipleTextures(CONVERT_MODIFIED);
}

void TextureBrowser::ConvertMultipleTextures(eTextureConvertMode convertMode)
{
    if (convertMode != CONVERT_FORCE && convertMode != CONVERT_MODIFIED)
    {
        return;
    }

	DAVA::Scene* activeScene = QtMainWindow::Instance()->GetCurrentScene();
	if(NULL != activeScene)
	{
		QMessageBox msgBox(this);

        QString str = "";
        switch (convertMode)
        {
            case CONVERT_FORCE:
                str = "all";
                break;

            case CONVERT_MODIFIED:
                str = "modified";
                break;

            default:
                DVASSERT(false && "Invalid case");
                break;
        }

        QString msg = QString("You are going to convert %1 textures.").arg(str);
		msgBox.setText(msg);
		msgBox.setInformativeText("This could take a long time. Would you like to continue?");
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		msgBox.setIcon(QMessageBox::Warning);
		int ret = msgBox.exec();

		if(ret == QMessageBox::Ok)
		{
			TextureConvertor::Instance()->Reconvert(activeScene, convertMode);
			TextureConvertor::Instance()->WaitConvertedAll(this);
		}
	}
}

void TextureBrowser::convertStatusImg(const QString &curPath, int curGpu)
{
	if(curPath.isEmpty())
	{
		statusBar->showMessage("Done", 10000);
	}
	else
	{
		QString statusText;
		statusText.sprintf("Converting (%s): ", GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(curGpu));
		statusText += curPath;

		statusBar->showMessage(statusText);
	}
}

void TextureBrowser::convertStatusQueue(int curJob, int jobCount)
{
	if(0 == jobCount)
	{
		statusBarProgress->setVisible(false);
		statusQueueLabel->setText("");
	}
	else
	{
		statusBarProgress->setVisible(true);
		statusBarProgress->setRange(0, jobCount);
		statusBarProgress->setValue(curJob);

		QString queueText;
		statusQueueLabel->setText(queueText.sprintf("%d(%d)", curJob, jobCount));
	}
}

void TextureBrowser::setScene(DAVA::Scene *scene)
{
    if (scene != curScene)
    {
        DAVA::SafeRelease(curScene);
        curScene = DAVA::SafeRetain(scene);
    }

    // reset current texture
    setTexture(nullptr, nullptr);

    // set new scene
    textureListModel->setScene(curScene);
}

void TextureBrowser::sceneActivated(SceneEditor2 *scene)
{
	if(isVisible())
	{
		// set new scene
		if(curScene != scene)
		{
			setScene(scene);
		}
		else
		{
			Update();
		}
	}
}

void TextureBrowser::sceneDeactivated(SceneEditor2 *scene)
{
	if(curScene == scene)
	{
        setScene(nullptr);
    }
}

void TextureBrowser::sceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
	if(!isHidden())
	{
		textureListModel->setHighlight(selected);
	}
}

void TextureBrowser::textureViewChanged(int index)
{
	DAVA::eGPUFamily newView = (DAVA::eGPUFamily) ui->viewTabBar->tabData(index).toInt();
	setTextureView(newView);
}

void TextureBrowser::clearFilter()
{
    ui->textureFilterEdit->setText("");
}
