#ifndef DROPPERLENS_H
#define DROPPERLENS_H


#include <QWidget>
#include <QPointer>
#include <QPixmap>


class DropperShade;


class DropperLens
    : public QWidget
{
    Q_OBJECT

public:
    explicit DropperLens(DropperShade* shade);
    ~DropperLens();

    QRect lensGeometry() const;
    int ZoomFactor() const;
    void SetZoomFactor( int zoom );

private slots:
    void updatePreview();

private:
    void paintEvent(QPaintEvent *e) override;

    QImage preview;
    QPointer<DropperShade> shade;
    int zoomFactor;
};


#endif // DROPPERLENS_H
