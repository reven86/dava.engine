#ifndef __QUICKED_DOCUMENT_WIDGETS_H__
#define __QUICKED_DOCUMENT_WIDGETS_H__

#include <QObject>

class PackageWidget;
class PropertiesWidget;
class PreviewWidget;
class LibraryWidget;

class DocumentWidgets : public QObject
{
    Q_OBJECT
public:
    DocumentWidgets(QObject *parent, PackageWidget *_packageWidget, PropertiesWidget *_propertiesWidget, PreviewWidget *_previewWidget, LibraryWidget *_libraryWidget);
    virtual ~DocumentWidgets();
    
    PackageWidget *GetPackageWidget() const;
    PropertiesWidget *GetPropertiesWidget() const;
    PreviewWidget *GetPreviewWidget() const;
    LibraryWidget *GetLibraryWidget() const;
    
    void SetWidgetsEnabled(bool active);
    
private:
    PackageWidget *packageWidget;
    PropertiesWidget *propertiesWidget;
    PreviewWidget *previewWidget;
    LibraryWidget *libraryWidget;
};

#endif // __QUICKED_DOCUMENT_WIDGETS_H__
