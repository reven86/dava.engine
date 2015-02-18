
#include <Utils/UTF8Utils.h>

#include "MemProfWidget.h"
#include "MemProfController.h"
#include "MemProfInfoModel.h"
using namespace DAVA;
using namespace DAVA::Net;

MemProfController::MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *_parentWidget, QObject* parent)
    : QObject(parent)
    , parentWidget(_parentWidget)
    , peer(peerDescr)
{
    ShowView();
    model = new MemProfInfoModel();
  
   
    view->SetModel(model);
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
    const  MemoryProfDataChunk* src = static_cast<const MemoryProfDataChunk*>(packet);
    if (sizeof(MemoryProfDataChunk) == length)
    {
        MemoryProfDataChunk* dst = new MemoryProfDataChunk();
        *dst = *src;
        v.push_back(dst);
        model->addMoreData(*dst);
        
        view->UpdateStat(dst);
    }
}

void MemProfController::Output(const String& msg)
{
    view->AppendText(msg.c_str());
}
