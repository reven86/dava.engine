#ifndef COLORPREVIEW_H
#define COLORPREVIEW_H

#include <QWidget>
#include <QPointer>


class MouseHelper;

class ColorPreview
    : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPreview(QWidget* parent);
    ~ColorPreview();

    void SetDragPreviewSize(const QSize& size);

public slots:
    void SetColorOld(const QColor& c);
    void SetColorNew(const QColor& c);

private slots:
    void OnMousePress(const QPoint& pos);
    void OnMouseRelease(const QPoint& pos);

private:
    void paintEvent(QPaintEvent* e);

    QColor GetColorAt(const QPoint& pos) const;
    QRect OldColorSRect() const;
    QRect OldColorRect() const;
    QRect NewColorSRect() const;
    QRect NewColorRect() const;

    QColor cOld;
    QColor cNew;
    QBrush bgBrush;
    QSize dragPreviewSize;
    QPointer<MouseHelper> mouse;
};

#endif // COLORPREVIEW_H
