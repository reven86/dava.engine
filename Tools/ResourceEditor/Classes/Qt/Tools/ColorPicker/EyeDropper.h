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
    struct ScreenData
    {
        int id;
        QRect rc;
    };
    typedef QVector< QPointer<DropperShade> > Shades;
    typedef QVector< ScreenData > ScreenArray;

signals:
    void canceled();
    void picked(const QColor& color);
    void moved(const QColor& color);

public:
    explicit EyeDropper(QWidget* parent = NULL);
    ~EyeDropper();

public slots:
    void Exec();

private slots:
    void OnDone();

private:
    void InitShades();

    QPointer<QWidget> parentWidget;
    Shades shades;
};


#endif // EYEDROPPER_H
