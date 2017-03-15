#pragma once

#include <QWidget>
#include <QPointer>
#include <QPixmap>
#include <QList>

namespace DAVA
{
namespace TArc
{
class DropperShade;

class EyeDropper : public QObject
{
    Q_OBJECT

private:
    typedef QVector<QPointer<DropperShade>> Shades;

signals:
    void canceled();
    void picked(const QColor& color);
    void moved(const QColor& color);

public:
    explicit EyeDropper(QWidget* parent = NULL);

public slots:
    void Exec();

private slots:
    void OnDone();

private:
    void InitShades();

    QPointer<QWidget> parentWidget;
    Shades shades;
};
}
}
