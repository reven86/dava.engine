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


#ifndef __SPRITES_PACKER_MODULE_H__
#define __SPRITES_PACKER_MODULE_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

#include "TextureCompression/TextureConverter.h"

#include <QObject>

namespace DAVA
{
class AssetCacheClient;
}

class SpritesPacker;
class QAction;
class QDialog;

class SpritesPackerModule final : public QObject
{
    Q_OBJECT

public:
    SpritesPackerModule();
    ~SpritesPackerModule() override;

    QAction* GetReloadAction() const;
    void SetAction(QAction* reloadSpritesAction);

    void RepackImmediately(const DAVA::FilePath& projectPath, DAVA::eGPUFamily gpu);

signals:
    void SpritesReloaded();

private slots:

    void RepackWithDialog();

private:
    void SetupSpritesPacker(const DAVA::FilePath& projectPath);
    void ShowPackerDialog();
    void ReloadObjects();

    void ConnectCacheClient();
    void DisconnectCacheClient();
    void DisconnectCacheClientInternal(DAVA::AssetCacheClient* cacheClient);

    void ProcessSilentPacking(bool clearDirs, bool forceRepack, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality);

    void CreateWaitDialog(const DAVA::FilePath& projectPath);
    void CloseWaitDialog();

private:
    DAVA::AssetCacheClient* cacheClient = nullptr;

    std::unique_ptr<SpritesPacker> spritesPacker;
    QAction* reloadSpritesAction = nullptr;

    QDialog* waitDialog = nullptr;
};

#endif // __SPRITES_PACKER_MODULE_H__
