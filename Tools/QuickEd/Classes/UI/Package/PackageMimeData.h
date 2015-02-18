#ifndef __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__
#define __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__

#include <QMimeData>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QStringList>

#include "Base/BaseTypes.h"

class ControlNode;

class PackageMimeData: public QMimeData
{
    Q_OBJECT
    
public:
    static const QString MIME_TYPE;

public:
    PackageMimeData();
    virtual ~PackageMimeData();
    
    void AddControlNode(ControlNode *node);
    const DAVA::Vector<ControlNode*> &GetControlNodes() const;
    
    virtual bool hasFormat(const QString &mimetype) const override;
    virtual QStringList formats() const override;
    
protected:
    virtual QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const override;
    
private:
    DAVA::Vector<ControlNode*> nodes;
};


#endif // __UI_EDITOR_UI_PACKAGE_MIME_DATA_H__
