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
        QScreen *screen = QApplication::screens()[i];
        const double scale = screen->devicePixelRatio();
        QRect rc = screens[i].rc;

        QPixmap pix = screen->grabWindow(0, rc.left(), rc.top(), rc.width(), rc.height());
        pix.setDevicePixelRatio(scale);
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

