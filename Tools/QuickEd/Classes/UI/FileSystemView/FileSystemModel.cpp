#include "FileSystemModel.h"
#include <QRegularExpression>

FileSystemModel::FileSystemModel(QObject* parent)
    : QFileSystemModel(parent)
{
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex& index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant FileSystemModel::data(const QModelIndex& index, int role) const
{
    QVariant data = QFileSystemModel::data(index, role);
    if (index.isValid() && role == Qt::EditRole && !isDir(index) && data.canConvert<QString>())
    {
        return data.toString().remove(QRegularExpression(GetYamlExtensionString() + "$"));
    }
    return data;
}

bool FileSystemModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (idx.isValid() && !isDir(idx) && value.canConvert<QString>())
    {
        QString name = value.toString();
        if (!name.endsWith(GetYamlExtensionString()))
        {
            return QFileSystemModel::setData(idx, name + GetYamlExtensionString(), role);
        }
    }
    return QFileSystemModel::setData(idx, value, role);
}

QString FileSystemModel::GetYamlExtensionString()
{
    static QString str(".yaml");
    return str;
}
