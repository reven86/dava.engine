#ifndef JOYPADPROPERTYGRIDWIDGET_H
#define JOYPADPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class JoypadPropertyGridWidget;
}

class JoypadPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit JoypadPropertyGridWidget(QWidget *parent = 0);
    ~JoypadPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);

protected:
    virtual void UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
                                                            const QMetaProperty& curProperty);
    
protected slots:
    void OpenSpriteDialog();
    void RemoveSprite();
    
    
    
private:
    Ui::JoypadPropertyGridWidget *ui;
};

#endif // JOYPADPROPERTYGRIDWIDGET_H
