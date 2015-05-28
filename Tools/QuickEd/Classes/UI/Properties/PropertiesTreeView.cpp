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


#include "PropertiesTreeView.h"
#include <QPainter>
#include <QHeaderView>

PropertiesTreeView::PropertiesTreeView( QWidget *parent /*= NULL*/ )
{

}

PropertiesTreeView::~PropertiesTreeView()
{

}

void PropertiesTreeView::drawRow(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    QColor gridColor = option.palette.color(QPalette::Normal, QPalette::Window);

    // draw horizontal bottom line
    painter->setPen(gridColor);
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

    // adjust rect, so that grid line wont be overdrawn
    QStyleOptionViewItemV4 opt = option;
    opt.rect.adjust(0, 0, 0, -1);

    // draw row
    QTreeView::drawRow(painter, opt, index);

    // draw vertical line
    if(!(option.state & QStyle::State_Selected))
    {
        QHeaderView *hdr = header();
        if(NULL != hdr && hdr->count() > 1)
        {
            int sz = hdr->sectionSize(0);

            QPoint p1 = option.rect.topLeft();
            QPoint p2 = option.rect.bottomLeft();

            p1.setX(p1.x() + sz - 1);
            p2.setX(p2.x() + sz - 1);

            painter->setPen(gridColor);
            painter->drawLine(p1, p2);
        }
    }
}
