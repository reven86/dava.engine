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
#include "DropperShade.h"


EyeDropper::EyeDropper(QObject* parent)
    : QObject(parent)
{
}

EyeDropper::~EyeDropper()
{
}

void EyeDropper::Exec()
{
    DropperShade *shade = new DropperShade();
    connect( shade, &DropperShade::picked, this, &EyeDropper::picked );
    connect( shade, &DropperShade::canceled, this, &EyeDropper::canceled );
    shade->show();
}
