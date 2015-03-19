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

bool SpySearch::eventFilter( QObject* obj, QEvent* e )
{
    QSet<int> filterOut;

    filterOut << QEvent::Timer;
    filterOut << QEvent::UpdateRequest;
    filterOut << QEvent::DynamicPropertyChange;
    filterOut << QEvent::StyleAnimationUpdate;
    filterOut << QEvent::Paint;
    filterOut << QEvent::WindowActivate;
    filterOut << QEvent::InputMethodQuery;
    filterOut << QEvent::FocusIn;
    filterOut << QEvent::FocusOut;
    filterOut << QEvent::ZeroTimerEvent;
    filterOut << QEvent::WindowDeactivate;
    filterOut << QEvent::NonClientAreaMouseMove;
    filterOut << QEvent::MouseMove;
    filterOut << QEvent::Enter;
    filterOut << QEvent::Leave;
    filterOut << QEvent::Move;
    filterOut << QEvent::Expose;
    filterOut << QEvent::HoverEnter;
    filterOut << QEvent::HoverLeave;
    filterOut << QEvent::UpdateLater;
    filterOut << QEvent::DeferredDelete;
    filterOut << QEvent::PolishRequest;
    filterOut << QEvent::LayoutRequest;
    filterOut << QEvent::Polish;
    filterOut << QEvent::ChildPolished;
    filterOut << QEvent::FontChange;
    filterOut << QEvent::Resize;
    filterOut << QEvent::Show;
    filterOut << QEvent::ShowToParent;
    filterOut << QEvent::FocusAboutToChange;
    filterOut << QEvent::Hide;
    filterOut << QEvent::HideToParent;
    filterOut << QEvent::NonClientAreaMouseButtonRelease;
    filterOut << QEvent::NonClientAreaMouseButtonPress;
    filterOut << QEvent::HoverMove;
    filterOut << QEvent::ToolTip;
    filterOut << QEvent::KeyRelease;

    if ( !filterOut.contains( e->type() ) )
    {
        const auto& mo = QEvent::staticMetaObject;
        auto me = mo.enumerator( mo.indexOfEnumerator( "Type" ) );
        qDebug() << e->type() << " -> " << me.valueToKey( e->type() );
    }

    return QObject::eventFilter( obj, e );
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

    qDebug() << "!!! Get ready!";

    auto w = new QWidget();
    w->setWindowTitle( "OTAKE!" );
    w->show();

    qDebug() << "!!! Done!";


    {
        auto list = QApplication::topLevelWidgets();
        for ( auto item : list )
        {
            qDebug() << item->metaObject()->className();
        }
        qDebug() << "-----";
    }

}
