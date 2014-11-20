#ifndef EYEDROPPERV2_H
#define EYEDROPPERV2_H

#include <QWidget>
#include <QPointer>
#include <QPixmap>
#include <QList>



class EyeDropperV2
    : public QWidget
{
    Q_OBJECT

private:

signals:
    void canceled();
    void picked(const QColor& color);
    void moved(const QColor& color);

public:
    explicit EyeDropperV2(QWidget* parent = NULL);
    ~EyeDropperV2();

public slots:
    void Exec();

private slots:
    void OnDone();

private:
};


#endif // EYEDROPPERV2_H
