#ifndef __VECTOR2EDIT_H__
#define __VECTOR2EDIT_H__

#include <QWidget>
#include <QVector2D>

class QLineEdit;

class Vector2DEdit: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVector2D Vector2D READ vector2D WRITE setVector2D USER true);
public:
    explicit Vector2DEdit(QWidget *parent = NULL);
    ~Vector2DEdit();

    QVector2D vector2D() const;
    void setVector2D(const QVector2D &newValue);

    QLineEdit *lineEditX() const { return editX; }
    QLineEdit *lineEditY() const { return editY; }

private:
    QLineEdit *editX;
    QLineEdit *editY;
};

#endif // __VECTOR2EDIT_H__
