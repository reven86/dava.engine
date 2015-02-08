#include <Utils/UTF8Utils.h>

#include "MemProfWidget.h"
#include "MemProfController.h"

using namespace DAVA;
using namespace DAVA::Net;

MemProfController::MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *_parentWidget, QObject* parent)
    : QObject(parent)
    , parentWidget(_parentWidget)
    , peer(peerDescr)
{
    ShowView();
}

MemProfController::~MemProfController() {}

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

        connect(this, &QObject::destroyed, view, &QObject::deleteLater);
    }
    view->showNormal();
    view->activateWindow();
    view->raise();
}

void MemProfController::ChannelOpen()
{
    view->ChangeStatus("connected", nullptr);
    view->ClearStat();
}

void MemProfController::ChannelClosed(const char8* message)
{
    view->ChangeStatus("disconnected", message);
    for (auto* x : v)
        delete x;
    v.clear();
}

void MemProfController::PacketReceived(const void* packet, size_t length)
{
    const net_mem_stat_t* src = static_cast<const net_mem_stat_t*>(packet);
    if (sizeof(net_mem_stat_t) == length)
    {
        net_mem_stat_t* dst = new net_mem_stat_t;
        *dst = *src;
        v.push_back(dst);
        view->UpdateStat(dst);
    }
}

void MemProfController::Output(const String& msg)
{
    view->AppendText(msg.c_str());
}
