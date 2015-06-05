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
#include "TexturePacker/ResourcePacker2D.h"
#include <TexturePacker/CommandLineParser.h>
#include "Classes/Helpers/ResourcesManageHelper.h"
#include "Platform/Qt5/QtLayer.h"
#include "Render/2D/Sprite.h"
#include <QDirIterator>
#include <QApplication>
#include <QtConcurrent>

using namespace DAVA;

SpritesPacker::SpritesPacker(QObject* parent)
    : QObject(parent)
    , resourcePacker2D(new ResourcePacker2D())
    , running(false)
{
    resourcePacker2D->isLightmapsPacking = true;
}

SpritesPacker::~SpritesPacker()
{
    delete resourcePacker2D;
}

void SpritesPacker::ReloadSprites(bool clearDirs, const eGPUFamily gpu, const TextureConverter::eConvertQuality quality)
{
    process = QtConcurrent::run(this, &SpritesPacker::ReloadSpritePrivate, clearDirs, gpu, quality);
}

void SpritesPacker::ReloadSpritePrivate(bool clearDirs, const eGPUFamily gpu, const TextureConverter::eConvertQuality quality)
{
    if (!ProcessStared())
    {
        return;
    }
    SetRunning(true);
    QString currentProjectPath = ResourcesManageHelper::GetProjectPath();
    QDir inputDir(currentProjectPath + "/DataSource");
    QDirIterator it(inputDir);
    void *pool = QtLayer::Instance()->CreateAutoreleasePool();
    resourcePacker2D->SetRunning(true);
    while (it.hasNext() && resourcePacker2D->IsRunning())
    {
        const QFileInfo &fileInfo = it.fileInfo();
        if (fileInfo.isDir() && fileInfo.absoluteFilePath().contains("gfx", Qt::CaseInsensitive))
        {
            QString outputPath = fileInfo.absoluteFilePath();
            outputPath.replace(outputPath.indexOf("DataSource"), QString("DataSource").size(), "Data");
            QDir outputDir(outputPath);
            if (!outputDir.exists())
            {
                outputDir.mkdir(".");
            }

            const FilePath inputFilePath = FilePath(QDir::toNativeSeparators(fileInfo.absoluteFilePath()).toStdString()).MakeDirectoryPathname();
            const FilePath outputFilePath = FilePath(QDir::toNativeSeparators(outputDir.absolutePath()).toStdString()).MakeDirectoryPathname();
            CommandLineParser::Instance()->Clear(); //CommandLineParser is used in ResourcePackerScreen
            resourcePacker2D->clearProcessDirectory = clearDirs;
            resourcePacker2D->SetConvertQuality(quality);
            resourcePacker2D->InitFolders(inputFilePath, outputFilePath);
            resourcePacker2D->PackResources(gpu);
        }
        it.next();
    }
    QtLayer::Instance()->ReleaseAutoreleasePool(pool);
    Sprite::ReloadSprites();
    SetRunning(false);
}

void SpritesPacker::Cancel()
{
    resourcePacker2D->SetRunning(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    while (!process.isFinished())
    {
        QApplication::processEvents();
    }
    QApplication::restoreOverrideCursor();
}

void SpritesPacker::Stop()
{
    resourcePacker2D->SetRunning(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    process.waitForFinished();
    QApplication::restoreOverrideCursor();
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
        DAVA::String message = DAVA::String("Sprites packer ") + (arg ? "started" : "finished");
        Logger::Debug(message.c_str());
        emit RunningStateChanged(arg);
    }
}
