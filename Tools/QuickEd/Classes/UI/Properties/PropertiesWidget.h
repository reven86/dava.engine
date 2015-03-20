#ifndef __QUICKED_PROPERTIES_WIDGET_H__
#define __QUICKED_PROPERTIES_WIDGET_H__

#include <QDockWidget>

class WidgetContext;

namespace Ui {
    class PropertiesWidget;
}

class ControlNode;

class PropertiesWidget : public QDockWidget
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget *parent = NULL);
    virtual ~PropertiesWidget();

public slots:
void OnContextChanged(WidgetContext *context);
void OnDataChanged(const QString &role);

private:
    void UpdateModel();
    Ui::PropertiesWidget *ui;
    WidgetContext *widgetContext;
};

#endif //__QUICKED_PROPERTIES_WIDGET_H__
