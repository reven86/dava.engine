#ifndef __QT_PROPERTY_DATA_META_OBJECT_H__
#define __QT_PROPERTY_DATA_META_OBJECT_H__

#include "Base/Introspection.h"
#include "../QtPropertyData.h"
#include "QtPropertyDataDavaVariant.h"
#include "Commands2/MetaObjModifyCommand.h"

class QtPropertyDataMetaObject : public QtPropertyDataDavaVariant
{
public:
    QtPropertyDataMetaObject(const DAVA::FastName& name, void* _object, const DAVA::MetaInfo* _meta);
    virtual ~QtPropertyDataMetaObject();

    const DAVA::MetaInfo* MetaInfo() const override;
    Command2::Pointer CreateLastCommand() const override;

    void* object;
    const DAVA::MetaInfo* meta;

protected:
    MetaObjModifyCommand* lastCommand;

    virtual void SetValueInternal(const QVariant& value);
    virtual bool UpdateValueInternal();
    virtual bool EditorDoneInternal(QWidget* editor);
};

#endif // __QT_PROPERTY_DATA_META_OBJECT_H__
