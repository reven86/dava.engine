#include "SpySearch.h"

#include <QMetaObject>
#include <QDebug>
#include <QMetaEnum>
#include <QEvent>

#include "SpySearchView.h"

#include "QtTools/SpyWidget/WidgetModel/WidgetListModel.h"
#include "QtTools/SpyWidget/SpyWidgetInfo.h"


SpySearch::SpySearch( QObject* parent )
    : QObject( parent )
    , view( new SpySearchView() )
    , resultModel( new WidgetListModel(this) )
{
    view->setAttribute( Qt::WA_DeleteOnClose );
    view->resultTree->setModel( resultModel );

    connect( view.data(), &QObject::destroyed, this, &QObject::deleteLater );
    connect( view.data(), &SpySearchView::triggered, this, &SpySearch::trigger );
    connect( view->resultTree, &QAbstractItemView::doubleClicked, this, &SpySearch::onWidgetSelect );
}

SpySearch::~SpySearch()
{
    delete view;
}

SpySearchView* SpySearch::GetView() const
{
    return view;
}

void SpySearch::show()
{
    view->show();
}

void SpySearch::trigger( const QPoint& pos )
{
    const bool isHidden = view->autoHide->isChecked();
    if ( isHidden )
    {
        view->setWindowOpacity( 0.0 );
        view->hide();
    }

    auto widget = QApplication::widgetAt( pos );

    if ( isHidden )
    {
        view->show();
        view->setWindowOpacity( 1.0 );
    }

    if ( isSelf( widget ) )
        return;

    resultModel->setWidgetList( QWidgetList() << widget );
}

void SpySearch::onWidgetSelect( const QModelIndex& index )
{
    auto w = resultModel->widgetFromIndex( index );
    showWidgetInfo( w );

    if ( view->autoClose->isChecked() )
        deleteLater();
}

bool SpySearch::isSelf( QWidget* w ) const
{
    auto p = w;

    while ( p != nullptr )
    {
        if ( p == view.data() )
            return true;

        p = p->parentWidget();
    }

    return false;
}

void SpySearch::showWidgetInfo( QWidget* w ) const
{
    if ( w == nullptr )
        return;

    //auto root = new QWidget();
    //root->setObjectName( "root" );
    //{
    //    auto child1 = new QWidget( root );
    //    child1->setObjectName( "child1" );
    //    {
    //        auto child11 = new QWidget( child1 );
    //        child11->setObjectName( "child1-1" );
    //    }
    //    auto child2 = new QWidget( root );
    //    child2->setObjectName( "child2" );
    //}
    //
    //{
    //    root->setAttribute( Qt::WA_DeleteOnClose );
    //    root->show();
    //}

    auto info = new SpyWidgetInfo();
    info->trackWidget( w );
    info->show();
}
