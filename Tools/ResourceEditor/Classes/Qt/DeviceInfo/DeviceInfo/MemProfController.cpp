#include <QMessageBox>
#include <QFileDialog>

#include <unordered_map>

#include <Utils/UTF8Utils.h>

#include "Base/FunctionTraits.h"
#include "FileSystem/FileSystem.h"

#include "MemProfWidget.h"
#include "MemProfController.h"
#include "ProfilingSession.h"
#include "BacktraceSymbolTable.h"

using namespace DAVA;
using namespace DAVA::Net;

MemProfController::MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *_parentWidget, QObject* parent)
    : QObject(parent)
    , parentWidget(_parentWidget)
    , peer(peerDescr)
{
    ShowView();
    netClient.SetCallbacks(MakeFunction(this, &MemProfController::OnChannelOpen),
                           MakeFunction(this, &MemProfController::OnChannelClosed),
                           MakeFunction(this, &MemProfController::OnCurrentStat),
                           MakeFunction(this, &MemProfController::OnDump));
}

MemProfController::~MemProfController() {}

void MemProfController::OnDumpPressed()
{
    netClient.RequestDump();
}

void MemProfController::ShowView()
{
    if (NULL == view)
    {
        const QString title = QString("%1 (%2 %3)")
            .arg(peer.GetName().c_str())
            .arg(peer.GetPlatformString().c_str())
            .arg(peer.GetVersion().c_str());

        view = new MemProfWidget(parentWidget);
        view->setWindowFlags(Qt::Window);
        view->setWindowTitle(title);

        connect(this, &MemProfController::ConnectionEstablished, view, &MemProfWidget::ConnectionEstablished);
        connect(this, &MemProfController::ConnectionLost, view, &MemProfWidget::ConnectionLost);
        connect(this, &MemProfController::StatArrived, view, &MemProfWidget::StatArrived);
        connect(this, &MemProfController::DumpArrived, view, &MemProfWidget::DumpArrived);

        connect(view, SIGNAL(OnDumpButton()), this, SLOT(OnDumpPressed()));
        connect(view, SIGNAL(OnViewDumpButton()), this, SLOT(OnViewDump()));
        connect(view, SIGNAL(OnViewFileDumpButton()), this, SLOT(OnViewFileDump()));
        connect(this, &QObject::destroyed, view, &QObject::deleteLater);
    }
    view->showNormal();
    view->activateWindow();
    view->raise();
}

void MemProfController::OnChannelOpen(const DAVA::MMStatConfig* config)
{
    bool newConnection = false;
    if (config != nullptr)
    {
        newConnection = true;
        profilingSession = std::make_unique<ProfilingSession>(config, peer);
    }
    emit ConnectionEstablished(newConnection, profilingSession.get());
}

void MemProfController::OnChannelClosed(const char8* message)
{
    if (profilingSession)
    {
        profilingSession->Flush();
        emit ConnectionLost(message);
    }
}

void MemProfController::OnCurrentStat(const DAVA::MMCurStat* stat)
{
    profilingSession->AddStatItem(stat);
    emit StatArrived();
}

void MemProfController::OnDump(size_t total, size_t recv, Vector<uint8>* v)
{
    if (total == recv)
    {
        DVASSERT(v != nullptr);

        const MMDump* dump = reinterpret_cast<const MMDump*>(v->data());
        const MMCurStat* stat = OffsetPointer<MMCurStat>(dump, sizeof(MMDump));
        OnCurrentStat(stat);

        profilingSession->AddDump(dump);
    }
    emit DumpArrived(total, recv);
}
