#ifndef __UIEditor__LibraryDockWidget__
#define __UIEditor__LibraryDockWidget__

#include <QWidget>
#include <QDockWidget>

namespace Ui {
    class LibraryDockWidget;
}

class Document;

class LibraryDockWidget: public QDockWidget
{
    Q_OBJECT
public:
    explicit LibraryDockWidget(QWidget *parent = NULL);
    virtual ~LibraryDockWidget();

public:
    void SetDocument(Document *newDocument);
    
private:
    Ui::LibraryDockWidget *ui;
    Document *document;
};

#endif /* defined(__UIEditor__LibraryDockWidget__) */
