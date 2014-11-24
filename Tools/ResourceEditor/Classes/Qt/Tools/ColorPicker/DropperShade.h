#ifndef __DROPPPER_SHADE_
#define __DROPPPER_SHADE_


#include <QWidget>
#include <QPointer>


class MouseHelper;
class DropperLens;

class DropperShade
    : public QWidget
{
    Q_OBJECT

signals:
    void canceled();
    void picked(const QColor& color);

public:
    DropperShade();
    ~DropperShade();

    const QImage& screenImage() const;
    QPoint CursorPos() const;

private slots:
    void OnMouseMove(const QPoint& pos);
    void OnClicked(const QPoint& pos);
    void OnMouseWheel(int delta);

private:
    void paintEvent( QPaintEvent *e ) override;

    QPixmap screen;
    QImage screenData;
    QPointer<MouseHelper> mouse;
    QPointer<DropperLens> lens;

private:
    static QPixmap screenShot();
};


#endif
