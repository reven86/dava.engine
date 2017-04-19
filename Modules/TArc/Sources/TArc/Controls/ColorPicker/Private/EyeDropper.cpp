#include "TArc/Controls/ColorPicker/Private/EyeDropper.h"
#include "TArc/Controls/ColorPicker/Private/DropperShade.h"
#include "TArc/Controls/ColorPicker/Private/MouseHelper.h"

#include <Platform/DPIHelper.h>

#include <QApplication>
#include <QCursor>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QScreen>
#include <QTimer>

namespace DAVA
{
namespace TArc
{
EyeDropper::EyeDropper(QWidget* parent)
    : QObject(parent)
    , parentWidget(parent)
{
}

void EyeDropper::Exec()
{
    InitShades();
}

void EyeDropper::OnDone()
{
    for (int i = 0; i < shades.size(); i++)
    {
        DropperShade* shade = shades[i];
        if (shade)
        {
            shades[i]->deleteLater();
        }
    }
}

void EyeDropper::InitShades()
{
    QList<QScreen*> screens = QApplication::screens();
    int screensCount = screens.size();

    shades.resize(screensCount);
    for (int i = 0; i < screensCount; i++)
    {
        QScreen* screen = screens[i];
        const double scale = screen->devicePixelRatio();

        QRect screenRect = screen->geometry();
        QPixmap pix = screen->grabWindow(0, screenRect.left(), screenRect.top(), screenRect.width() / scale, screenRect.height() / scale);

        const QImage img = pix.toImage();

        DropperShade* shade = new DropperShade(img, screenRect);
        shades[i] = shade;
        shade->show();

        connect(shade, SIGNAL(canceled()), SIGNAL(canceled()));
        connect(shade, SIGNAL(picked(const QColor&)), SIGNAL(picked(const QColor&)));
        connect(shade, SIGNAL(moved(const QColor&)), SIGNAL(moved(const QColor&)));

        connect(shade, SIGNAL(canceled()), SLOT(OnDone()));
        connect(shade, SIGNAL(picked(const QColor&)), SLOT(OnDone()));
    }

    for (int i = 0; i < shades.size(); i++)
    {
        for (int j = 0; j < shades.size(); j++)
        {
            if (i != j)
            {
                connect(shades[i], SIGNAL(zoonFactorChanged(int)), shades[j], SLOT(SetZoomFactor(int)));
            }
        }
    }
}
}
}
