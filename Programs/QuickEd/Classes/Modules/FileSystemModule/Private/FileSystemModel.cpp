#include "Modules/FileSystemModule/FileSystemModel.h"
#include "Modules/LegacySupportModule/Private/Project.h"

#include <QRegularExpression>

FileSystemModel::FileSystemModel(QObject* parent)
    : QFileSystemModel(parent)
{
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QFileSystemModel::flags(index) | Qt::ItemIsEditable;

    //qfilesystemModel not detect isDir correctly after dirLoaded signal is emited
    //same bug: https://bugreports.qt.io/browse/QTBUG-27388
    //is reproduced on Qt 5.6
    QFileInfo fi(filePath(index));
    bool isDir = fi.isDir();
    if (isDir)
    {
        flags &= ~Qt::ItemNeverHasChildren;
    }

    return flags;
}

QVariant FileSystemModel::data(const QModelIndex& index, int role) const
{
    QVariant data = QFileSystemModel::data(index, role);
    if (index.isValid() && role == Qt::EditRole && !isDir(index) && data.canConvert<QString>())
    {
        return data.toString().remove(QRegularExpression(Project::GetUiFileExtension() + "$"));
    }
    return data;
}

bool FileSystemModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (idx.isValid() && !isDir(idx) && value.canConvert<QString>())
    {
        QString name = value.toString();
        if (!name.endsWith(Project::GetUiFileExtension()))
        {
            return QFileSystemModel::setData(idx, name + Project::GetUiFileExtension(), role);
        }
    }
    return QFileSystemModel::setData(idx, value, role);
}
