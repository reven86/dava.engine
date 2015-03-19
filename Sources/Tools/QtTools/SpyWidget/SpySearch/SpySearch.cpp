#include "SpySearch.h"

#include <QMetaObject>
#include <QDebug>
#include <QMetaEnum>
#include <QEvent>
#include <QApplication>

#include "SpySearchView.h"


SpySearch::SpySearch( QObject* parent )
    : QObject( parent )
    , view( new SpySearchView() )
{
    view->setAttribute( Qt::WA_DeleteOnClose );

    connect( view.data(), &QObject::destroyed, this, &QObject::deleteLater );
    connect( view.data(), &SpySearchView::triggered, this, &SpySearch::trigger );

    qApp->installEventFilter( this );
}

SpySearch::~SpySearch()
{
    delete view;
}

SpySearchView* SpySearch::GetView() const
{
    return view;
}

void SpySearch::Show()
{
    view->show();
}

void SpySearch::trigger( const QPoint& pos )
{
    const bool isVisible = view->isVisible();
    if ( isVisible )
    {
        view->setWindowOpacity( 0.0 );
        view->hide();
    }

    auto widget = QApplication::widgetAt( pos );

    if ( isVisible )
    {
        view->show();
        view->setWindowOpacity( 1.0 );
    }

    if ( widget == nullptr )
        return;
}
