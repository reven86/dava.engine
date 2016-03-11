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

#ifndef __RESOURCEEDITOR_MATERIALCONFIGCOMMANDS_H__
#define __RESOURCEEDITOR_MATERIALCONFIGCOMMANDS_H__

#include "Command2.h"

#include "Render/Material/NMaterial.h"

class MaterialConfigModify : public Command2
{
public:
    MaterialConfigModify(DAVA::NMaterial* material, int id, const DAVA::String& text = DAVA::String());
    ~MaterialConfigModify();

    DAVA::NMaterial* GetMaterial() const;
    DAVA::Entity* GetEntity() const override;

protected:
    DAVA::NMaterial* material;
};

class MaterialChangeCurrentConfig : public MaterialConfigModify
{
public:
    MaterialChangeCurrentConfig(DAVA::NMaterial* material, DAVA::uint32 newCurrentConfigIndex);

    void Undo() override;
    void Redo() override;

private:
    DAVA::uint32 newCurrentConfig = static_cast<DAVA::uint32>(-1);
    DAVA::uint32 oldCurrentConfig = static_cast<DAVA::uint32>(-1);
};

class MaterialRemoveConfig : public MaterialConfigModify
{
public:
    MaterialRemoveConfig(DAVA::NMaterial* material, DAVA::uint32 configIndex);

    void Undo() override;
    void Redo() override;

private:
    DAVA::MaterialConfig config;
    DAVA::uint32 configIndex;
    std::unique_ptr<MaterialChangeCurrentConfig> changeCurrentConfigCommand;
};

class MaterialCreateConfig : public MaterialConfigModify
{
public:
    MaterialCreateConfig(DAVA::NMaterial* material, const DAVA::MaterialConfig& config);

    void Undo() override;
    void Redo() override;

private:
    DAVA::MaterialConfig config;
    DAVA::uint32 configIndex = static_cast<DAVA::uint32>(-1);
    std::unique_ptr<MaterialChangeCurrentConfig> changeCurrentConfigCommand;
};

inline DAVA::NMaterial* MaterialConfigModify::GetMaterial() const
{
    return material;
}

inline DAVA::Entity* MaterialConfigModify::GetEntity() const
{
    return nullptr;
}


#endif // __RESOURCEEDITOR_MATERIALCONFIGCOMMANDS_H__