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
#include "DropperLens.h"


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
    lens = new DropperLens();
    InitShades();
    lens->show();
    lens->moveTo(QCursor::pos());
}

void EyeDropper::OnDone()
{
    for ( int i = 0; i < shades.size(); i++ )
    {
        DropperShade *shade = shades[i];
        if (shade)
        {
            shades[i]->deleteLater();
        }
    }

    lens->deleteLater();
}

void EyeDropper::InitShades()
{
    QDesktopWidget* desktop = QApplication::desktop();
    const int n = desktop->screenCount();

    ScreenArray screens;
    screens.reserve(n);
    for (int i = 0; i < n; i++)
    {
        const QRect& screenRect = desktop->screenGeometry(i);
        ScreenData data = { i, screenRect };
        screens.push_back(data);
    }

    shades.resize(n);
    for (int i = 0; i < n; i++)
    {
        QWidget *s = desktop->screen(i);
        QScreen *screen = QApplication::screens()[i];
        const double scale = screen->devicePixelRatio();
        
        QRect rc = screens[i].rc;
        const bool scaled = scale > 1.0;

        QPixmap pix;
        //if (scaled)
        //{
        //    pix = screen->grabWindow(0, rc.left(), rc.top(), rc.width(), rc.height() ).scaled(screens[i].rc.size());
        //}
        //else
        //{
        //    pix = screen->grabWindow(0, rc.left(), rc.top(), rc.width(), rc.height());
        //}
        pix = screen->grabWindow(0, rc.left(), rc.top(), rc.width(), rc.height());
        pix.setDevicePixelRatio(scale);
        
        DropperShade *shade = new DropperShade( pix, screens[i].rc, lens );
        shades[i] = shade;
        shade->show();

        connect( shade, SIGNAL( canceled() ), SIGNAL( canceled() ) );
        connect( shade, SIGNAL( picked(const QColor&) ), SIGNAL( picked(const QColor&) ) );
        connect( shade, SIGNAL( changed(const QColor&) ), SIGNAL( moved(const QColor&) ) );

        connect( shade, SIGNAL( canceled() ), SLOT( OnDone() ) );
        connect( shade, SIGNAL( picked(const QColor&) ), SLOT( OnDone() ) );
    }

    for (int i = 0; i < shades.size(); i++ )
    {
        connect(shades[i], &DropperShade::moved, lens, &DropperLens::moveTo);
    }

}
