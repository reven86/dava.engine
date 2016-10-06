#include "CustomUI3DView.h"
#include "UI/UIControlSystem.h"
#include "Scene3D/Scene.h"
#include "Tests/BaseTest.h"

using namespace DAVA;

CustomUI3DView::CustomUI3DView(BaseTest* baseTest, const DAVA::Rect& rect)
    : UI3DView(rect)
    , baseTest(baseTest)
{
}

void CustomUI3DView::OnVisible()
{
    UI3DView::OnVisible();
    UIControlSystem::Instance()->update.Disconnect(this); // Disconnect original update slot
    UIControlSystem::Instance()->update.Connect(this, &CustomUI3DView::Update); // Connect new update slot
}

void CustomUI3DView::Update(DAVA::float32 timeElapsed)
{
    DVASSERT_MSG(baseTest, "BaseTest ptr corrupted!");
    if (scene)
    {
        scene->Update(baseTest->GetCurrentFrameDelta());
    }
}
