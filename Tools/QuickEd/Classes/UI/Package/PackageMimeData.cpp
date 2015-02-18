#include "PackageMimeData.h"

#include "Model/PackageHierarchy/ControlNode.h"

using namespace DAVA;

const QString PackageMimeData::MIME_TYPE = "application/packageModel";

PackageMimeData::PackageMimeData()
{
}

PackageMimeData::~PackageMimeData()
{
    for (ControlNode *node : nodes)
        node->Release();
    nodes.clear();
}

void PackageMimeData::AddControlNode(ControlNode *node)
{
    nodes.push_back(SafeRetain(node));
}

const Vector<ControlNode*> &PackageMimeData::GetControlNodes() const
{
    return nodes;
}

bool PackageMimeData::hasFormat(const QString &mimetype) const
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

QVariant PackageMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
{
    if (mimetype == MIME_TYPE)
        return QVariant(QVariant::UserType);
    
    return QMimeData::retrieveData(mimetype, preferredType);
}
