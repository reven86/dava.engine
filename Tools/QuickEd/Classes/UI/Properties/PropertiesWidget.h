#ifndef __QUICKED_PROPERTIES_WIDGET_H__
#define __QUICKED_PROPERTIES_WIDGET_H__

#include <QDockWidget>
#include "ui_PropertiesWidget.h"

class WidgetContext;


class ControlNode;

class PropertiesWidget : public QDockWidget, public Ui::PropertiesWidget
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget *parent = NULL);

public slots:
    void OnContextChanged(WidgetContext *context);
    void OnDataChanged(const QByteArray &role);

private:
    void UpdateActivatedControls();
    WidgetContext *widgetContext;
};

#endif //__QUICKED_PROPERTIES_WIDGET_H__
