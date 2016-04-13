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


#ifndef __SPRITES_PACKER_H__
#define __SPRITES_PACKER_H__


#include "Base/Introspection.h"
#include "Render/RenderBase.h"
#include "TextureCompression/TextureConverter.h"
#include "TexturePacker/ResourcePacker2D.h"
#include <QObject>
#include <QDir>
#include <atomic>

namespace DAVA
{
class ResourcePacker2D;
class AssetCacheClient;
}

class SpritesPacker : public QObject, DAVA::InspBase
{
    Q_OBJECT
    Q_PROPERTY(bool running READ IsRunning WRITE SetRunning NOTIFY RunningStateChanged);

public:
    SpritesPacker(QObject* parent = nullptr);

    void SetCacheClient(DAVA::AssetCacheClient* cacheClient, const DAVA::String& comment);

    void AddTask(const QDir& inputDir, const QDir& outputDir);
    void ClearTasks();
    Q_INVOKABLE void ReloadSprites(bool clearDirs, bool forceRepack, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality);

    const DAVA::ResourcePacker2D& GetResourcePacker() const;

public slots:
    void Cancel();
signals:
    void Finished();

private:
    DAVA::ResourcePacker2D resourcePacker2D;
    QList<QPair<QDir, QDir>> tasks;

    //properties section
public:
    bool IsRunning() const;
public slots:
    void SetRunning(bool arg);
signals:
    void RunningStateChanged(bool arg);

private:
    std::atomic<bool> running;

    bool IsUsingAssetCache() const;
    void SetUsingAssetCacheEnabled(bool enabled);

    DAVA::String GetAssetCacheIp() const;
    void SetAssetCacheIp(const DAVA::String& ip);

    DAVA::String GetAssetCachePort() const;
    void SetAssetCachePort(const DAVA::String& port);

    DAVA::String GetAssetCacheTimeout() const;
    void SetAssetCacheTimeout(const DAVA::String& timeout);

public:
    INTROSPECTION(SpritesPacker,
                  PROPERTY("isUsingAssetCache", "Use asset cache", IsUsingAssetCache, SetUsingAssetCacheEnabled, DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  PROPERTY("assetCacheIp", "Asset Cache IP", GetAssetCacheIp, SetAssetCacheIp, DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  PROPERTY("assetCachePort", "Asset Cache Port", GetAssetCachePort, SetAssetCachePort, DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  PROPERTY("assetCacheTimeout", "Asset Cache Timeout", GetAssetCacheTimeout, SetAssetCacheTimeout, DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  )
};

#endif //__SPRITES_PACKER_H__