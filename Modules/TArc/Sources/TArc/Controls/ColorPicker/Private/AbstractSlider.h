#pragma once

#include <QWidget>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class MouseHelper;
class AbstractSlider : public QWidget
{
    Q_OBJECT

signals:
    void started(const QPointF&);
    void changing(const QPointF&);
    void changed(const QPointF&);
    void canceled();

public:
    explicit AbstractSlider(QWidget* parent);

    const QPointF& PosF() const;
    void SetPosF(const QPointF& posF);

protected:
    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* e);

    virtual void DrawBackground(QPainter* p) const;
    virtual void DrawForeground(QPainter* p) const;
    virtual QRect PosArea() const;

    QPoint Pos() const;
    void SetPos(const QPoint& pos);
    MouseHelper* Mouse() const;

private slots:
    void OnMousePress(const QPoint& pos);
    void OnMouseMove(const QPoint& pos);
    void OnMouseRelease(const QPoint& pos);

private:
    QPointF posF;
    QPoint pressPos;
    QPointer<MouseHelper> mouse;
};
}
}
