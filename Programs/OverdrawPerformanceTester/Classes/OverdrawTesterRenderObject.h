#pragma once

#include "Base/BaseTypes.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
class NMaterial;
class RenderBatch;
}

namespace OverdrawPerformanceTester
{
class OverdrawTesterRenderObject : public DAVA::RenderObject
{
public:
    OverdrawTesterRenderObject(DAVA::NMaterial* drawMaterial);
    ~OverdrawTesterRenderObject();

    void PrepareToRender(DAVA::Camera* camera) override;
    void RecalculateWorldBoundingBox() override;

private:
    DAVA::uint32 vertexLayoutId;
    DAVA::NMaterial* material;
    DAVA::RenderBatch* batch;
};
}
