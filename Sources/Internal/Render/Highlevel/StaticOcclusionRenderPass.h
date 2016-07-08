#ifndef __DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__
#define __DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"

namespace DAVA
{
// use only for debug purposes
// enabling this will save each rendered frame to documents folder
#define SAVE_OCCLUSION_IMAGES 0

struct StaticOcclusionFrameResult;
class StaticOcclusionData;

class StaticOcclusionRenderPass : public RenderPass
{
public:
    StaticOcclusionRenderPass(const FastName& name);
    ~StaticOcclusionRenderPass();

    void DrawOcclusionFrame(RenderSystem* renderSystem, Camera* occlusionCamera,
                            StaticOcclusionFrameResult& target, const StaticOcclusionData&, uint32 blockIndex);

private:
    bool ShouldEnableDepthWriteForRenderObject(RenderObject*);

private:
    enum RenderBatchDepthOption : uint32
    {
        Option_DepthWriteDisabled = 0,
        Option_DepthWriteEnabled = 1
    };
    using RenderBatchWithDepthOption = std::pair<RenderBatch*, RenderBatchDepthOption>;

    rhi::HTexture colorBuffer;
    rhi::HTexture depthBuffer;
    rhi::HDepthStencilState depthWriteStateState[2];
    Vector<RenderBatch*> terrainBatches;
    UnorderedMap<RenderObject*, bool> switchRenderObjects;
    Vector<RenderBatchWithDepthOption> meshBatchesWithDepthWriteOption;
};
};

#endif //__DAVAENGINE_STATIC_OCCLUSION_RENDER_PASS__
