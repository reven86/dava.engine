#ifndef __QUICKED_PROPERTIES_WIDGET_H__
#define __QUICKED_PROPERTIES_WIDGET_H__

#include <QDockWidget>
#include "ui_PropertiesWidget.h"

class SharedData;

class ControlNode;

class PropertiesWidget : public QDockWidget, public Ui::PropertiesWidget
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget *parent = NULL);

public slots:
    void OnDocumentChanged(SharedData *sharedData);
    void OnDataChanged(const QByteArray &role);

    void OnAddComponent(QAction *action);
    void OnRemoveComponent();
    void OnSelectionChanged(const QItemSelection &selected,
                            const QItemSelection &deselected);

private:
    void UpdateActivatedControls();
    SharedData *sharedData;
    QAction *addComponentAction;
    QAction *removeComponentAction;
};

#endif //__QUICKED_PROPERTIES_WIDGET_H__
