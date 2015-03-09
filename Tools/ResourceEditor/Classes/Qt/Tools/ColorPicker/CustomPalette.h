#ifndef CUSTOMPALETTE_H
#define CUSTOMPALETTE_H

#include <QWidget>
#include <QPointer>


class ColorCell;
class MouseHelper;

class CustomPalette
    : public QWidget
{
    Q_OBJECT

public:
    typedef QVector<QColor> Colors;

signals:
    void selected(const QColor& c);

public:
    explicit CustomPalette(QWidget *parent = NULL);
    ~CustomPalette();

    void SetColors(const Colors& colors);
    void SetCellSize(const QSize& size);
    void SetCellCount(int w, int h);
    Colors GetColors() const;

private:
    void resizeEvent(QResizeEvent* e);

    void CreateControls();
    void AdjustControls();
    int Count() const;

    int nRows;
    int nColumns;
    Colors colors;
    QList< QPointer<ColorCell> > controls;
    QSize cellSize;
};



#endif // PALETTEHSV_H
