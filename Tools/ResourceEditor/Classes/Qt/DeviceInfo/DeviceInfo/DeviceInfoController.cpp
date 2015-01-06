#include "DeviceInfoController.h"


#include <QDebug>

#include "DeviceInfoWidget.h"

using namespace DAVA;
using namespace DAVA::Net;

DeviceInfoController::DeviceInfoController( QWidget *_parentWidget, QObject* parent )
    : QObject( parent )
    , parentWidget( _parentWidget )
{
    InitView();
    DebugOutput();
}

DeviceInfoController::~DeviceInfoController()
{
}

void DeviceInfoController::InitView()
{
    if (NULL == view)
    {
        view = new DeviceInfoWidget( parentWidget );
        view->setWindowFlags( Qt::Window );
        //view->setAttribute( Qt::WA_DeleteOnClose );

        connect( this, &QObject::destroyed, view, &QObject::deleteLater );
        //connect( view, &QObject::destroyed, this, &QObject::deleteLater );
    }
    view->show();
}

void DeviceInfoController::DebugOutput()
{
    //view->AppendText( "Preved\nMedved" );
}

void DeviceInfoController::ChannelOpen()
{
    view->AppendText("************* Channel open");
}

void DeviceInfoController::ChannelClosed()
{
    view->AppendText("************ Channel closed");
}

void DeviceInfoController::PacketReceived(const void* packet, size_t length)
{
    String s(static_cast<const char8*>(packet), length);
    view->AppendText(s.c_str());
}
