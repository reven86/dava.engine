#ifndef __DROPPPER_SHADE_
#define __DROPPPER_SHADE_


#include <QWidget>
#include <QPointer>


class MouseHelper;

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

private slots:
    void OnMouseMove(const QPoint& pos);
    void OnClicked(const QPoint& pos);
    void OnMouseWheel(int delta);

private:
    void paintEvent( QPaintEvent *e ) override;

    QPixmap m_screen;
    QPointer<MouseHelper> mouse;

private:
    static QPixmap screenShot();
};


#endif
