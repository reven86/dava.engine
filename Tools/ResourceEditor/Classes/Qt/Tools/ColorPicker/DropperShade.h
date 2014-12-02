#ifndef __DROPPPER_SHADE_
#define __DROPPPER_SHADE_

#include <QWidget>
#include <QPointer>
#include <QImage>


class MouseHelper;


class DropperShade
    : public QWidget
{
    Q_OBJECT

signals:
    void canceled();
    void picked(const QColor& color);
    void moved(const QColor& color);
    void zoonFactorChanged(int zoom);

public:
    DropperShade( const QImage& src, const QRect& rect );
    ~DropperShade();

public slots:
    void SetZoomFactor(int zoom);

private slots:
    void OnMouseMove(const QPoint& pos);
    void OnClicked(const QPoint& pos);
    void OnMouseWheel(int delta);
    void OnMouseEnter();
    void OnMouseLeave();

private:
    void paintEvent(QPaintEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void DrawCursor(const QPoint& pos, QPainter* p);
    QColor GetPixel(const QPoint& pos) const;

    const QImage cache;
    QSize cursorSize;
    QPoint cursorPos;
    int zoomFactor;
    QPointer<MouseHelper> mouse;
    bool drawCursor;
};



#endif
