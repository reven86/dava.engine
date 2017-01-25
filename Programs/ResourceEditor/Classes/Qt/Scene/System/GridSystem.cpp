#include "Scene/System/GridSystem.h"
#include "Qt/Settings/SettingsManager.h"

// framework
#include "Render/RenderHelper.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Scene.h"

#define LOWEST_GRID_STEP 0.1f
#define LOWEST_GRID_SIZE 1.0f

SceneGridSystem::SceneGridSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
}

void SceneGridSystem::Draw()
{
    const DAVA::float32 gridStep = SettingsManager::GetValue(Settings::Scene_GridStep).AsFloat();
    const DAVA::float32 gridMax = SettingsManager::GetValue(Settings::Scene_GridSize).AsFloat();

    if (gridStep >= LOWEST_GRID_STEP && gridMax >= LOWEST_GRID_SIZE)
    {
        for (DAVA::float32 x = -gridMax; x <= gridMax; x += gridStep)
        {
            const DAVA::Vector3 v1(x, -gridMax, 0);
            const DAVA::Vector3 v2(x, gridMax, 0);

            const DAVA::Vector3 v3(-gridMax, x, 0);
            const DAVA::Vector3 v4(gridMax, x, 0);

            if (x != 0.0f)
            {
                static const DAVA::Color gridColor(0.4f, 0.4f, 0.4f, 1.0f);

                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v1, v2, gridColor);
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v3, v4, gridColor);
            }
        }

        static const DAVA::Color grid0Color(0.0f, 0.0f, 0.0f, 1.0f);

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(DAVA::Vector3(-gridMax, 0, 0), DAVA::Vector3(gridMax, 0, 0), grid0Color);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(DAVA::Vector3(0, -gridMax, 0), DAVA::Vector3(0, gridMax, 0), grid0Color);
    }
}
