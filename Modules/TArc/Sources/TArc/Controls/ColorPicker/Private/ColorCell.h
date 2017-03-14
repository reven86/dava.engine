#pragma once

#include <QWidget>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class MouseHelper;
class ColorCell : public QWidget
{
    Q_OBJECT

public:
    typedef QVector<QColor> Colors;

signals:
    void clicked(const QColor& c);

public:
    explicit ColorCell(QWidget* parent);

    const QColor& GetColor() const;
    void SetColor(const QColor& color);

private slots:
    void OnMouseEnter();
    void OnMouseLeave();
    void OnMouseClick();

private:
    void paintEvent(QPaintEvent* e);

    void dragEnterEvent(QDragEnterEvent* e);
    void dragLeaveEvent(QDragLeaveEvent* e);
    void dragMoveEvent(QDragMoveEvent* e);
    void dropEvent(QDropEvent* e);

    QColor color;
    bool isHovered;
    QBrush bgBrush;
    QPointer<MouseHelper> mouse;
};
}
}
