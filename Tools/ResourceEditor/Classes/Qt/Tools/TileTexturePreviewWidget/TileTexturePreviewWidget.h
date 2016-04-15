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


#ifndef __RESOURCEEDITORQT__TILETEXTUREPREVIEWWIDGET__
#define __RESOURCEEDITORQT__TILETEXTUREPREVIEWWIDGET__

#include "DAVAEngine.h"

#include <QTreeWidget>

class QLabel;

class TileTexturePreviewWidget : public QTreeWidget
{
    Q_OBJECT

public:
    enum eWidgetModes
    {
        MODE_WITH_COLORS = 0,
        MODE_WITHOUT_COLORS,

        MODES_COUNT
    };

    static const DAVA::int32 TEXTURE_PREVIEW_WIDTH = 180;
    static const DAVA::int32 TEXTURE_PREVIEW_HEIGHT = 32;

    explicit TileTexturePreviewWidget(QWidget* parent = 0);
    ~TileTexturePreviewWidget();

    void AddTexture(DAVA::Image* previewTexture, const DAVA::Color& color = DAVA::Color::White);
    void UpdateColor(DAVA::uint32 index, const DAVA::Color& color);

    DAVA::uint32 GetSelectedTexture();
    void SetSelectedTexture(DAVA::uint32 number);

    void SetMode(eWidgetModes mode);

    void Clear();

protected:
    virtual bool eventFilter(QObject* obj, QEvent* ev);

signals:
    void SelectionChanged(int selectedTexture);
    void TileColorChanged(DAVA::int32 tileNumber, DAVA::Color color);

private slots:
    void OnCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void OnItemChanged(QTreeWidgetItem* item, int column);

private:
    static const DAVA::int32 COLOR_PREVIEW_COLUMN = 1;
    static const DAVA::int32 COLOR_SELECT_BUTTON_COLUMN = 2;
    static const DAVA::int32 DEF_TILE_TEXTURES_COUNT = 4;
    static const DAVA::int32 TEXTURE_PREVIEW_WIDTH_SMALL = 110;

    DAVA::int32 selectedTexture;

    DAVA::Vector<DAVA::Color> colors;
    DAVA::Vector<DAVA::Image*> images;
    DAVA::Vector<QLabel*> labels;

    eWidgetModes mode;

    QRegExpValidator* validator;

    void ConnectToSignals();

    void SetColor(DAVA::uint32 number, const DAVA::Color& color);
    void UpdateImage(DAVA::uint32 number);
    void UpdateColor(DAVA::uint32 number);
    void UpdateSelection();

    void InitWithColors();
    void InitWithoutColors();

    DAVA::Image* MultiplyImageWithColor(DAVA::Image* image, const DAVA::Color& color);
};

#endif /* defined(__RESOURCEEDITORQT__TILETEXTUREPREVIEWWIDGET__) */
