#ifndef __LABELED_SLIDER_H__
#define __LABELED_SLIDER_H__

#include <QWidget>

class QLabel;
class QSlider;
class LabeledSlider : public QWidget
{
    Q_OBJECT

public:
    LabeledSlider(QWidget* parent = 0);
    ~LabeledSlider();

    void setMinimum(int);
    int minimum() const;

    void setMaximum(int);
    int maximum() const;

    void setRange(int min, int max);

    int value() const;

signals:
    void valueChanged(int value);

public slots:
    void setValue(int);

protected slots:
    void ValueChanged(int value);
    void RangeChanged(int min, int max);

protected:
    void InitUI();

private:
    QSlider* slider;
    QLabel* minText;
    QLabel* maxText;
    QLabel* valueText;
};

#endif // __LABELED_SLIDER_H__
