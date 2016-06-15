#ifndef __QUICKED_LIBRARY_WIDGET_H__
#define __QUICKED_LIBRARY_WIDGET_H__

#include <QDockWidget>
#include <QPointer>
#include "ui_LibraryWidget.h"

class Document;
class LibraryModel;

class LibraryWidget : public QDockWidget, public Ui::LibraryWidget
{
    Q_OBJECT
public:
    LibraryWidget(QWidget* parent = nullptr);
    ~LibraryWidget() = default;
public slots:
    void OnDocumentChanged(Document* document);

private:
    LibraryModel* libraryModel;
};

#endif // __QUICKED_LIBRARY_WIDGET_H__
