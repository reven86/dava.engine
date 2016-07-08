#include "LandscapeEditorSystem.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"

#include "Scene/SceneEditor2.h"
#include "Main/QtUtils.h"

using namespace DAVA;

LandscapeEditorSystem::LandscapeEditorSystem(Scene* scene, const DAVA::FilePath& cursorPathname)
    : SceneSystem(scene)
    , cursorPosition(-100.f, -100.f)
    , prevCursorPos(-1.f, -1.f)
{
    cursorTexture = CreateSingleMipTexture(cursorPathname);
    cursorTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);

    collisionSystem = ((SceneEditor2*)GetScene())->collisionSystem;
    selectionSystem = ((SceneEditor2*)GetScene())->selectionSystem;
    modifSystem = ((SceneEditor2*)GetScene())->modifSystem;
    drawSystem = ((SceneEditor2*)GetScene())->landscapeEditorDrawSystem;
}

LandscapeEditorSystem::~LandscapeEditorSystem()
{
    SafeRelease(cursorTexture);

    collisionSystem = nullptr;
    selectionSystem = nullptr;
    modifSystem = nullptr;
    drawSystem = nullptr;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorSystem::IsCanBeEnabled() const
{
    return drawSystem->VerifyLandscape();
}

bool LandscapeEditorSystem::IsLandscapeEditingEnabled() const
{
    return enabled;
}

void LandscapeEditorSystem::UpdateCursorPosition()
{
    Vector3 landPos;
    isIntersectsLandscape = collisionSystem->LandRayTestFromCamera(landPos);
    if (isIntersectsLandscape)
    {
        landPos.x = (float32)((int32)landPos.x);
        landPos.y = (float32)((int32)landPos.y);

        const AABBox3& box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

        cursorPosition.x = (landPos.x - box.min.x) / (box.max.x - box.min.x);
        cursorPosition.y = (landPos.y - box.min.y) / (box.max.y - box.min.y);
        cursorPosition.x = cursorPosition.x;
        cursorPosition.y = 1.f - cursorPosition.y;

        drawSystem->SetCursorPosition(cursorPosition);
    }
    else
    {
        // hide cursor
        drawSystem->SetCursorPosition(DAVA::Vector2(-100.f, -100.f));
    }
}
