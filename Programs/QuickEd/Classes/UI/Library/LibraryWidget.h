#ifndef __QUICKED_LIBRARY_WIDGET_H__
#define __QUICKED_LIBRARY_WIDGET_H__

#include <QDockWidget>
#include <QPointer>
#include "ui_LibraryWidget.h"

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "FileSystem/FilePath.h"

class Document;
class LibraryModel;
class Project;

class LibraryWidget : public QDockWidget, public Ui::LibraryWidget
{
    Q_OBJECT
public:
    LibraryWidget(QWidget* parent = nullptr);
    ~LibraryWidget() = default;

    void SetLibraryPackages(const DAVA::Vector<DAVA::FilePath>& libraryPackages);
    void SetPrototypes(const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes);

public slots:
    void OnDocumentChanged(Document* document);

private:
    LibraryModel* libraryModel;
};

#endif // __QUICKED_LIBRARY_WIDGET_H__
