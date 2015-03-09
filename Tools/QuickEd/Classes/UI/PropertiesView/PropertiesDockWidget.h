//
//  PropertiesTreeWidget.h
//  UIEditor
//
//  Created by Dmitry Belsky on 12.9.14.
//
//

#ifndef __UI_EDITOR_PROPERTIES_TREE_WIDGET_H__
#define __UI_EDITOR_PROPERTIES_TREE_WIDGET_H__

#include <QWidget>
#include <QDockWidget>

#include "DAVAEngine.h"

namespace Ui {
    class PropertiesDockWidget;
}

class PropertiesViewContext;
class ControlNode;

class PropertiesDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    PropertiesDockWidget(QWidget *parent = NULL);
    virtual ~PropertiesDockWidget();
    
    void SetContext(PropertiesViewContext *newContext);

protected:
    void SetControl(ControlNode *controlNode);
    
private slots:
    void OnControlsSelectionChanged(const QList<ControlNode*> &activatedControls, const QList<ControlNode*> &deactivatedControls);

private:
    Ui::PropertiesDockWidget *ui;
    PropertiesViewContext *context;
};

#endif // __UI_EDITOR_PROPERTIES_TREE_WIDGET_H__
