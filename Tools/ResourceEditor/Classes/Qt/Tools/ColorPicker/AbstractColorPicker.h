#ifndef ABSTRACTCOLORPICKER_H
#define ABSTRACTCOLORPICKER_H

#include <QWidget>


class AbstractColorPicker
    : public QWidget
{
    Q_OBJECT

signals:
    void begin();                       // Sends, when user starts color modification (click on color editor).
    void changing(const QColor& c);     // Sends, when user modifying color, but doesn't complete action. Used for editors that supports mouse dragging.
    void changed(const QColor& c);      // Sends, when user accepts editing. For example - release mouse button.
    void canceled();                    // Sends, when user calcels editing. For expample - picked same color, as been before.

public:
    explicit AbstractColorPicker(QWidget* parent);
    ~AbstractColorPicker();

    const QColor& GetColor() const;

public slots:
    void SetColor(const QColor& c);

protected:
    virtual void SetColorInternal(const QColor& c) = 0;
    QColor color;
};


#endif // ABSTRACTCOLORPICKER_H
