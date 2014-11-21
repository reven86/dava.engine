#ifndef DROPPERLENS_H
#define DROPPERLENS_H


#include <QWidget>
#include <QPointer>
#include <QPixmap>


class DropperLens
    : public QWidget
{
    Q_OBJECT

public:
    explicit DropperLens(QWidget* parent);
    ~DropperLens();
};


#endif // DROPPERLENS_H
