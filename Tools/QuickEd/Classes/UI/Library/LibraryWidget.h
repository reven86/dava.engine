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

public:
    void SetDocument(Document *newDocument);
    
private:
    Ui::LibraryWidget *ui;
    Document *document;
};

#endif // __QUICKED_LIBRARY_WIDGET_H__
