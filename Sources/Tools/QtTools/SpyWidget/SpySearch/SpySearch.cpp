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
