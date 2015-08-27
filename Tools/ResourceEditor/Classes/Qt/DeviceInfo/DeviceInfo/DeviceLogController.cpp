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


#include <Utils/UTF8Utils.h>

#include "QtTools/ConsoleWidget/LogWidget.h"
#include "DeviceLogController.h"
#include "QtTools/ConsoleWidget/LogModel.h"

using namespace DAVA;
using namespace DAVA::Net;

DeviceLogController::DeviceLogController(const DAVA::Net::PeerDescription& peerDescr, QWidget *_parentWidget, QObject* parent)
    : QObject(parent)
    , parentWidget(_parentWidget)
    , peer(peerDescr)
{
    ShowView();
}

DeviceLogController::~DeviceLogController() {}

void DeviceLogController::ShowView()
{
    if (NULL == view)
    {
        const QString title = QString("%1 (%2 %3)")
            .arg(peer.GetName().c_str())
            .arg(peer.GetPlatformString().c_str())
            .arg(peer.GetVersion().c_str());

        view = new LogWidget(parentWidget);
        view->setWindowFlags(Qt::Window);
        view->setWindowTitle(title);

        connect(this, &QObject::destroyed, view, &QObject::deleteLater);
    }
    view->show();
    view->activateWindow();
    view->raise();
}

void DeviceLogController::ChannelOpen()
{
    Output("************* Connection open");
}

void DeviceLogController::ChannelClosed(const char8* message)
{
    String s("************ Connection closed: ");
    s += message;
    Output(s);
}

void DeviceLogController::PacketReceived(const void* packet, size_t length)
{
    String msg(static_cast<const char8*>(packet), length);
    Output(msg);
}

void DeviceLogController::Output(const String& msg)
{
    // Temporal workaround to extract log level from message
    QStringList list = QString(msg.c_str()).split(" ");
    Logger::eLogLevel ll = Logger::LEVEL_WARNING;
    // Current message format: <date> <time> <level> <text>
    if (list.size() > 3)
    {
        if (list[2] == "framwork")
            ll = Logger::LEVEL_FRAMEWORK;
        else if (list[2] == "debug")
            ll = Logger::LEVEL_DEBUG;
        else if (list[2] == "info")
            ll = Logger::LEVEL_INFO;
        else if (list[2] == "warning")
            ll = Logger::LEVEL_WARNING;
        else if (list[2] == "error")
            ll = Logger::LEVEL_ERROR;
    }
    view->AddMessage(ll, msg.c_str());
}
