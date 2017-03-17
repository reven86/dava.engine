#pragma once

#include <QWidget>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class MouseHelper;
class ColorPreview : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPreview(QWidget* parent);
    ~ColorPreview() override;

    void SetDragPreviewSize(const QSize& size);

public slots:
    void SetColorOld(const QColor& c);
    void SetColorNew(const QColor& c);

private slots:
    void OnMousePress(const QPoint& pos);

private:
    void paintEvent(QPaintEvent* e) override;

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
}
}
