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


#include "SpritesPacker.h"
#include <TexturePacker/CommandLineParser.h>
#include "Platform/Qt5/QtLayer.h"
#include "Render/2D/Sprite.h"
#include <QDirIterator>


using namespace DAVA;

SpritesPacker::SpritesPacker(QObject* parent)
    : QObject(parent)
    , running(false)
{
}

SpritesPacker::~SpritesPacker()
{
}

void SpritesPacker::SetCacheTool(const String& ip, const String& port, const String& timeout)
{
    resourcePacker2D.SetCacheClientTool("~res:/AssetCacheClient", ip, port, timeout);
}

void SpritesPacker::ClearCacheTool()
{
    resourcePacker2D.ClearCacheClientTool();
}

void SpritesPacker::AddTask(const QDir &inputDir, const QDir &outputDir)
{
    tasks.push_back(qMakePair(inputDir, outputDir));
}

void SpritesPacker::ClearTasks()
{
    tasks.clear();
}

void SpritesPacker::ReloadSprites(bool clearDirs, const eGPUFamily gpu, const TextureConverter::eConvertQuality quality)
{
    SetRunning(true);
    void *pool = QtLayer::Instance()->CreateAutoreleasePool();
    resourcePacker2D.SetRunning(true);
    for (const auto &task : tasks)
    {
        const auto &inputDir = task.first;
        const auto &outputDir = task.second;
        if (!outputDir.exists())
        {
            outputDir.mkdir(".");
        }

        const FilePath inputFilePath = FilePath(inputDir.absolutePath().toStdString()).MakeDirectoryPathname();
        const FilePath outputFilePath = FilePath(outputDir.absolutePath().toStdString()).MakeDirectoryPathname();

        resourcePacker2D.forceRepack = true;
        resourcePacker2D.clearOutputDirectory = clearDirs;
        resourcePacker2D.SetConvertQuality(quality);
        resourcePacker2D.InitFolders(inputFilePath, outputFilePath);
        resourcePacker2D.PackResources(gpu);
        if (!resourcePacker2D.IsRunning())
        {
            break;
        }

    }
    QtLayer::Instance()->ReleaseAutoreleasePool(pool);
    Sprite::ReloadSprites();
    SetRunning(false);
}

void SpritesPacker::Cancel()
{
    resourcePacker2D.SetRunning(false);
}

bool SpritesPacker::IsRunning() const
{
    return running;
}

void SpritesPacker::SetRunning(bool arg)
{
    if (arg != running)
    {
        running = arg;
        if(!arg)
        {
            emit Finished();
        }
        String message = String("Sprites packer ") + (arg ? "started" : (resourcePacker2D.IsRunning() ? "finished" : "canceled"));
        Logger::Debug(message.c_str());
        emit RunningStateChanged(arg);
    }
}
