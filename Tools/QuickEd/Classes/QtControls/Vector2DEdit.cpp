#include "Vector2DEdit.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QValidator>

Vector2DEdit::Vector2DEdit(QWidget *parent)
    : QWidget(parent)
    , editX(NULL)
    , editY(NULL)
{
    QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setSpacing(1);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    QLabel * label1 = new QLabel(this);
    label1->setText("[");

    horizontalLayout->addWidget(label1);

    editX = new QLineEdit(this);
    editX->setObjectName(QString::fromUtf8("lineEditX"));
    editX->setValidator(new QRegExpValidator(QRegExp("\\s*-?\\d*[,\\.]?\\d*\\s*")));
    horizontalLayout->addWidget(editX);

    QLabel * label2 = new QLabel(this);
    label2->setText(",");

    horizontalLayout->addWidget(label2);

    editY = new QLineEdit(this);
    editY->setObjectName(QString::fromUtf8("lineEditY"));
    editY->setValidator(new QRegExpValidator(QRegExp("\\s*-?\\d*[,\\.]?\\d*\\s*")));
    horizontalLayout->addWidget(editY);

    QLabel * label3 = new QLabel(this);
    label3->setText("]");

    horizontalLayout->addWidget(label3);

    horizontalLayout->setStretch(1, 1);
    horizontalLayout->setStretch(3, 1);

    setAutoFillBackground(true);
}

Vector2DEdit::~Vector2DEdit()
{
}

QVector2D Vector2DEdit::vector2D() const
{
    return QVector2D(editX->text().toDouble(), editY->text().toDouble() );
}

void Vector2DEdit::setVector2D(const QVector2D &newValue)
{
    if (signalsBlocked())
    {
        editX->blockSignals(true);
        editY->blockSignals(true);
    }

    editX->setText(QString("%1").arg(newValue.x()));
    editY->setText(QString("%1").arg(newValue.y()));

    if (signalsBlocked())
    {
        editX->blockSignals(false);
        editY->blockSignals(false);
    }
}
