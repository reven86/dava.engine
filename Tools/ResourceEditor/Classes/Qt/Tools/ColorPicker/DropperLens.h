#ifndef __DROPPPER_LENS_
#define __DROPPPER_LENS_

#include <QWidget>
#include <QPointer>



class DropperLens
    : public QWidget
{
    Q_OBJECT

public:
    DropperLens();
    ~DropperLens();

    int ZoomFactor() const;

public slots:
    void changeZoom(int delta);
    void updatePreview(const QPixmap& preview);
    void moveTo(const QPoint& pos);

private:
    void paintEvent(QPaintEvent* e);

    int zoomFactor;
    QPixmap pixmap;
};


#endif // __DROPPPER_LENS_
