#include "PackageMimeData.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"

using namespace DAVA;

const QString PackageMimeData::MIME_TYPE = "application/packageModel";

PackageMimeData::PackageMimeData()
{
}

PackageMimeData::~PackageMimeData()
{
    for (ControlNode* control : controls)
        control->Release();
    controls.clear();

    for (StyleSheetNode* style : styles)
        style->Release();

    styles.clear();
}

void PackageMimeData::AddControl(ControlNode* node)
{
    controls.push_back(SafeRetain(node));
}

void PackageMimeData::AddStyle(StyleSheetNode* node)
{
    styles.push_back(SafeRetain(node));
}

const Vector<ControlNode*>& PackageMimeData::GetControls() const
{
    return controls;
}

const Vector<StyleSheetNode*>& PackageMimeData::GetStyles() const
{
    return styles;
}

bool PackageMimeData::hasFormat(const QString& mimetype) const
{
    if (mimetype == MIME_TYPE)
        return true;
    return QMimeData::hasFormat(mimetype);
}

QStringList PackageMimeData::formats() const
{
    QStringList types;
    types << "text/plain";
    types << MIME_TYPE;
    return types;
}

QVariant PackageMimeData::retrieveData(const QString& mimetype, QVariant::Type preferredType) const
{
    if (mimetype == MIME_TYPE)
        return QVariant(QVariant::UserType);

    return QMimeData::retrieveData(mimetype, preferredType);
}
