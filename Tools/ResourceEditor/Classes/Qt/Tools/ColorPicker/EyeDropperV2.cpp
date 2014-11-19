#include "EyeDropper.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QScreen>
#include <QTimer>


#include "../Helpers/MouseHelper.h"


EyeDropper::EyeDropper(QWidget* parent)
    : QObject(parent)
    , parentWidget(parent)
{
}

EyeDropper::~EyeDropper()
{
}

void EyeDropper::Exec()
{
}

void EyeDropper::OnDone()
{
}
