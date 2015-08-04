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


#include "AbstractWidgetModel.h"

#include <QMetaObject>


AbstractWidgetModel::AbstractWidgetModel( QObject* parent )
    : QAbstractItemModel( parent )
{
}

AbstractWidgetModel::~AbstractWidgetModel()
{
}

int AbstractWidgetModel::columnCount( const QModelIndex& parent ) const
{
    return COLUMN_COUNT;
}

QVariant AbstractWidgetModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    return QVariant();
}

Qt::ItemFlags AbstractWidgetModel::flags( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant AbstractWidgetModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    auto w = widgetFromIndex( index );
    if ( w == nullptr )
        return QVariant();

    switch ( role )
    {
    case Qt::DisplayRole:
        return textDataForColumn( w, index.column() );
    default:
        break;
    }

    return QVariant();
}

bool AbstractWidgetModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    Q_UNUSED( index );
    Q_UNUSED( value );
    Q_UNUSED( role );

    return false;
}

QVariant AbstractWidgetModel::textDataForColumn( QWidget *w, int column ) const
{
    auto meta = w->metaObject();

    switch ( column )
    {
    case TITLE:
        return QString( "%1" ).arg( meta->className() );
    case CLASSNAME:
        return QString( ".%1" ).arg( meta->className() );
    case OBJECTNAME:
        return QString( "#%1" ).arg( w->objectName() );
    default:
        break;
    }

    return QVariant();
}
