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


#include "Commands2/InspDynamicModifyCommand.h"

InspDynamicModifyCommand::InspDynamicModifyCommand(DAVA::InspInfoDynamic* _dynamicInfo, const DAVA::InspInfoDynamic::DynamicData& _ddata, DAVA::FastName _key, const DAVA::VariantType& _newValue)
    : Command2(CMDID_INSP_DYNAMIC_MODIFY, "Modify dynamic value")
    , dynamicInfo(_dynamicInfo)
    , key(_key)
    , newValue(_newValue)
    , ddata(_ddata)
{
    if (nullptr != dynamicInfo)
    {
        // if value can't be edited, it means that it was inherited
        // so don't retrieve oldValue, but leave it as uninitialized variant
        if (dynamicInfo->MemberFlags(ddata, key) & DAVA::I_EDIT)
        {
            oldValue = dynamicInfo->MemberValueGet(ddata, key);
        }
	}
}

InspDynamicModifyCommand::~InspDynamicModifyCommand()
{ }

void InspDynamicModifyCommand::Undo()
{
    if (nullptr != dynamicInfo)
    {
        dynamicInfo->MemberValueSet(ddata, key, oldValue);
    }
}

void InspDynamicModifyCommand::Redo()
{
    if (nullptr != dynamicInfo)
    {
        dynamicInfo->MemberValueSet(ddata, key, newValue);
    }
}
