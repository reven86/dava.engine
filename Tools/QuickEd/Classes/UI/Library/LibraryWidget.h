#ifndef __QUICKED_LIBRARY_WIDGET_H__
#define __QUICKED_LIBRARY_WIDGET_H__

#include <QWidget>
#include <QDockWidget>

namespace Ui {
    class LibraryWidget;
}

class Document;

class LibraryWidget: public QDockWidget
{
    Q_OBJECT
public:
    LibraryWidget(QWidget *parent = NULL);
    virtual ~LibraryWidget();
public slots:
    void OnModelChanged(QAbstractItemModel*);    
private:
    Ui::LibraryWidget *ui;
};

#endif // __QUICKED_LIBRARY_WIDGET_H__
