#ifndef __QUICKED_PROPERTIES_WIDGET_H__
#define __QUICKED_PROPERTIES_WIDGET_H__

#include <QWidget>
#include <QDockWidget>

class Document;
class PropertiesContext;

namespace Ui {
    class PropertiesWidget;
}

class PropertiesContext;
class ControlNode;
class PropertiesModel;

class PropertiesWidget : public QDockWidget
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget *parent = NULL);
    virtual ~PropertiesWidget();
    void SetDocument(Document *document);

private slots:
    void OnModelChanged(PropertiesModel *model);
    void OnAddComponent(QAction *action);

private:
    Ui::PropertiesWidget *ui;
    PropertiesContext *context;
    
    QAction *addComponentAction;
    QAction *removeComponentAction;
};

#endif //__QUICKED_PROPERTIES_WIDGET_H__
