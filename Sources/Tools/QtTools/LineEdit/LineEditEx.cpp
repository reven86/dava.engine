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


#include "LineEditEx.h"

#include <QAction>
#include <QActionEvent>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCursor>

#include "LineEditStyle.h"


LineEditEx::LineEditEx(QWidget* parent)
    : QLineEdit(parent)
    , timer(new QTimer(this))
    , useDelayedUpdate(true)
    , buttonsWidth(0)
{
    timer->setSingleShot(true);
    SetAcceptInterval(500);
    SetUseDelayedUpdate(useDelayedUpdate);

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(QMargins());
    l->setSpacing(1);
    l->addStretch();
    setLayout(l);

    LineEditStyle *proxyStyle = new LineEditStyle( style() );
    setStyle(proxyStyle);
}

LineEditEx::~LineEditEx()
{
}

void LineEditEx::SetAcceptInterval(int msec)
{
    const bool wasActive = timer->isActive();
    timer->stop();
    timer->setInterval(msec);
    if (wasActive)
        timer->start();
}

bool LineEditEx::IsDelayedUpdateUsed() const
{
    return useDelayedUpdate;
}

void LineEditEx::SetUseDelayedUpdate(bool use)
{
    useDelayedUpdate = use;

    SetupConnections(false, false); // Reset connections first
    SetupConnections(useDelayedUpdate, !useDelayedUpdate);
}

QAbstractButton* LineEditEx::CreateButton(const QAction * action)
{
    QPushButton *btn = new QPushButton();
    SyncButtonWithAction( action, btn );

    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn->setFixedSize(ButtonSizeHint(action));
    btn->setCursor(Qt::ArrowCursor);

    return btn;
}

QSize LineEditEx::ButtonSizeHint(const QAction* action) const
{
    Q_UNUSED(action);
    return QSize(20, 20);
}

void LineEditEx::SyncButtonWithAction(const QAction* action, QAbstractButton* button)
{
    button->setText(action->text());
    button->setToolTip(action->toolTip());
    button->setIcon(action->icon());
}

int LineEditEx::ButtonsWidth() const
{
    return buttonsWidth;
}

void LineEditEx::AddCustomWidget(QWidget* w)
{
    if (widgets.contains(w))
        return ;

    widgets.insert(w);
    layout()->addWidget(w);
    UpdatePadding();
}

void LineEditEx::RemoveCustomWidget(QWidget* w)
{
    if (!widgets.contains(w))
        return ;

    widgets.remove(w);
    layout()->removeWidget(w);
    UpdatePadding();
}

void LineEditEx::OnTextEdit()
{
    timer->start();
}

void LineEditEx::OnAcceptEdit()
{
    timer->stop();
    emit textUpdated(text());
}

void LineEditEx::UpdatePadding()
{
    buttonsWidth = 0;
    for ( auto it = widgets.constBegin(); it != widgets.constEnd(); ++it )
    {
        buttonsWidth += (*it)->width();
    }
    if ( buttons.size() > 0 )
    {
        buttonsWidth += layout()->spacing() * ( buttons.size() - 1 );
    }
    updateGeometry();
    update();
}

void LineEditEx::OnActionChanged()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if ( action == NULL)
        return;

    auto it = buttons.find(action);
    if ( it != buttons.end() )
    {
        SyncButtonWithAction( it.key(), it.value() );
    }
}

void LineEditEx::SetupConnections(bool delayed, bool instant)
{
    if (delayed)
    {
        connect(timer, SIGNAL( timeout() ), this, SLOT( OnAcceptEdit() ));
        connect(this, SIGNAL( editingFinished() ), this, SLOT( OnAcceptEdit() ));
        connect(this, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnTextEdit() ));
    }
    else
    {
        disconnect(timer, SIGNAL( timeout() ), this, SLOT( OnAcceptEdit() ));
        disconnect(this, SIGNAL( editingFinished() ), this, SLOT( OnAcceptEdit() ));
        disconnect(this, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnTextEdit() ));
    }

    if (instant)
    {
        connect(this, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textUpdated( const QString& ) ));
    }
    else
    {
        disconnect(this, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textUpdated( const QString& ) ));
    }
}

void LineEditEx::AddActionHandler(QAction* action)
{
    QAbstractButton *btn = CreateButton(action);
    buttons[action] = btn;
    AddCustomWidget(btn);

    connect( action, SIGNAL( changed() ), SLOT( OnActionChanged() ) );
    connect( btn, SIGNAL( clicked() ), action, SLOT( trigger() ) );
}

void LineEditEx::RemoveActionHandler(QAction* action)
{
    auto it = buttons.find(action);
    if ( it != buttons.end() )
    {
        RemoveCustomWidget(it.value());
        it.value()->deleteLater();
        buttons.erase(it);
    }
}

void LineEditEx::actionEvent(QActionEvent* event)
{
    switch ( event->type() )
    {
    case QEvent::ActionAdded:
        AddActionHandler(event->action());
        break;

    case QEvent::ActionRemoved:
        RemoveActionHandler(event->action());
        break;

    default:
        break;
    }
}
