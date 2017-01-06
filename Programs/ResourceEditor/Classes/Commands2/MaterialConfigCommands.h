#ifndef __RESOURCEEDITOR_MATERIALCONFIGCOMMANDS_H__
#define __RESOURCEEDITOR_MATERIALCONFIGCOMMANDS_H__

#include "Commands2/Base/RECommand.h"

#include "Render/Material/NMaterial.h"

class MaterialConfigModify : public RECommand
{
public:
    MaterialConfigModify(DAVA::NMaterial* material, int id, const DAVA::String& text = DAVA::String());
    ~MaterialConfigModify();

    DAVA::NMaterial* GetMaterial() const;
    DAVA::Entity* GetEntity() const;

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