#pragma once

#include <QObject>
#include <QDoubleSpinBox>
#include <QWidget>
#include <QKeyEvent>

class EventFilterDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    explicit EventFilterDoubleSpinBox(QWidget* parent = 0);

    QValidator::State validate(QString& input, int& pos) const override;
    double valueFromText(const QString& text) const override;
    QString textFromValue(double val) const override;

private:
    void keyPressEvent(QKeyEvent* event) override;
};
