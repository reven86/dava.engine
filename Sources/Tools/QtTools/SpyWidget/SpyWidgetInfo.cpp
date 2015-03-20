#include "SpyWidgetInfo.h"

#include <QWidget>

#include "SpyWidget.h"


SpyWidgetInfo::SpyWidgetInfo( QObject* parent )
    : QObject( parent )
    , view( new SpyWidget() )
{
    view->setAttribute( Qt::WA_DeleteOnClose );

    connect( view.data(), &QObject::destroyed, this, &QObject::deleteLater );
}

SpyWidgetInfo::~SpyWidgetInfo()
{
    delete view;
}

void SpyWidgetInfo::trackWidget( QWidget* w )
{
    widget = w;
}

void SpyWidgetInfo::show()
{
    view->show();
}
