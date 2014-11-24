#include "DropperLens.h"

#include <QDebug>
#include <QPainter>
#include <QBrush>

#include "DropperShade.h"


namespace
{
    
    const unsigned int nRadius = 50;
    const QRect cLensGeometry(0, 0, nRadius * 2 + 1, nRadius * 2 + 1);

}


DropperLens::DropperLens(DropperShade* _shade)
    : QWidget(_shade)
    , shade(_shade)
    , zoomFactor(0)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::BlankCursor);
    setFixedSize(cLensGeometry.size());

    preview = QImage(lensGeometry().size(), QImage::Format_ARGB32);
    preview.fill(Qt::transparent);
}

DropperLens::~DropperLens()
{
}

QRect DropperLens::lensGeometry() const
{
    return cLensGeometry;
}

int DropperLens::ZoomFactor() const
{
    return zoomFactor;
}

void DropperLens::SetZoomFactor(int zoom)
{
    if ( zoom == zoomFactor )
        return ;

    zoomFactor = zoom;
    update();
}

void DropperLens::updatePreview()
{
    const QImage& src = shade->screenImage();
    const QPoint& pos = shade->CursorPos();
    const int scale = zoomFactor * 2 + 1;
    const int nDstPixels = lensGeometry().width();
    const int nSrcPixels = nDstPixels / scale;
    const QRect srcRect(pos.x() - nSrcPixels / 2, pos.y() - nSrcPixels / 2, nSrcPixels, nSrcPixels);
    const QRect dstRect(0, 0, nDstPixels, nDstPixels);

    const int srcWidth = src.width();
    const QRgb* const from = reinterpret_cast<const QRgb *>(src.constScanLine(0));
    QRgb *to = reinterpret_cast<QRgb *>(preview.scanLine(0));

    for ( int sx = 0; sx < srcRect.width(); sx++ )
    {
        const int realSx = sx + srcRect.x();
        if ( realSx < 0 )
            continue;

        for ( int sy = 0; sy < srcRect.height(); sy++ )
        {
            const int realSy = sy + srcRect.y();
            if ( realSy < 0 )
                continue;

            const QRgb pixel = *(from + realSy * srcWidth + realSx);

            for ( int dx = 0; dx < scale; dx++ )
            {
                for ( int dy = 0; dy < scale; dy++ )
                {
                    const int realDx = sx * scale + dx;
                    const int realDy = sy * scale + dy;
                    *(to + realDy * dstRect.width() + realDx) = pixel;
                }
            }
        }
    }
}

void DropperLens::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    updatePreview();
    const QRect& lensRect = lensGeometry();

    QPainter p(this);
    QRegion clip(lensRect, QRegion::Ellipse);

    // Background
    //p.fillRect(0, 0, width(), height(), Qt::red);

    // Preview
    p.setClipRegion(clip);
    p.drawImage(lensRect, preview);

    // Foreground
    p.setClipping(false);
    p.setRenderHints( QPainter::Antialiasing );
    p.setPen( QPen( Qt::darkGray, 4.0 ) );
    
    const int r = lensRect.width() / 2;
    p.drawEllipse(lensRect.center(), r, r);

    const int dim = zoomFactor * 2 + 1;
    const QPoint& mid = lensRect.center();
    QRect frame( mid.x() - zoomFactor - 1, mid.y() - zoomFactor -1, dim + 1, dim + 1 );

    p.setRenderHints(QPainter::Antialiasing, false);
    p.setPen(QPen(Qt::white, 1.0));
    p.drawRect( frame );

    frame.adjust(-1, -1, 1, 1);
    p.setPen(QPen(Qt::black, 1.0));
    p.drawRect( frame );

    //p.setPen( Qt::red );
    //p.drawLine( lensRect.center().x(), lensRect.top(), lensRect.center().x(), lensRect.bottom() );
    //p.drawLine( lensRect.left(), lensRect.center().y(), lensRect.right(), lensRect.center().y() );
}
