#ifndef __DROPPPER_LENS_
#define __DROPPPER_LENS_

#include <QWidget>
#include <QPointer>


class QLabel;


class DropperLens
    : public QWidget
{
    Q_OBJECT

public:
    DropperLens();
    ~DropperLens();

    int ZoomFactor() const;
    QPixmap& preview();

public slots:
    void changeZoom(int delta);
    void moveTo(const QPoint& pos);

private:
    void paintEvent(QPaintEvent* e);

    int zoomFactor;
    QPixmap pixmap;
    QPointer<QLabel> label;
};


#endif // __DROPPPER_LENS_
