#ifndef __UIEditor__LibraryDockWidget__
#define __UIEditor__LibraryDockWidget__

#include <QWidget>
#include <QDockWidget>

namespace Ui {
    class LibraryDockWidget;
}

class PackageDocument;

class LibraryDockWidget: public QDockWidget
{
    Q_OBJECT
public:
    explicit LibraryDockWidget(QWidget *parent = NULL);
    virtual ~LibraryDockWidget();

public:
    void SetDocument(PackageDocument *newDocument);
    
private:
    Ui::LibraryDockWidget *ui;
    PackageDocument *document;
};

#endif /* defined(__UIEditor__LibraryDockWidget__) */
