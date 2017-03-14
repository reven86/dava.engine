#pragma once

#include <QWidget>
#include <QPointer>

class QLineEdit;

namespace DAVA
{
namespace TArc
{
class MouseHelper;
class ValueSlider : public QWidget
{
    Q_OBJECT

signals:
    void started(double);
    void changing(double);
    void changed(double);
    void canceled();

public:
    explicit ValueSlider(QWidget* parent = NULL);

    void SetDigitsAfterDot(int c);
    void SetRange(double min, double max);
    void SetValue(double val);
    double GetValue() const;
    double GetMaxValue() const;

protected:
    virtual void DrawBackground(QPainter* p) const;
    virtual void DrawForeground(QPainter* p) const;
    virtual QRect PosArea() const;

    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* e);

    bool eventFilter(QObject* obj, QEvent* e);

    bool IsEditorMode() const;

private slots:
    void OnMousePress(const QPoint& pos);
    void OnMouseMove(const QPoint& pos);
    void OnMouseRelease(const QPoint& pos);
    void OnMouseClick();

private:
    void normalize();
    void undoEditing();
    void acceptEditing();

    double minVal;
    double maxVal;
    double val;
    int digitsAfterDot;

    QPointer<MouseHelper> mouse;
    QPoint clickPos;
    double clickVal;
    mutable QPixmap arrows;

    QPointer<QLineEdit> editor;
};
}
}
