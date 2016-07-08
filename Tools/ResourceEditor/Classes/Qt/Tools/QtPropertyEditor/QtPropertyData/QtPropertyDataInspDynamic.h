#ifndef __QT_PROPERTY_DATA_INSP_DYNAMIC_H__
#define __QT_PROPERTY_DATA_INSP_DYNAMIC_H__

#include "Base/Introspection.h"
#include "Base/FastName.h"

#include "../QtPropertyData.h"
#include "QtPropertyDataDavaVariant.h"
#include "Commands2/InspDynamicModifyCommand.h"

class QtPropertyDataInspDynamic : public QtPropertyDataDavaVariant
{
public:
    QtPropertyDataInspDynamic(const DAVA::FastName& name, DAVA::InspInfoDynamic* _dynamicInfo, DAVA::InspInfoDynamic::DynamicData _ddata);
    virtual ~QtPropertyDataInspDynamic();

    int InspFlags() const;

    const DAVA::MetaInfo* MetaInfo() const override;
    Command2::Pointer CreateLastCommand() const override;

    DAVA::InspInfoDynamic* GetDynamicInfo() const
    {
        return dynamicInfo;
    }

    DAVA::VariantType GetVariant() const
    {
        return dynamicInfo->MemberValueGet(ddata, name);
    }

    DAVA::VariantType GetAliasVariant() const
    {
        return dynamicInfo->MemberAliasGet(ddata, name);
    }

    DAVA::FastName name;
    DAVA::InspInfoDynamic* dynamicInfo;
    DAVA::InspInfoDynamic::DynamicData ddata;

protected:
    int inspFlags;
    InspDynamicModifyCommand* lastCommand;

    virtual QVariant GetValueAlias() const;
    virtual void SetValueInternal(const QVariant& value);
    virtual void SetTempValueInternal(const QVariant& value);
    virtual bool UpdateValueInternal();
    virtual bool EditorDoneInternal(QWidget* editor);
};

#endif // __QT_PROPERTY_DATA_INSP_DYNAMIC_H__
