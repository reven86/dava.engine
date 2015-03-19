#ifndef QTTOOLS_SPYDRAGWIDGET_H
#define QTTOOLS_SPYDRAGWIDGET_H


#include <QLabel>
#include <QPixmap>
#include <QCursor>


class SpyDragWidget
    : public QLabel
{
private:
    Q_OBJECT

signals:
    void mousePressed();
    void mouseReleased( const QPoint& globalPos );

public:
    explicit SpyDragWidget( QWidget *parent = nullptr );
    ~SpyDragWidget();

private:
    void mousePressEvent( QMouseEvent * e ) override;
    void mouseReleaseEvent( QMouseEvent * e ) override;

    void onMousePress();
    void onMouseRelease();

    QPixmap pix;
    QCursor cur;
};


#endif // QTTOOLS_SPYDRAGWIDGET_H
