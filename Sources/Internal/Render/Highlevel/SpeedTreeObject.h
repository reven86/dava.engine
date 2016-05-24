#ifndef __DAVAENGINE_SPEEDTREE_OBJECT_H__
#define __DAVAENGINE_SPEEDTREE_OBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Render/Highlevel/Mesh.h"

namespace DAVA
{
class SpeedTreeUpdateSystem;
class SpeedTreeObject : public RenderObject
{
public:
    SpeedTreeObject();
    virtual ~SpeedTreeObject();

    void RecalcBoundingBox() override;
    RenderObject* Clone(RenderObject* newObject) override;
    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    static bool IsTreeLeafBatch(RenderBatch* batch);

    void BindDynamicParameters(Camera* camera) override;

    void SetSphericalHarmonics(const Vector<Vector3>& coeffs);
    const Vector<Vector3>& GetSphericalHarmonics() const;

    //Interpolate between globally smoothed (0.0) and locally smoothed (1.0) leafs lighting
    void SetLightSmoothing(const float32& smooth);
    const float32& GetLightSmoothing() const;

protected:
    static const FastName FLAG_WIND_ANIMATION;

    AABBox3 CalcBBoxForSpeedTreeGeometry(RenderBatch* rb);

    void SetTreeAnimationParams(const Vector2& trunkOscillationParams, const Vector2& leafOscillationParams);
    void UpdateAnimationFlag(int32 maxAnimatedLod);

    Vector2 trunkOscillation;
    Vector2 leafOscillation;

    Vector<Vector3> sphericalHarmonics;
    float32 lightSmoothing;

public:
    INTROSPECTION_EXTEND(SpeedTreeObject, RenderObject,
                         PROPERTY("lightSmoothing", "Light Smoothing", GetLightSmoothing, SetLightSmoothing, I_SAVE | I_EDIT | I_VIEW)
                         );

    friend class SpeedTreeUpdateSystem;
};
};

#endif // __DAVAENGINE_SPEEDTREE_OBJECT_H__
