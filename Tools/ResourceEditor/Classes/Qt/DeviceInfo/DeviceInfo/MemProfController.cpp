#include <Utils/UTF8Utils.h>

#include "Base/FunctionTraits.h"

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
    netClient.SetCallbacks(MakeFunction(this, &MemProfController::ChannelOpen),
                           MakeFunction(this, &MemProfController::ChannelClosed),
                           MakeFunction(this, &MemProfController::CurrentStat));
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

void MemProfController::ChannelOpen(DAVA::MMStatConfig* config)
{
    view->ChangeStatus("connected", nullptr);
    view->ClearStat();

    Logger::Debug("MemProfController::ChannelOpen");
    if (config)
    {
        Logger::Debug("   maxTags=%u, ntags=%u", config->maxTagCount, config->tagCount);
        for (uint32 i = 0;i < config->tagCount;++i)
            Logger::Debug("      %d, %s", i, config->names[i].name);
        Logger::Debug("   maxPools=%u, npools=%u", config->maxAllocPoolCount, config->allocPoolCount);
        for (uint32 i = 0;i < config->allocPoolCount;++i)
            Logger::Debug("      %d, %s", i, config->names[i + config->tagCount].name);
    }
    view->SetStatConfig(config);
    
}

void MemProfController::ChannelClosed(char8* message)
{
    view->ChangeStatus("disconnected", message);
}

void MemProfController::CurrentStat(DAVA::MMStat* stat)
{
    view->UpdateStat(stat);
}

void MemProfController::Output(const String& msg)
{
    view->AppendText(msg.c_str());
}
