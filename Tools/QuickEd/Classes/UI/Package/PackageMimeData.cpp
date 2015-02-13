#include "PackageMimeData.h"

PackageMimeData::PackageMimeData()
{
}

PackageMimeData::~PackageMimeData()
{
}

bool PackageMimeData::hasFormat(const QString &mimetype) const
{
    if (mimetype == "application/packageModel")
        return true;
    return QMimeData::hasFormat(mimetype);
}

QStringList PackageMimeData::formats() const
{
    QStringList types;
    types << "text/plain";
    types << "application/packageModel";
    return types;
}

QVariant PackageMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
{
    if (mimetype == "application/packageModel")
        return QVariant(QVariant::UserType);
    
    return QMimeData::retrieveData(mimetype, preferredType);
}
