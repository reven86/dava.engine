#ifndef EYEDROPPER_H
#define EYEDROPPER_H

#include <QWidget>
#include <QPointer>
#include <QPixmap>
#include <QList>


class DropperShade;

class EyeDropper
    : public QObject
{
    Q_OBJECT

private:

signals:
    void canceled();
    void picked(const QColor& color);

public:
    explicit EyeDropper(QObject* parent = NULL);
    ~EyeDropper();

public slots:
    void Exec();

private:
};


#endif // EYEDROPPER_H
