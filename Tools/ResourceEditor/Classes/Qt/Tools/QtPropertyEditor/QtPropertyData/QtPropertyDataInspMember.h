#ifndef __QT_PROPERTY_DATA_INSP_MEMBER_H__
#define __QT_PROPERTY_DATA_INSP_MEMBER_H__

#include "Base/Introspection.h"
#include "../QtPropertyData.h"
#include "QtPropertyDataDavaVariant.h"
#include "Commands2/InspMemberModifyCommand.h"

class QtPropertyDataInspMember : public QtPropertyDataDavaVariant
{
public:
    QtPropertyDataInspMember(const DAVA::FastName& name, void* _object, const DAVA::InspMember* _member);
    virtual ~QtPropertyDataInspMember();

    const DAVA::MetaInfo* MetaInfo() const override;
    Command2::Pointer CreateLastCommand() const override;

    void* object;
    const DAVA::InspMember* member;

protected:
    InspMemberModifyCommand* lastCommand;

    virtual void SetValueInternal(const QVariant& value);
    virtual void SetTempValueInternal(const QVariant& value);
    virtual bool UpdateValueInternal();
    virtual bool EditorDoneInternal(QWidget* editor);
};

#endif // __QT_PROPERTY_DATA_INSP_MEMBER_H__
