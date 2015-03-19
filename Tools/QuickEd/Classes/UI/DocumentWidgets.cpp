#include "DocumentWidgets.h"

#include "Package/PackageWidget.h"
#include "Properties/PropertiesWidget.h"
#include "Preview/PreviewWidget.h"
#include "Library/LibraryWidget.h"

DocumentWidgets::DocumentWidgets(QObject *parent, PackageWidget *_packageWidget, PropertiesWidget *_propertiesWidget, PreviewWidget *_previewWidget, LibraryWidget *_libraryWidget)
    : QObject(parent)
    , packageWidget(_packageWidget)
    , propertiesWidget(_propertiesWidget)
    , previewWidget(_previewWidget)
    , libraryWidget(_libraryWidget)
{
    SetWidgetsEnabled(false);
}

DocumentWidgets::~DocumentWidgets()
{
    
}

PackageWidget *DocumentWidgets::GetPackageWidget() const
{
    return packageWidget;
}

PropertiesWidget *DocumentWidgets::GetPropertiesWidget() const
{
    return propertiesWidget;
}

PreviewWidget *DocumentWidgets::GetPreviewWidget() const
{
    return previewWidget;
}

LibraryWidget *DocumentWidgets::GetLibraryWidget() const
{
    return libraryWidget;
}

void DocumentWidgets::SetWidgetsEnabled(bool active)
{
    packageWidget->setEnabled(active);
    propertiesWidget->setEnabled(active);
    previewWidget->setEnabled(active);
    libraryWidget->setEnabled(active);
}
