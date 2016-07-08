#include "ParticleForce.h"
using namespace DAVA;

// Particle Force class is needed to store Particle Force data.
ParticleForce::ParticleForce(RefPtr<PropertyLine<Vector3>> force_, RefPtr<PropertyLine<float32>> forceOverLife_)
    : force(force_)
    , forceOverLife(forceOverLife_)
{
}

ParticleForce* ParticleForce::Clone()
{
    ParticleForce* dst = new ParticleForce();
    if (force)
    {
        dst->force = force->Clone();
        dst->force->Release();
    }
    if (forceOverLife)
    {
        dst->forceOverLife = forceOverLife->Clone();
        dst->forceOverLife->Release();
    }
    return dst;
}

void ParticleForce::GetModifableLines(List<ModifiablePropertyLineBase*>& modifiables)
{
    PropertyLineHelper::AddIfModifiable(force.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(forceOverLife.Get(), modifiables);
}
