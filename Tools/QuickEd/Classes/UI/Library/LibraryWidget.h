#ifndef __QUICKED_LIBRARY_WIDGET_H__
#define __QUICKED_LIBRARY_WIDGET_H__

#include <QDockWidget>
class QAbstractItemModel;
class WidgetContext;

namespace Ui {
    class LibraryWidget;
}

class LibraryWidget: public QDockWidget
{
    Q_OBJECT
public:
    LibraryWidget(QWidget *parent = NULL);
    virtual ~LibraryWidget();
public slots:
    void OnContextChanged(WidgetContext *context);
    void OnDataChanged(const QByteArray &role);
private:
    void UpdateModel();
    Ui::LibraryWidget *ui;
    WidgetContext *widgetContext;
};

#endif // __QUICKED_LIBRARY_WIDGET_H__
