#ifndef __QUICKED_FILE_SYSTEM_MODEL_H__
#define __QUICKED_FILE_SYSTEM_MODEL_H__

#include <QFileSystemModel>

class FileSystemModel : public QFileSystemModel
{
public:
    FileSystemModel(QObject* parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) override;
};

#endif //__QUICKED_FILE_SYSTEM_MODEL_H__