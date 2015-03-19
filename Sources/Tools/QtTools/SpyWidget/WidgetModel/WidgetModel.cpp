#include "WidgetModel.h"

#include <QApplication>



WidgetModel::WidgetModel( QObject* parent )
    : QAbstractItemModel( parent )
    , isGlobal(false)
{
    if ( parent == nullptr )
        parent = QApplication::instance();
    if ( qobject_cast<QApplication *>(parent) != nullptr )
        isGlobal = true;

    parent->installEventFilter(this);
}

WidgetModel::~WidgetModel()
{
}

bool WidgetModel::eventFilter( QObject* obj, QEvent* e )
{
    if ( isGlobal )
        return applicationEventFilterInternal( obj, e );

    return widgetEventFilterInternal( obj, e );
}

bool WidgetModel::applicationEventFilterInternal( QObject* obj, QEvent* e )
{
    return QAbstractItemModel::eventFilter( obj, e );
}

bool WidgetModel::widgetEventFilterInternal( QObject* obj, QEvent* e )
{
    return QAbstractItemModel::eventFilter( obj, e );
}
