#ifndef __QUICKED_LIBRARY_WIDGET_H__
#define __QUICKED_LIBRARY_WIDGET_H__

#include <QDockWidget>

class QAbstractItemModel;
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
    void OnModelChanged(QAbstractItemModel*);    
private:
    Ui::LibraryWidget *ui;
};

#endif // __QUICKED_LIBRARY_WIDGET_H__
