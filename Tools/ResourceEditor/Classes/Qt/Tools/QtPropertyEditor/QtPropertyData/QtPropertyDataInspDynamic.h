/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
    QtPropertyDataInspDynamic(DAVA::InspInfoDynamic* _dynamicInfo, DAVA::InspInfoDynamic::DynamicData _ddata, DAVA::FastName name);
    virtual ~QtPropertyDataInspDynamic();

	int InspFlags() const;

	virtual const DAVA::MetaInfo * MetaInfo() const;
	virtual void* CreateLastCommand() const;

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
	DAVA::InspInfoDynamic *dynamicInfo;
    DAVA::InspInfoDynamic::DynamicData ddata;

protected:
	int inspFlags;
	InspDynamicModifyCommand* lastCommand;


	virtual QVariant GetValueAlias() const;
	virtual void SetValueInternal(const QVariant &value);
    virtual void SetTempValueInternal(const QVariant& value);
	virtual bool UpdateValueInternal();
	virtual bool EditorDoneInternal(QWidget *editor);
};

#endif // __QT_PROPERTY_DATA_INSP_DYNAMIC_H__
