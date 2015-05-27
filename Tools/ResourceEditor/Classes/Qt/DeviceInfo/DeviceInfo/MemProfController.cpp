#include <QMessageBox>
#include <QFileDialog>

#include <Utils/UTF8Utils.h>

#include "Base/FunctionTraits.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Logger.h"
#include "Platform/DateTime.h"
#include "Network/Services/MMNet/MMNetClient.h"

#include "MemProfWidget.h"
#include "MemProfController.h"
#include "ProfilingSession.h"
#include "BacktraceSymbolTable.h"

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
        connect(this, &MemProfController::SnapshotArrived, view, &MemProfWidget::SnapshotArrived);

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
            Logger::Error("Faild to start new memory profiling session");
        }
    }
}

void MemProfController::NetConnLost(const DAVA::char8* message)
{
    profilingSession->Flush();
    emit ConnectionLost(message);
}

void MemProfController::NetStatRecieved(const DAVA::MMCurStat* stat, size_t count)
{
    profilingSession->AppendStatItems(stat, count);
    emit StatArrived(count);
}

void MemProfController::NetSnapshotRecieved(int stage, size_t totalSize, size_t recvSize, const void* data)
{
    switch (stage)
    {
    case MMNetClient::SNAPSHOT_STAGE_STARTED:
        emit SnapshotArrived(totalSize, 0);
        break;
    case MMNetClient::SNAPSHOT_STAGE_PROGRESS:
        emit SnapshotArrived(totalSize, recvSize);
        break;
    case MMNetClient::SNAPSHOT_STAGE_FINISHED:
        profilingSession->AppendSnapshot(static_cast<const MMSnapshot*>(data));
        emit SnapshotArrived(totalSize, recvSize);
        break;
    case MMNetClient::SNAPSHOT_STAGE_ERROR:
        emit SnapshotArrived(0, 0);
        break;
    default:
        break;
    }
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
                           now.GetYear(), now.GetMonth(), now.GetDay(),
                           now.GetHour(), now.GetMinute(), now.GetSecond());

    result = "~doc:/";
    result += "memory-profiling/";
    result += profiledPeer.GetPlatformString();
    result += "/";
    result += level1;
    result += level2;
    result += level3;
}
