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



#include "Particles/ParticleEmitter3D.h"
#include "Particles/ParticleLayer3D.h"
#include "Render/Highlevel/Camera.h"
#include "Utils/Random.h"

namespace DAVA
{

ParticleEmitter3D::ParticleEmitter3D()
{
	is3D = true;
}

void ParticleEmitter3D::AddLayer(ParticleLayer * layer)
{
	// Only ParticleLayer3Ds are allowed on this level.
	if (dynamic_cast<ParticleLayer3D*>(layer))
	{
		ParticleEmitter::AddLayer(layer);
	}
}
	
void ParticleEmitter3D::InsertLayer(ParticleLayer * layer, ParticleLayer * beforeLayer)
{
	// Only ParticleLayer3Ds are allowed on this level.
	if (dynamic_cast<ParticleLayer3D*>(layer))
	{
		ParticleEmitter::InsertLayer(layer, beforeLayer);
	}
}

void ParticleEmitter3D::RenderUpdate(Camera *camera, float32 timeElapsed)
{
	eBlendMode srcMode = RenderManager::Instance()->GetSrcBlend();
	eBlendMode destMode = RenderManager::Instance()->GetDestBlend();

	Draw(camera);
    
	RenderManager::Instance()->SetBlendMode(srcMode, destMode);
}

void ParticleEmitter3D::Draw(Camera * camera)
{
	//Dizz: now layer->Draw is called from ParticleLayerBatch
}

void ParticleEmitter3D::RecalcBoundingBox()
{
	RenderObject::RecalcBoundingBox(); //transpass ParticleEmitter::RecalcBoundingBox()
	if (GetWorldTransformPtr()) //add emmiter anyway
	{
		Vector3 emmiterPos = GetWorldTransformPtr()->GetTranslationVector();
		bbox.AddPoint(emmiterPos);
	}	
}


void ParticleEmitter3D::PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex)
{
	Vector3 tempPosition = Vector3();
	Matrix4 * worldTransformPtr = GetWorldTransformPtr();
	Matrix3 rotationMatrix;
	rotationMatrix.Identity();

	if(worldTransformPtr)
	{
		tempPosition = worldTransformPtr->GetTranslationVector();
		rotationMatrix = Matrix3(*worldTransformPtr);;
	}

	//Vector3 tempPosition = particlesFollow ? Vector3() : position;
    if (emitterType == EMITTER_POINT)
    {
        particle->position = tempPosition;
    }
    else if (emitterType == EMITTER_RECT)
    {        
        float32 rand05_x = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_y = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_z = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        Vector3 lineDirection(0, 0, 0);
        if(size)
            lineDirection = Vector3(size->GetValue(time).x * rand05_x, size->GetValue(time).y * rand05_y, size->GetValue(time).z * rand05_z);		
		//particle->position = tempPosition + lineDirection;
        particle->position = tempPosition + TransformPerserveLength(lineDirection, rotationMatrix);
    }
	else if ((emitterType == EMITTER_ONCIRCLE_VOLUME) ||
			 (emitterType == EMITTER_ONCIRCLE_EDGES))
	{
		CalculateParticlePositionForCircle(particle, tempPosition, rotationMatrix);
	}

    if (emitterType == EMITTER_SHOCKWAVE)
    {
		// For "Shockwave" emitters the calculation is different.
		PrepareEmitterParametersShockwave(particle, velocity, emitIndex, tempPosition, rotationMatrix);
	}
	else
	{
		PrepareEmitterParametersGeneric(particle, velocity, emitIndex, tempPosition, rotationMatrix);
	}

	if(worldTransformPtr)
	{
		Matrix4 newTransform = *worldTransformPtr;
		newTransform._30 = newTransform._31 = newTransform._32 = 0;		
		float32 speedLength = particle->speed.Length();
		
		particle->speed = particle->speed*newTransform;
		float32 speedLengthAfter = particle->speed.Length();
		if (speedLengthAfter)
			particle->speed*=speedLength/speedLengthAfter;
		
	}	
}

void ParticleEmitter3D::CalculateParticlePositionForCircle(Particle* particle, const Vector3& tempPosition,
														   const Matrix3& rotationMatrix)
{
	float32 curRadius = 1.0f;
	if (radius)
	{
		curRadius = radius->GetValue(time);
	}

	float32 curAngle = PI_2 * (float32)Random::Instance()->RandFloat();
	if (emitterType == EMITTER_ONCIRCLE_VOLUME)
	{
		// "Volume" means we have to emit particles from the whole circle.
		curRadius *= (float32)Random::Instance()->RandFloat();
	}

	float sinAngle = 0.0f;
	float cosAngle = 0.0f;
	SinCosFast(curAngle, sinAngle, cosAngle);

	Vector3 directionVector(curRadius * cosAngle,
							curRadius * sinAngle,
							0.0f);
		
	// Rotate the direction vector according to the current emission vector value.
	Vector3 zNormalVector(0.0f, 0.0f, 1.0f);
	Vector3 curEmissionVector;
	if (emissionVector)
	{
		curEmissionVector = emissionVector->GetValue(time);
		curEmissionVector.Normalize();
	}
		
	// This code rotates the (XY) plane with the particles to the direction vector.
	// Taking into account that a normal vector to the (XY) plane is (0,0,1) this
	// code is very simplified version of the generic "plane rotation" code.
	float32 length = curEmissionVector.Length();
	if (FLOAT_EQUAL(length, 0.0f) == false)
	{
		float32 cosAngleRot = curEmissionVector.z / length;
		float32 angleRot = acos(cosAngleRot);
		Vector3 axisRot(curEmissionVector.y, -curEmissionVector.x, 0);

		Matrix3 planeRotMatrix;
		planeRotMatrix.CreateRotation(axisRot, angleRot);
		Vector3 rotatedVector = directionVector * planeRotMatrix;
		directionVector = rotatedVector;
	}
		
	particle->position = tempPosition + TransformPerserveLength(directionVector, rotationMatrix);	
}
	
void ParticleEmitter3D::PrepareEmitterParametersShockwave(Particle * particle, float32 velocity,
														 int32 emitIndex, const Vector3& tempPosition,
														 const Matrix3& rotationMatrix)
{
	// Emit ponts from the circle in the XY plane.
	float32 curRadius = 1.0f;
	if (radius)
	{
		curRadius = radius->GetValue(time);
	}

	float32 curAngle = PI_2 * (float32)Random::Instance()->RandFloat();
	float sinAngle = 0.0f;
	float cosAngle = 0.0f;
	SinCosFast(curAngle, sinAngle, cosAngle);

	Vector3 directionVector(curRadius * cosAngle,
							curRadius * sinAngle,
							0.0f);

	particle->position = tempPosition + TransformPerserveLength(directionVector, rotationMatrix);	

	

	// Calculate Z value.
	const float32 TANGENT_EPSILON = (float32)(1E-4);
	if (this->emissionRange)
	{
		float32 emissionRangeValue = DegToRad(emissionRange->GetValue(time));
		SinCosFast(emissionRangeValue, sinAngle, cosAngle);
		if (fabs(cosAngle) < TANGENT_EPSILON)
		{
			// Reset the direction vector.
			directionVector.x = 0;
			directionVector.y = 0;
			directionVector.z = -curRadius / 2 + (float32)Random::Instance()->RandFloat() * curRadius;
		}
		else
		{
			float32 zValue = (curRadius * sinAngle / cosAngle);
			directionVector.z = -zValue / 2 + (float32)Random::Instance()->RandFloat() * zValue;
		}
	}
		
	float32 dvl = directionVector.Length();
	if (dvl>EPSILON)
	{
		directionVector*=velocity/dvl;
	}	
	particle->speed = directionVector;

}

void ParticleEmitter3D::PrepareEmitterParametersGeneric(Particle * particle, float32 velocity,
														int32 emitIndex, const Vector3& tempPosition,
														const Matrix3& rotationMatrix)
{
    Vector3 vel = Vector3(1.0f, 0.0f, 0.0f);
    if(emissionVector)
	{
		// Yuri Coder, 2013/04/12. Need to invert the directions in the emission vector, since
		// their coordinates are in the opposite directions for the Particles Editor.        
		vel = emissionVector->GetValue(0) * -1.0f;
	}

    Vector3 rotVect(0, 0, 1);
    float32 phi = PI*2*(float32)Random::Instance()->RandFloat();
    if(vel.x != 0)
    {
        rotVect.y = sinf(phi);
        rotVect.z = cosf(phi);
        rotVect.x = - rotVect.y*vel.y/vel.x - rotVect.z*vel.z/vel.x;
    }
    else if(vel.y != 0)
    {
        rotVect.x = cosf(phi);
        rotVect.z = sinf(phi);
        rotVect.y = - rotVect.z*vel.z/vel.y;
    }
    else if(vel.z != 0)
    {
        rotVect.x = cosf(phi);
        rotVect.y = sinf(phi);
        rotVect.z = 0;
    }
    rotVect.Normalize();
	
    float32 range = 0;
    if(emissionRange)
        range = DegToRad(emissionRange->GetValue(time) + angle);
    float32 rand05 = (float32)Random::Instance()->RandFloat() - 0.5f;
	
    Vector3 q_v(rotVect*sinf(range*rand05/2));
    float32 q_w = cosf(range*rand05/2);
	
    Vector3 q1_v(q_v);
    float32 q1_w = -q_w;
    q1_v /= (q_v.SquareLength() + q_w*q_w);
    q1_w /= (q_v.SquareLength() + q_w*q_w);
	
    Vector3 v_v(vel);
	
    Vector3 qv_v = q_v.CrossProduct(v_v) + q_w*v_v;
    float32 qv_w = - q_v.DotProduct(v_v);
	
    Vector3 qvq1_v = qv_v.CrossProduct(q1_v) + qv_w*q1_v + q1_w*qv_v;
	
	particle->speed = qvq1_v * velocity;
	
	
	// Yuri Coder, 2013/03/26. After discussion with Ivan it appears this angle
	// calculation is incorrect. TODO: return to this code later on.
    
    //particle->angle = atanf(particle->direction.z/particle->direction.x);
}

void ParticleEmitter3D::LoadParticleLayerFromYaml(const YamlNode* yamlNode, bool isLong)
{
	ParticleLayer3D* layer = new ParticleLayer3D(this);
	layer->SetLong(isLong);

	AddLayer(layer);
	layer->LoadFromYaml(configPath, yamlNode);
	SafeRelease(layer);
}

bool ParticleEmitter3D::Is3DFlagCorrect()
{
	// For ParticleEmitter3D is3D flag must be set to TRUE.
	return (is3D == true);
}

RenderObject * ParticleEmitter3D::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<ParticleEmitter3D>(this), "Can clone only ParticleEmitter3D");
		newObject = new ParticleEmitter3D();
	}
	else
	{
		CleanupLayers();
		ReleaseFromCache(static_cast<ParticleEmitter *>(newObject)->emitterFileName);
	}

	ParticleEmitter* clonedEmitter = static_cast<ParticleEmitter*>(newObject);
	clonedEmitter->SetConfigPath(this->configPath);
	clonedEmitter->SetPosition(this->position);
	clonedEmitter->SetAngle(this->angle);
	
	clonedEmitter->SetLifeTime(this->lifeTime);
	clonedEmitter->SetRepeatCount(this->repeatCount);
	clonedEmitter->SetTime(this->time);
	clonedEmitter->SetEmitPointsCount(this->emitPointsCount);
	clonedEmitter->SetPaused(this->isPaused);
	clonedEmitter->SetAutoRestart(this->isAutorestart);
	clonedEmitter->SetParticlesFollow(this->particlesFollow);
	clonedEmitter->Set3D(this->is3D);
	clonedEmitter->SetPlaybackSpeed(this->playbackSpeed);

	clonedEmitter->SetInitialTranslationVector(this->initialTranslationVector);

	if (this->emissionVector)
	{
		clonedEmitter->emissionVector = this->emissionVector->Clone();
        clonedEmitter->emissionVector->Release();
	}
	if (this->emissionAngle)
	{
		clonedEmitter->emissionAngle = this->emissionAngle->Clone();
        clonedEmitter->emissionAngle->Release();
	}
	if (this->emissionRange)
	{
		clonedEmitter->emissionRange = this->emissionRange->Clone();
        clonedEmitter->emissionRange->Release();
	}
	if (this->radius)
	{
		clonedEmitter->radius = this->radius->Clone();
        clonedEmitter->radius->Release();
	}
	if (this->colorOverLife)
	{
		clonedEmitter->colorOverLife = this->colorOverLife->Clone();
        clonedEmitter->colorOverLife->Release();
	}
	if (this->size)
	{
		clonedEmitter->size = this->size->Clone();
        clonedEmitter->size->Release();
	}
	
	clonedEmitter->emitterType = this->emitterType;
	clonedEmitter->currentColor = this->currentColor;
	clonedEmitter->SetShortEffect(shortEffect);
	
	// Now can add Layers. Need to update their parents.
	for (Vector<ParticleLayer*>::iterator iter = this->layers.begin(); iter != this->layers.end();
		 iter ++)
	{
		ParticleLayer* clonedLayer = (*iter)->Clone(NULL);
		ParticleLayer3D* clonedLayer3D = dynamic_cast<ParticleLayer3D*>(clonedLayer);
		if (clonedLayer3D)
		{
			clonedLayer3D->SetParent(clonedEmitter);
		}

		clonedEmitter->AddLayer(clonedLayer);
		SafeRelease(clonedLayer);
	}

	time = 0.0f;
	repeatCount = 0;
	lodLevelLocked = false;
	currentLodLevel = desiredLodLevel;

	clonedEmitter->emitterFileName = emitterFileName;
	RetainInCache(emitterFileName);

	return newObject;
}

}

