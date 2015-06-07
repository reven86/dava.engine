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

#include "Render/RenderBase.h"
#include "TextureCompression/TextureConverter.h"
#include <QObject>
#include <QFuture>
#include <QRegularExpression>
#include <atomic>

namespace DAVA {
    class ResourcePacker2D;
}

class SpritesPacker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ IsRunning WRITE SetRunning NOTIFY RunningStateChanged);
    Q_PROPERTY(QString projectPath READ GetProjectPath WRITE SetProjectPath NOTIFY ProjectPathChanged)
    Q_PROPERTY(QRegularExpression searchPattern READ GetSearchPattern WRITE SetSearchPattern NOTIFY SearchPatternChanged)
public:
    SpritesPacker(QObject *parent = nullptr);
    ~SpritesPacker();
    void ReloadSprites(bool clearDirs, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality);
public slots:
    void Cancel();
    void Stop();
signals:
    bool ProcessStared();
private:
    void ReloadSpritePrivate(bool clearDirs, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality);
    DAVA::ResourcePacker2D *resourcePacker2D;
    QFuture<void> process;

    //properties section
public:
    bool IsRunning() const;
public slots:
    void SetRunning(bool arg);
signals:
    void RunningStateChanged(bool arg);
private:
    std::atomic<bool> running;

public:
    QString GetProjectPath() const;
public slots:
    void SetProjectPath(QString arg);
signals:
    void ProjectPathChanged(QString arg);
private:
    QString projectPath;
    
public:
    QRegularExpression GetSearchPattern() const;
public slots:
    void SetSearchPattern(QRegularExpression arg);
signals:
    void SearchPatternChanged(QRegularExpression arg);
private:
    QRegularExpression searchPattern;
};

#endif //__SPRITES_PACKER_H__