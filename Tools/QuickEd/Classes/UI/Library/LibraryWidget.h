#ifndef __QUICKED_LIBRARY_WIDGET_H__
#define __QUICKED_LIBRARY_WIDGET_H__

#include <QDockWidget>
#include "ui_LibraryWidget.h"

class SharedData;

class LibraryWidget : public QDockWidget, public Ui::LibraryWidget
{
    Q_OBJECT
public:
    LibraryWidget(QWidget *parent = nullptr);
    ~LibraryWidget() = default;
public slots:
    void OnDocumentChanged(SharedData *data);
private:
    void LoadContext();
    SharedData *sharedData;
};

#endif // __QUICKED_LIBRARY_WIDGET_H__
