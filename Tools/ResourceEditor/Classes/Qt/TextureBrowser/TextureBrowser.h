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


#ifndef __TEXTURE_BROWSER_H__
#define __TEXTURE_BROWSER_H__

#include <QDialog>
#include <QMap>
#include "DAVAEngine.h"
#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Scene/SceneSignals.h"

#include "TextureInfo.h"
#include "TextureConvertMode.h"

class QModelIndex;
class TextureListDelegate;
class TextureListModel;
class TextureConvertor;
class QAbstractItemDelegate;
class QStatusBar;
class QLabel;
class QProgressBar;
class QSlider;
struct JobItem;

namespace Ui {
class TextureBrowser;
}

class TextureBrowser : public QDialog, public DAVA::Singleton<TextureBrowser>
{
    Q_OBJECT

public:
    explicit TextureBrowser(QWidget *parent = 0);
    ~TextureBrowser();

	void Close();
	void Update();

	static QColor gpuColor_PVR_ISO;
	static QColor gpuColor_PVR_Android;
	static QColor gpuColor_Tegra;
	static QColor gpuColor_MALI;
	static QColor gpuColor_Adreno;
    static QColor gpuColor_DX11;
    static QColor errorColor;

protected:
	void closeEvent(QCloseEvent * e) override;

public slots:
	void sceneActivated(SceneEditor2 *scene);
	void sceneDeactivated(SceneEditor2 *scene);
	void sceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected);

private:
    Ui::TextureBrowser *ui;
	QtPosSaver posSaver;

	TextureListModel *textureListModel;
	TextureListDelegate *textureListImagesDelegate;

	QSlider *toolbarZoomSlider;
	QLabel *toolbarZoomSliderValue;
	
	QStatusBar *statusBar;
	QLabel *statusQueueLabel;
	QProgressBar *statusBarProgress;
	
	QMap<QString, int> textureListSortModes;
	QMap<int, DAVA::eGPUFamily> tabIndexToViewMode;

	DAVA::Scene *curScene;
	DAVA::eGPUFamily curTextureView;

	DAVA::Texture *curTexture;
	DAVA::TextureDescriptor *curDescriptor;

	void setScene(DAVA::Scene *scene);

	void setupTextureListToolbar();
	void setupTextureToolbar();
	void setupTexturesList();
	void setupImagesScrollAreas();
	void setupTextureListFilter();
	void setupStatusBar();
	void setupTextureProperties();
	void setupTextureViewTabBar();
	
	void resetTextureInfo();

	void setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor);
	void setTextureView(DAVA::eGPUFamily view, eTextureConvertMode convertMode = CONVERT_NOT_EXISTENT);

	void updateConvertedImageAndInfo(const QList<QImage> &images, DAVA::TextureDescriptor& descriptor);
	void updateInfoColor(QLabel *label, const QColor &color = QColor());
	void updateInfoPos(QLabel *label, const QPoint &pos = QPoint());
	void updateInfoOriginal(const QList<QImage> &images);
	void updateInfoConverted();
	void updatePropertiesWarning();

	void reloadTextureToScene(DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu);

    void ConvertMultipleTextures(eTextureConvertMode convertMode);

    void UpdateSceneMaterialsWithTexture(DAVA::Texture* texture);

private slots:
	void textureListViewImages(bool checked);
	void textureListViewText(bool checked);
	void textureListFilterChanged(const QString &text);
	void textureListFilterSelectedNodeChanged(bool checked);
	void textureListSortChanged(const QString &text);
	void texturePressed(const QModelIndex & index);
	void textureColorChannelPressed(bool checked);
	void textureBorderPressed(bool checked);
	void textureBgMaskPressed(bool checked);
	void texturePropertyChanged(int type);
	void textureReadyOriginal(const DAVA::TextureDescriptor *descriptor, const TextureInfo & images);
	void textureReadyConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const TextureInfo & images);
	void texturePixelOver(const QPoint &pos);
	void textureZoomSlide(int value);
	void textureZoom100(bool checked);
	void textureZoomFit(bool checked);
	void textureAreaWheel(int delta);
	void textureConver(bool checked);
	void textureConverAll(bool checked);
	void textureViewChanged(int index);
    void ConvertModifiedTextures(bool);

	void convertStatusImg(const QString &curPath, int curGpu);
	void convertStatusQueue(int curJob, int jobCount);
    
    void clearFilter();
};

#endif // __TEXTURE_BROWSER_H__
