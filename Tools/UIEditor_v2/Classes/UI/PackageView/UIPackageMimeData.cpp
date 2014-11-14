#include "UIPackageMimeData.h"

UIPackageMimeData::UIPackageMimeData()
{
}

UIPackageMimeData::~UIPackageMimeData()
{
}

bool UIPackageMimeData::hasFormat(const QString &mimetype) const
{
    if (mimetype == "application/packageModel")
        return true;
    return QMimeData::hasFormat(mimetype);
}

QStringList UIPackageMimeData::formats() const
{
    QStringList types;
    types << "application/packageModel";
    return types;
}

QVariant UIPackageMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
{
    if (mimetype == "application/packageModel")
        return QVariant(QVariant::UserType);
    
    return QMimeData::retrieveData(mimetype, preferredType);
}
