#include "DeviceInfoController.h"


#include <QDebug>

#include "DeviceInfoWidget.h"


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
    delete view;
    view = new DeviceInfoWidget( parentWidget );
    view->setWindowFlags( Qt::Window );
    view->setAttribute( Qt::WA_DeleteOnClose );

    connect( this, &QObject::destroyed, view, &QObject::deleteLater );
    connect( view, &QObject::destroyed, this, &QObject::deleteLater );

    view->show();
}

void DeviceInfoController::DebugOutput()
{
    view->AppendText( "Preved\nMedved" );
}