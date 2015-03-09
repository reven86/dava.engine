#ifndef COLORCELL_H
#define COLORCELL_H

#include <QWidget>
#include <QPointer>


class MouseHelper;

class ColorCell
    : public QWidget
{
    Q_OBJECT

public:
    typedef QVector<QColor> Colors;

signals:
    void clicked(const QColor& c);

public:
    explicit ColorCell(QWidget *parent);
    ~ColorCell();

    const QColor& GetColor() const;
    void SetColor(const QColor& color);

private slots:
    void OnMouseEnter();
    void OnMouseLeave();
    void OnMouseClick();

private:
    void paintEvent(QPaintEvent* e);

    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

    QColor color;
    bool isHovered;
    QBrush bgBrush;
    QPointer<MouseHelper> mouse;
};


#endif // COLORCELL_H
