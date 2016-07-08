#ifndef __EDITOR_MATERIAL_SYSTEM_H__
#define __EDITOR_MATERIAL_SYSTEM_H__

#include "DAVAEngine.h"
#include "Base/Introspection.h"

class Command2;
class EditorMaterialSystem : public DAVA::SceneSystem
{
    friend class SceneEditor2;

public:
    enum MaterialLightViewMode
    {
        LIGHTVIEW_NOTHING = 0x0,

        LIGHTVIEW_ALBEDO = 0x1,
        LIGHTVIEW_AMBIENT = 0x2,
        LIGHTVIEW_DIFFUSE = 0x4,
        LIGHTVIEW_SPECULAR = 0x8,

        LIGHTVIEW_ALL = (LIGHTVIEW_ALBEDO | LIGHTVIEW_AMBIENT | LIGHTVIEW_DIFFUSE | LIGHTVIEW_SPECULAR)
    };

    EditorMaterialSystem(DAVA::Scene* scene);
    ~EditorMaterialSystem();

    const DAVA::Set<DAVA::NMaterial*>& GetTopParents() const;

    DAVA::Entity* GetEntity(DAVA::NMaterial*) const;
    const DAVA::RenderBatch* GetRenderBatch(DAVA::NMaterial*) const;

    void SetLightViewMode(EditorMaterialSystem::MaterialLightViewMode viewMode, bool set);
    bool GetLightViewMode(EditorMaterialSystem::MaterialLightViewMode viewMode) const;

    void SetLightViewMode(int fullViewMode);
    int GetLightViewMode();

    void SetLightmapCanvasVisible(bool enable);
    bool IsLightmapCanvasVisible() const;

    bool HasMaterial(DAVA::NMaterial*) const;

private:
    struct MaterialMapping
    {
        MaterialMapping() = default;
        MaterialMapping(DAVA::Entity* entity_, DAVA::RenderBatch* renderBatch_);

        ~MaterialMapping();

        MaterialMapping(const MaterialMapping& other);
        MaterialMapping(MaterialMapping&& other) = delete;

        MaterialMapping& operator=(const MaterialMapping& other);
        MaterialMapping& operator=(MaterialMapping&& other) = delete;

        DAVA::Entity* entity = nullptr;
        DAVA::RenderBatch* renderBatch = nullptr;
    };
    using MaterialToObjectsMap = DAVA::Map<DAVA::NMaterial*, MaterialMapping>;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void ProcessCommand(const Command2* command, bool redo);

    void AddMaterials(DAVA::Entity* entity);
    void AddMaterial(DAVA::NMaterial*, const MaterialMapping& mapping);

    void RemoveMaterial(DAVA::NMaterial* material);

    void ApplyViewMode();
    void ApplyViewMode(DAVA::NMaterial* material);

    bool IsEditable(DAVA::NMaterial* material) const;

private:
    MaterialToObjectsMap materialToObjectsMap;
    DAVA::Set<DAVA::NMaterial*> ownedParents;
    DAVA::uint32 curViewMode = LIGHTVIEW_ALL;
    bool showLightmapCanvas = false;
};

#endif // __EDITOR_MATERIAL_SYSTEM_H__
