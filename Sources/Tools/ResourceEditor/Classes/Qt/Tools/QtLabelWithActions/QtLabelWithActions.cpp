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


#include "QtLabelWithActions.h"

#include "DAVAEngine.h"

#include <QMenu>
#include <QAction>
#include <QMouseEvent>

QtLabelWithActions::QtLabelWithActions(QWidget *parent /*= 0*/)
	: QLabel(parent)
	, menu(NULL)
{
	SetTextColor(Qt::white);
}


QtLabelWithActions::~QtLabelWithActions()
{
}

void QtLabelWithActions::mousePressEvent( QMouseEvent * event )
{
	if(menu)
	{
		menu->exec(mapToGlobal(geometry().bottomLeft()));
	}
}

void QtLabelWithActions::enterEvent(QEvent *event)
{
	SetTextColor(Qt::yellow);
}

void QtLabelWithActions::leaveEvent(QEvent *event)
{
	SetTextColor(Qt::white);
}


void QtLabelWithActions::setMenu(QMenu *_menu)
{
	if(menu)
	{
		QObject::disconnect(this, SLOT(MenuTriggered(QAction *)));
	}

	menu = _menu;

	if(menu)
	{
		QObject::connect(menu, SIGNAL(triggered(QAction *)) , this, SLOT(MenuTriggered(QAction *)));
	}
}

void QtLabelWithActions::setDefaultAction(QAction *action)
{
	if(action)
	{
		setText(DAVA::Format("[ %s ]", action->text().toStdString().c_str()).c_str());
	}
	else
	{
		setText("");
	}
}

void QtLabelWithActions::MenuTriggered( QAction *action )
{
	setDefaultAction(action);
}

void QtLabelWithActions::SetTextColor( const QColor &color )
{
	QPalette pal = palette();
	pal.setColor(QPalette::WindowText, color);
	setPalette(pal);
}
