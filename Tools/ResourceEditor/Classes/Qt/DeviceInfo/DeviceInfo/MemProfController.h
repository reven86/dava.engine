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


#ifndef __MEMPROFCONTROLLER_H__
#define __MEMPROFCONTROLLER_H__

#include <QObject>
#include <QPointer>

#include "FileSystem/FilePath.h"
#include "Network/PeerDesription.h"
#include "MemoryManager/MemoryManagerTypes.h"

namespace DAVA
{
class File;
namespace Net
{
    struct IChannelListener;
    class MMNetClient;
}   // namespace Net
}   // namespace DAVA

class MemProfWidget;
class ProfilingSession;

class MemProfController : public QObject
{
    Q_OBJECT
    
public:
    enum eMode
    {
        MODE_NETWORK = 0,
        MODE_FILE,
        MODE_SELFPROFILING
    };

public:
    MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *parentWidget, QObject *parent = nullptr);
    MemProfController(const DAVA::FilePath& srcDir, QWidget *parentWidget, QObject *parent = nullptr);
    ~MemProfController();

    void ShowView();

    DAVA::Net::IChannelListener* NetObject() const;

    int Mode() const;
    bool IsFileLoaded() const;

    void NetConnEstablished(bool resumed, const DAVA::MMStatConfig* config);
    void NetConnLost(const DAVA::char8* message);
    void NetStatRecieved(const DAVA::MMCurStat* stat, DAVA::uint32 count);
    void NetSnapshotRecieved(DAVA::uint32 totalSize, DAVA::uint32 chunkOffset, DAVA::uint32 chunkSize, const DAVA::uint8* chunk);

signals:
    void ConnectionEstablished(bool newConnection);
    void ConnectionLost(const DAVA::char8* message);
    void StatArrived(DAVA::uint32 itemCount);
    void SnapshotProgress(DAVA::uint32 totalSize, DAVA::uint32 recvSize);
    void SnapshotSaved(const DAVA::FilePath* filePath);

public slots:
    void OnSnapshotPressed();

private slots:
    void OnSnapshotSaved(const DAVA::FilePath* filePath);

private:
    void ComposeFilePath(DAVA::FilePath& result);

private:
    int mode;
    QPointer<MemProfWidget> view;
    QPointer<QWidget> parentWidget;

    bool snapshotInProgress = false;
    DAVA::FilePath snapshotTempName;
    DAVA::File* snapshotFile = nullptr;
    DAVA::uint32 snapshotTotalSize = 0;
    DAVA::uint32 snapshotRecvSize = 0;

    DAVA::Net::PeerDescription profiledPeer;
    std::unique_ptr<DAVA::Net::MMNetClient> netClient;
    std::unique_ptr<ProfilingSession> profilingSession;
};

//////////////////////////////////////////////////////////////////////////
inline int MemProfController::Mode() const
{
    return mode;
}

#endif // __MEMPROFCONTROLLER_H__
