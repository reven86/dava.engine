#ifndef __QUICKED_LIBRARY_WIDGET_H__
#define __QUICKED_LIBRARY_WIDGET_H__

#include <QDockWidget>
#include "ui_LibraryWidget.h"

class WidgetContext;

class LibraryWidget : public QDockWidget, public Ui::LibraryWidget
{
    Q_OBJECT
public:
    LibraryWidget(QWidget *parent = nullptr);
    ~LibraryWidget() = default;
public slots:
    void OnContextChanged(WidgetContext *context);
private:
    void LoadDelta();
    WidgetContext *widgetContext;
};

#endif // __QUICKED_LIBRARY_WIDGET_H__
