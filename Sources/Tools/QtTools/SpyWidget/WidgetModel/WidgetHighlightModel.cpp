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


#include "WidgetHighlightModel.h"

#include "AbstractWidgetModel.h"


WidgetHighlightModel::WidgetHighlightModel( QObject* parent )
    : QIdentityProxyModel( parent )
{
}

WidgetHighlightModel::~WidgetHighlightModel()
{
}

QVariant WidgetHighlightModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    auto ret = QIdentityProxyModel::data( index, role );

    auto model = qobject_cast<AbstractWidgetModel *>( sourceModel() );
    if ( model == nullptr )
        return ret;

    auto realIndex = mapToSource( index );
    auto w = model->widgetFromIndex( realIndex );

    switch ( role )
    {
    case Qt::BackgroundRole:
        if ( widgets.contains( w ) )
        {
            ret = QColor( 0, 255, 0, 150 );
        }
        break;
    default:
        break;
    }

    return ret;
}

void WidgetHighlightModel::setWidgetList( const QSet< QWidget * >& widgetsToHighlight )
{
    for ( auto w : widgets )
    {
        disconnect( w, nullptr, this, nullptr );
    }

    widgets = widgetsToHighlight;
    for ( auto w : widgets )
    {
        connect( w, &QObject::destroyed, this, &WidgetHighlightModel::onWidgetDestroyed );
    }
    invalidate();
}

void WidgetHighlightModel::onWidgetDestroyed()
{
    auto w = qobject_cast<QWidget *>( sender() );
    widgets.remove( w );
    invalidate();
}

void WidgetHighlightModel::invalidate()
{
    static const auto roles = QVector<int>() << Qt::DisplayRole << Qt::TextColorRole << Qt::BackgroundRole;
    emit dataChanged( QModelIndex(), QModelIndex(), roles );
}
