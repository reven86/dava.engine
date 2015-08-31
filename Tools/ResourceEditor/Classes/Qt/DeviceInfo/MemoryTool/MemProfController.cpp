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


#include <QMessageBox>
#include <QFileDialog>

#include <Utils/UTF8Utils.h>

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Logger.h"
#include "Platform/DateTime.h"
#include "Network/Services/MMNet/MMNetClient.h"

#include "Qt/DeviceInfo/MemoryTool/ProfilingSession.h"
#include "Qt/DeviceInfo/MemoryTool/BacktraceSymbolTable.h"
#include "Qt/DeviceInfo/MemoryTool/MemProfController.h"
#include "Qt/DeviceInfo/MemoryTool/Widgets/MemProfWidget.h"

using namespace DAVA;
using namespace DAVA::Net;

MemProfController::MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget* parentWidget_, QObject* parent)
    : QObject(parent)
    , mode(MODE_NETWORK)
    , parentWidget(parentWidget_)
    , profiledPeer(peerDescr)
    , netClient(new MMNetClient)
    , profilingSession(new ProfilingSession)
{
    ShowView();
    netClient->InstallCallbacks(MakeFunction(this, &MemProfController::NetConnEstablished),
                                MakeFunction(this, &MemProfController::NetConnLost),
                                MakeFunction(this, &MemProfController::NetStatRecieved),
                                MakeFunction(this, &MemProfController::NetSnapshotRecieved));
    connect(this, &MemProfController::SnapshotSaved, this, &MemProfController::OnSnapshotSaved, Qt::QueuedConnection);
}

MemProfController::MemProfController(const DAVA::FilePath& srcDir, QWidget* parentWidget_, QObject *parent)
    : QObject(parent)
    , mode(MODE_FILE)
    , parentWidget(parentWidget_)
    , netClient(new MMNetClient)
    , profilingSession(new ProfilingSession)
{
    if (profilingSession->LoadFromFile(srcDir))
    {
        profiledPeer = profilingSession->DeviceInfo();
        ShowView();
    }
    else
    {
        Logger::Error("Faild to load memory profiling session");
    }
}

MemProfController::~MemProfController() {}

void MemProfController::OnSnapshotPressed()
{
    if (MODE_NETWORK == mode)
    {
        netClient->RequestSnapshot();
    }
}

void MemProfController::ShowView()
{
    if (nullptr == view)
    {
        const QString title = QString("%1 (%2 %3)")
            .arg(profiledPeer.GetName().c_str())
            .arg(profiledPeer.GetPlatformString().c_str())
            .arg(profiledPeer.GetVersion().c_str());

        view = new MemProfWidget(profilingSession.get(), parentWidget);
        view->setWindowFlags(Qt::Window);
        view->setWindowTitle(title);
        if (MODE_FILE == mode)
        {
            view->setAttribute(Qt::WA_DeleteOnClose);
        }

        connect(this, &MemProfController::ConnectionEstablished, view, &MemProfWidget::ConnectionEstablished);
        connect(this, &MemProfController::ConnectionLost, view, &MemProfWidget::ConnectionLost);
        connect(this, &MemProfController::StatArrived, view, &MemProfWidget::StatArrived);
        connect(this, &MemProfController::SnapshotProgress, view, &MemProfWidget::SnapshotProgress, Qt::QueuedConnection);

        connect(view, SIGNAL(OnSnapshotButton()), this, SLOT(OnSnapshotPressed()));
        if (MODE_FILE == mode)
        {
            connect(view, &QObject::destroyed, this, &QObject::deleteLater);
        }
        else
        {
            connect(this, &QObject::destroyed, view, &QObject::deleteLater);
        }
    }
    view->show();
    view->activateWindow();
    view->raise();
}

DAVA::Net::IChannelListener* MemProfController::NetObject() const
{
    return netClient.get();
}

bool MemProfController::IsFileLoaded() const
{
    DVASSERT(MODE_FILE == mode);
    return profilingSession->IsValid();
}

void MemProfController::NetConnEstablished(bool resumed, const DAVA::MMStatConfig* config)
{
    if (!resumed)
    {
        FilePath path;
        ComposeFilePath(path);
        if (profilingSession->StartNew(config, profiledPeer, path))
        {
            emit ConnectionEstablished(!resumed);
        }
        else
        {
            Logger::Error("Failed to start new memory profiling session");
        }
    }
}

void MemProfController::NetConnLost(const DAVA::char8* message)
{
    profilingSession->Flush();
    emit ConnectionLost(message);
}

void MemProfController::NetStatRecieved(const DAVA::MMCurStat* stat, uint32 count)
{
    profilingSession->AppendStatItems(stat, count);
    emit StatArrived(count);
}

void MemProfController::NetSnapshotRecieved(uint32 totalSize, uint32 chunkOffset, uint32 chunkSize, const uint8* chunk)
{
    if (nullptr == chunk)   // Error while transferring snapshot
    {
        SafeRelease(snapshotFile);
        FileSystem::Instance()->DeleteFile(snapshotTempName);
        emit SnapshotProgress(0, 0);
        return;
    }

    if (!snapshotInProgress)
    {
        snapshotInProgress = true;
        snapshotTotalSize = totalSize;
        snapshotRecvSize = 0;
        snapshotTempName = profilingSession->GenerateSnapshotFilename();
        snapshotFile = File::Create(snapshotTempName, File::CREATE | File::WRITE);
        if (nullptr == snapshotFile)
        {
            Logger::Error("[MemProfController] Failed to create temporal snapshot file");
        }
        emit SnapshotProgress(totalSize, 0);
    }

    if (snapshotFile != nullptr)
    {
        snapshotFile->Write(chunk, chunkSize);
    }
    snapshotRecvSize += chunkSize;
    emit SnapshotProgress(totalSize, snapshotRecvSize);

    if (snapshotRecvSize == snapshotTotalSize)
    {
        snapshotInProgress = false;
        SafeRelease(snapshotFile);
        emit SnapshotSaved(new FilePath(snapshotTempName));
    }
}

void MemProfController::OnSnapshotSaved(const DAVA::FilePath* filePath)
{
    profilingSession->AppendSnapshot(*filePath);
    delete filePath;
}

void MemProfController::ComposeFilePath(DAVA::FilePath& result)
{
    String level1 = Format("%s %s/",
                           profiledPeer.GetManufacturer().c_str(),
                           profiledPeer.GetModel().c_str());
    String level2 = Format("%s {%s}/",
                           profiledPeer.GetName().c_str(),
                           profiledPeer.GetUDID().c_str());

    DateTime now = DateTime::Now();
    String level3 = Format("%04d-%02d-%02d %02d%02d%02d/",
                           now.GetYear(), now.GetMonth() + 1, now.GetDay(),
                           now.GetHour(), now.GetMinute(), now.GetSecond());

    result = "~doc:/";
    result += "memory-profiling/";
    result += profiledPeer.GetPlatformString();
    result += "/";
    result += level1;
    result += level2;
    result += level3;
}
