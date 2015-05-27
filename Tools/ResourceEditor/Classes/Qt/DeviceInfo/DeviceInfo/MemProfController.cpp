#include <QMessageBox>
#include <QFileDialog>

#include <Utils/UTF8Utils.h>

#include "Base/FunctionTraits.h"
#include "FileSystem/FileSystem.h"
#include "Network/Services/MMNet/MMNetClient.h"

#include "MemProfWidget.h"
#include "MemProfController.h"
#include "ProfilingSession.h"
#include "BacktraceSymbolTable.h"

using namespace DAVA;
using namespace DAVA::Net;

MemProfController::MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget* parentWidget_, QObject* parent)
    : QObject(parent)
    , parentWidget(parentWidget_)
    , profiledPeer(peerDescr)
    , netClient(new MMNetClient)
    , profilingSession(new ProfilingSession)
{
    ShowView();
    netClient->InstallCallbacks(MakeFunction(this, &MemProfController::NetConnEstablished),
                                MakeFunction(this, &MemProfController::NetConnLost),
                                MakeFunction(this, &MemProfController::NetStatRecieved),
                                MakeFunction(this, &MemProfController::NetDumpRecieved));
}

MemProfController::MemProfController(const DAVA::FilePath& srcDir, QWidget* parentWidget_, QObject *parent)
    : QObject(parent)
    , parentWidget(parentWidget_)
    , netClient(new MMNetClient)
    , profilingSession(new ProfilingSession)
{
    if (profilingSession->LoadFromFile(srcDir))
    {
        ShowView();
    }
}

MemProfController::~MemProfController() {}

void MemProfController::OnDumpPressed()
{
    netClient->RequestDump();
}

void MemProfController::ShowView()
{
    if (nullptr == view)
    {
        const QString title = QString("%1 (%2 %3)")
            .arg(profiledPeer.GetName().c_str())
            .arg(profiledPeer.GetPlatformString().c_str())
            .arg(profiledPeer.GetVersion().c_str());

        view = new MemProfWidget(parentWidget);
        view->setWindowFlags(Qt::Window);
        view->setWindowTitle(title);

        connect(this, &MemProfController::ConnectionEstablished, view, &MemProfWidget::ConnectionEstablished);
        connect(this, &MemProfController::ConnectionLost, view, &MemProfWidget::ConnectionLost);
        connect(this, &MemProfController::StatArrived, view, &MemProfWidget::StatArrived);
        connect(this, &MemProfController::DumpArrived, view, &MemProfWidget::DumpArrived);

        connect(view, SIGNAL(OnDumpButton()), this, SLOT(OnDumpPressed()));
        connect(this, &QObject::destroyed, view, &QObject::deleteLater);
    }
    view->showNormal();
    view->activateWindow();
    view->raise();
}

DAVA::Net::IChannelListener* MemProfController::NetObject() const
{
    return netClient.get();
}

void MemProfController::NetConnEstablished(bool resumed, const DAVA::MMStatConfig* config)
{
    if (!resumed)
    {
        profilingSession->StartNew(config, profiledPeer, FilePath("d:\\temp\\memory"));
    }
    emit ConnectionEstablished(!resumed, profilingSession.get());
}

void MemProfController::NetConnLost(const DAVA::char8* message)
{
    if (profilingSession)
    {
        profilingSession->Flush();
        emit ConnectionLost(message);
    }
}

void MemProfController::NetStatRecieved(const DAVA::MMCurStat* stat, size_t count)
{
    profilingSession->AppendStatItems(stat, count);
    emit StatArrived();
}

void MemProfController::NetDumpRecieved(int stage, size_t totalSize, size_t recvSize, const void* data)
{
    switch (stage)
    {
    case MMNetClient::DUMP_STAGE_STARTED:
        emit DumpArrived(totalSize, 0);
        break;
    case MMNetClient::DUMP_STAGE_PROGRESS:
        emit DumpArrived(totalSize, recvSize);
        break;
    case MMNetClient::DUMP_STAGE_FINISHED:
        profilingSession->AppendSnapshot(static_cast<const MMDump*>(data));
        emit DumpArrived(totalSize, recvSize);
        break;
    case MMNetClient::DUMP_STAGE_ERROR:
        emit DumpArrived(0, 0);
        break;
    default:
        break;
    }
}
