#include "EyeDropper.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>

#include <QTimer>


#include "../Helpers/MouseHelper.h"
#include "DropperShade.h"
#include "Platform/DpiHelper.h"


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
    InitShades();
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
        const double scale = DAVA::DPIHelper::GetDpiScaleFactor(i);
        
        QRect rc = screens[i].rc;
        const bool scaled = scale > 1.0;
        if (scaled)
        {
            rc.setWidth( int(scale * screens[i].rc.width()) );
            rc.setHeight( int(scale * screens[i].rc.height()) );
        }

        int l, t, r, b;
        FindExtraOfs(screens, i, l, t, r, b);
        rc.adjust(l, t, r, b);

        QPixmap pix;
        if (scaled)
        {
            pix = QPixmap::grabWindow(s->winId(), rc.left(), rc.top(), rc.width(), rc.height() ).scaled(screens[i].rc.size());
        }
        else
        {
            pix = QPixmap::grabWindow(s->winId(), rc.left(), rc.top(), rc.width(), rc.height());
        }
        const QImage img = pix.toImage();
        
        DropperShade *shade = new DropperShade( img, screens[i].rc );
        shades[i] = shade;
        shade->show();

        connect( shade, SIGNAL( canceled() ), SIGNAL( canceled() ) );
        connect( shade, SIGNAL( picked(const QColor&) ), SIGNAL( picked(const QColor&) ) );
        connect( shade, SIGNAL( moved(const QColor&) ), SIGNAL( moved(const QColor&) ) );

        connect( shade, SIGNAL( canceled() ), SLOT( OnDone() ) );
        connect( shade, SIGNAL( picked(const QColor&) ), SLOT( OnDone() ) );
    }

    for (int i = 0; i < shades.size(); i++ )
    {
        for (int j = 0; j < shades.size(); j++)
        {
            if (i != j)
            {
                connect(shades[i], SIGNAL( zoonFactorChanged(int) ), shades[j], SLOT( SetZoomFactor(int) ));
            }
        }
    }

}

void EyeDropper::FindExtraOfs(const ScreenArray& screens, int id, int& l, int& t, int& r, int& b)
{
    l = 0;
    t = 0;
    r = 0;
    b = 0;

#ifdef Q_OS_MAC
    const ScreenData& scr = screens[id];
    const QRect& rc = scr.rc.adjusted(-1, -1, 1, 1);

    for (int i = 0; i < screens.size(); i++)
    {
        if (i == id)
            continue;
        const QRect& sRc = screens[i].rc;

        if ( rc.left() >= sRc.left() && rc.left() < sRc.right() )
            l = -1;
        if ( rc.right() >= sRc.left() && rc.right() < sRc.right() )
            r = 1;
        if ( rc.top() >= sRc.top() && rc.top() < sRc.bottom() )
            t = -1;
        if ( rc.bottom() >= sRc.top() && rc.bottom() < sRc.bottom() )
            b = 1;
    }
#endif
}
