#ifndef __QUICKED_PROPERTIES_WIDGET_H__
#define __QUICKED_PROPERTIES_WIDGET_H__

#include <QWidget>
#include <QDockWidget>

#include "DAVAEngine.h"

namespace Ui {
    class PropertiesWidget;
}

class PropertiesContext;
class ControlNode;

class PropertiesWidget : public QDockWidget
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget *parent = NULL);
    virtual ~PropertiesWidget();
    
    void SetContext(PropertiesContext *newContext);

protected:
    void SetControl(ControlNode *controlNode);
    
private slots:
    void OnControlsSelectionChanged(const QList<ControlNode*> &activatedControls, const QList<ControlNode*> &deactivatedControls);

private:
    Ui::PropertiesWidget *ui;
    PropertiesContext *context;
};

#endif //__QUICKED_PROPERTIES_WIDGET_H__
