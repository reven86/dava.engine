#ifndef LINEEDITEX_H
#define LINEEDITEX_H


#include <QLineEdit>
#include <QPointer>
#include <QTimer>


class LineEditEx
    : public QLineEdit
{
    Q_OBJECT

    signals:
    void textUpdated(const QString& text);

public:
    explicit LineEditEx(QWidget* parent = NULL);
    ~LineEditEx();

    void SetAcceptInterval(int msec);

private slots:
    void OnTextEdit();
    void OnAcceptEdit();

private:
    QPointer<QTimer> timer;
};


#endif // LINEEDITEX_H
