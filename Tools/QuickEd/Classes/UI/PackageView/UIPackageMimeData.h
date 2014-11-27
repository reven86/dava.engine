#ifndef __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__
#define __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__

#include <QMimeData>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QStringList>

class UIPackageMimeData: public QMimeData
{
    Q_OBJECT
public:
    UIPackageMimeData();
    ~UIPackageMimeData();
    
    void SetIndex(const QModelIndex &_index ){ index = _index;}
    const QModelIndex &GetIndex() const{ return index;}
    
    virtual bool hasFormat(const QString &mimetype) const override;
    virtual QStringList formats() const override;
    
protected:
    virtual QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const override;
    
private:
    QPersistentModelIndex index;
};



#endif // __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__
