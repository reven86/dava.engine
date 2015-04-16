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


#ifndef _QT_CUBEMAPEDITORDIALOG_H_
#define _QT_CUBEMAPEDITORDIALOG_H_

#include <QDialog>
#include <QImage>
#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Render/Image/ImageSystem.h"

class ClickableQLabel;

namespace Ui {
class CubemapEditorDialog;
}

class CubemapEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CubemapEditorDialog(QWidget *parent = 0);
    ~CubemapEditorDialog();

    void InitForEditing(DAVA::FilePath& textureDescriptorPath, DAVA::FilePath& rootPath);
    void InitForCreating(DAVA::FilePath& textureDescriptorPath, DAVA::FilePath& rootPath);

public slots:
    
    virtual void done(int);
    
protected:
    typedef enum{
        eEditorModeNone,
        eEditorModeEditing,
        eEditorModeCreating
    } eEditorMode;

protected:
    DAVA::ImageInfo facesInfo;
    DAVA::Vector<DAVA::FilePath> facePathes;
    QString rootPath;

    bool faceChanged;

    eEditorMode editorMode;
    DAVA::FilePath targetFile;

protected:
    void ConnectSignals();
    void LoadImageFromUserFile(float rotation, int face);
    bool VerifyFirstImage(DAVA::ImageInfo imgInfo, QString &errorString);
    bool VerifyNextImage(DAVA::ImageInfo imgInfo, QString &errorString);
    bool IsFormatValid(const DAVA::ImageInfo &info);
    void UpdateFaceInfo();
    void UpdateButtonState();
    bool AnyFaceLoaded();
    bool AllFacesLoaded();
    int GetLoadedFaceCount();
    void LoadCubemap(const QString& path);
    void SaveCubemap(const QString& path);
    DAVA::uint8 GetFaceMask();
    bool LoadImageTo(const DAVA::FilePath& filePath, int face, bool silent);
    ClickableQLabel* GetLabelForFace(int face);
    bool IsCubemapEdited();

    void mouseMoveEvent(QMouseEvent *ev);

protected slots:
    void OnPXClicked();
    void OnNXClicked();
    void OnPYClicked();
    void OnNYClicked();
    void OnPZClicked();
    void OnNZClicked();
    void OnRotationChanged();

    void OnLoadTexture();
    void OnSave();

private:
    Ui::CubemapEditorDialog *ui;
};

#endif // _QT_CUBEMAPEDITORDIALOG_H_
