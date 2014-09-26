#include "LineEditEx.h"

LineEditEx::LineEditEx(QWidget* parent)
    : QLineEdit(parent)
      , timer(new QTimer(this))
{
    timer->setSingleShot(true);
    SetAcceptInterval(500);

    connect(timer, SIGNAL( timeout() ), SLOT( OnAcceptEdit() ));
    connect(this, SIGNAL( editingFinished() ), SLOT( OnAcceptEdit() ));
    connect(this, SIGNAL( textChanged( const QString& ) ), SLOT( OnTextEdit() ));
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

void LineEditEx::OnTextEdit()
{
    timer->start();
}

void LineEditEx::OnAcceptEdit()
{
    timer->stop();
    emit textUpdated(text());
}