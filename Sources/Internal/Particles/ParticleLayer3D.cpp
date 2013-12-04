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



#include "Particles/ParticleLayer3D.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"
#include "Render/Material.h"
#include "Math/MathHelpers.h"
#include "Render/Highlevel/Camera.h"
#include "ParticleEmitter3D.h"

namespace DAVA
{

Vector<uint16> ParticleLayer3D::indices;

ParticleLayer3D::ParticleLayer3D(ParticleEmitter* parent)
{
	isLong = false;
	renderData = new RenderDataObject();
	this->emitter = parent;

	//TODO: set material from outside
	
	
	Material *material = new Material();
	material->SetType(Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED);	
	material->SetAlphablend(true);
	material->SetBlendSrc(BLEND_SRC_ALPHA);
	material->SetBlendDest(BLEND_ONE);
	material->SetName("ParticleLayer3D_material");
	material->SetFog(true);
	material->SetTwoSided(true); //as particles can now be not camera facing
	
	

	renderBatch->SetMaterial(material);
	
	renderBatch->SetIndices(&indices);
	
	renderBatch->SetRenderDataObject(renderData);

	SafeRelease(material);
}

ParticleLayer3D::~ParticleLayer3D()
{
	SafeRelease(renderData);
}

void ParticleLayer3D::Draw(Camera * camera)
{
	//render data are now prepared explicitly
	//DrawLayer(camera);	
}

void ParticleLayer3D::PrepareRenderData(Camera* camera)
{
	AABBox3 bbox;
	if (emitter->GetWorldTransformPtr())
	{
		Vector3 emmiterPos = emitter->GetWorldTransformPtr()->GetTranslationVector();
		bbox = AABBox3(emmiterPos, emmiterPos);
	}	

	// Yuri Coder, 2013/06/07. Don't draw SuperEmitter layers - see pls DF-1251 for details.
	if (!sprite || type == TYPE_SUPEREMITTER_PARTICLES)
	{		
		//build bounding box as sum of inner particle emitters bboxes
		if (type == TYPE_SUPEREMITTER_PARTICLES)
		{
			Particle *current = head;
			while (current)
			{
				bbox.AddAABBox(current->GetInnerEmitter()->GetBoundingBox());
				current=current->next;
			}
		}
		
		renderBatch->SetLayerBoundingBox(bbox);
		renderBatch->SetTotalCount(0);		
		return;
	}


	Matrix4 mv;
	Matrix3 rotation;
	bool worldAlign = particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_WORLD_ALIGN;
	if (!worldAlign)
	{
		rotation = emitter->GetRotationMatrix();
	}
	Vector<std::pair<Vector3, Vector3> > basises;
	if (particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_CAMERA_FACING)
	{
		mv = camera->GetMatrix();
		basises.push_back(std::pair<Vector3, Vector3>(Vector3(mv._01, mv._11, mv._21), Vector3(mv._00, mv._10, mv._20)));
	}
	if (particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_X_FACING)
	{
		Vector3 up(0,1,0);
		Vector3 left(0,0,1);
		if (!worldAlign)
		{
			up = up*rotation;
			up.Normalize();
			left = left*rotation;
			left.Normalize();
		}
		basises.push_back(std::pair<Vector3, Vector3>(up, left));
	}
	if (particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_Y_FACING)
	{
		Vector3 up(0,0,1);
		Vector3 left(1,0,0);
		if (!worldAlign)
		{
			up = up*rotation;
			up.Normalize();
			left = left*rotation;
			left.Normalize();
		}
		basises.push_back(std::pair<Vector3, Vector3>(up, left));
	}
	if (particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_Z_FACING)
	{
		Vector3 up(0,1,0);
		Vector3 left(1,0,0);
		if (!worldAlign)
		{
			up = up*rotation;
			up.Normalize();
			left = left*rotation;
			left.Normalize();
		}
		basises.push_back(std::pair<Vector3, Vector3>(up, left));
	}
	
	int32 planesCount =  basises.size();

	direction = camera->GetDirection();

	verts.clear();
	textures.clear();
	colors.clear();

	if (enableFrameBlend)
	{
		textures2.clear();
		times.clear();
	}
	int32 totalCount = 0;

	// Reserve the memory for vectors to avoid the resize operations. Actually there can be less than count
	// particles (for Single Particle or Superemitter one), but never more than count.
	static const int32 POINTS_PER_PARTICLE = 4;
	static const int32 INDICES_PER_PARTICLE = 6;
	verts.resize(count * POINTS_PER_PARTICLE * 3 * planesCount); // 4 vertices per each particle, 3 coords per vertex.
	textures.resize(count * POINTS_PER_PARTICLE * 2 * planesCount); // 4 texture coords per particle, 2 values per texture coord.
	colors.resize(count * POINTS_PER_PARTICLE*planesCount);	
	
	//frame blending
	if (enableFrameBlend)
	{
		textures2.resize(count * POINTS_PER_PARTICLE * 2 * planesCount);
		times.resize(count * POINTS_PER_PARTICLE * planesCount); //single time value per vertex
	}

	Particle * current = head;
	if(current)
	{
		renderBatch->GetMaterial()->GetRenderState()->SetTexture(sprite->GetTexture(current->frame));
	}

	int32 verticesCount = 0;
	int32 texturesCount = 0;
	int32 texturesCount2 = 0;
	int32 timesCount = 0;
	int32 colorsCount = 0;	
	while(current != 0)
	{		
		for (int32 i=0; i<planesCount; ++i)
		{		
			_up = basises[i].first;
			_left = basises[i].second;
			Vector3 topRight;
			Vector3 topLeft;
			Vector3 botRight;
			Vector3 botLeft;

			if (IsLong())
			{
				CalcLong(current, topLeft, topRight, botLeft, botRight);
			}
			else
			{
				CalcNonLong(current, topLeft, topRight, botLeft, botRight);
			}

			verts[verticesCount] = topLeft.x;//0
			verticesCount ++;
			verts[verticesCount] = topLeft.y;
			verticesCount ++;
			verts[verticesCount] = topLeft.z;
			verticesCount ++;

			verts[verticesCount] = topRight.x;//1
			verticesCount ++;
			verts[verticesCount] = topRight.y;
			verticesCount ++;
			verts[verticesCount] = topRight.z;
			verticesCount ++;

			verts[verticesCount] = botLeft.x;//2
			verticesCount ++;
			verts[verticesCount] = botLeft.y;
			verticesCount ++;
			verts[verticesCount] = botLeft.z;
			verticesCount ++;

			verts[verticesCount] = botRight.x;//3
			verticesCount ++;
			verts[verticesCount] = botRight.y;
			verticesCount ++;
			verts[verticesCount] = botRight.z;
			verticesCount ++;

			bbox.AddPoint(topLeft);
			bbox.AddPoint(topRight);
			bbox.AddPoint(botLeft);
			bbox.AddPoint(botRight);

			float32 *pT = sprite->GetTextureVerts(current->frame);

			textures[texturesCount] = pT[0];
			texturesCount ++;
			textures[texturesCount] = pT[1];
			texturesCount ++;

			textures[texturesCount] = pT[2];
			texturesCount ++;
			textures[texturesCount] = pT[3];
			texturesCount ++;

			textures[texturesCount] = pT[4];
			texturesCount ++;
			textures[texturesCount] = pT[5];
			texturesCount ++;

			textures[texturesCount] = pT[6];
			texturesCount ++;
			textures[texturesCount] = pT[7];
			texturesCount ++;

			//frame blending
			if (enableFrameBlend)
			{
				int32 nextFrame = current->frame+1;
				if (nextFrame >= sprite->GetFrameCount())
				{
					if (loopSpriteAnimation)
						nextFrame = 0;
					else
						nextFrame = sprite->GetFrameCount()-1;
					
				}
				pT = sprite->GetTextureVerts(nextFrame);
				textures2[texturesCount2] = pT[0];
				texturesCount2 ++;
				textures2[texturesCount2] = pT[1];
				texturesCount2 ++;

				textures2[texturesCount2] = pT[2];
				texturesCount2 ++;
				textures2[texturesCount2] = pT[3];
				texturesCount2 ++;

				textures2[texturesCount2] = pT[4];
				texturesCount2 ++;
				textures2[texturesCount2] = pT[5];
				texturesCount2 ++;

				textures2[texturesCount2] = pT[6];
				texturesCount2 ++;
				textures2[texturesCount2] = pT[7];
				texturesCount2 ++;
				

				times[timesCount] = current->animTime;
				timesCount++;
				times[timesCount] = current->animTime;
				timesCount++;
				times[timesCount] = current->animTime;
				timesCount++;
				times[timesCount] = current->animTime;
				timesCount++;
			}
			

			// Yuri Coder, 2013/04/03. Need to use drawColor here instead of just colot
			// to take colorOverlife property into account.
			uint32 color = (((uint32)(current->drawColor.a*255.f))<<24) |  (((uint32)(current->drawColor.b*255.f))<<16) |
				(((uint32)(current->drawColor.g*255.f))<<8) | ((uint32)(current->drawColor.r*255.f));
			for(int32 i = 0; i < POINTS_PER_PARTICLE; ++i)
			{
				colors[i + colorsCount] = color;
			}
			colorsCount += POINTS_PER_PARTICLE;

			totalCount++;
		}
		current = TYPE_PARTICLES == type ? current->next : 0;
	}	
	int indexCount = indices.size()/INDICES_PER_PARTICLE;
	if (totalCount>indexCount)
	{
		indices.resize(totalCount*INDICES_PER_PARTICLE);
		for (;indexCount<totalCount; ++indexCount)
		{
			indices[indexCount*INDICES_PER_PARTICLE+0] = indexCount*POINTS_PER_PARTICLE+0;
			indices[indexCount*INDICES_PER_PARTICLE+1] = indexCount*POINTS_PER_PARTICLE+1;
			indices[indexCount*INDICES_PER_PARTICLE+2] = indexCount*POINTS_PER_PARTICLE+2;
			indices[indexCount*INDICES_PER_PARTICLE+3] = indexCount*POINTS_PER_PARTICLE+2;
			indices[indexCount*INDICES_PER_PARTICLE+4] = indexCount*POINTS_PER_PARTICLE+1;
			indices[indexCount*INDICES_PER_PARTICLE+5] = indexCount*POINTS_PER_PARTICLE+3; //preserve order
		}
	}

	renderBatch->SetTotalCount(totalCount);	
	renderBatch->SetLayerBoundingBox(bbox);
	if(totalCount > 0)
	{					
		renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &verts.front());
		renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textures.front());
		renderData->SetStream(EVF_COLOR, TYPE_UNSIGNED_BYTE, 4, 0, &colors.front());				
		if (enableFrameBlend)
		{
			renderData->SetStream(EVF_TEXCOORD1, TYPE_FLOAT, 2, 0, &textures2.front());
			renderData->SetStream(EVF_TIME, TYPE_FLOAT, 1, 0, &times.front());
		}				
	}
	
}

void ParticleLayer3D::CalcNonLong(Particle* current,
								  Vector3& topLeft,
								  Vector3& topRight,
								  Vector3& botLeft,
								  Vector3& botRight)
{
	Vector3 dx(_left);
	Vector3 dy(_up);

	float32 sine;
	float32 cosine;
	SinCosFast(current->angle, sine, cosine);

	// Draw pivot point is Sprite center + layer pivot point.
	Vector2 drawPivotPoint = GetDrawPivotPoint();

	float32 pivotRight = ((sprite->GetWidth()-drawPivotPoint.x)*current->size.x*current->sizeOverLife.x)/2.f;
	float32 pivotLeft = (drawPivotPoint.x*current->size.x*current->sizeOverLife.x)/2.f;
	float32 pivotUp = (drawPivotPoint.y*current->size.y*current->sizeOverLife.y)/2.f;
	float32 pivotDown = ((sprite->GetHeight()-drawPivotPoint.y)*current->size.y*current->sizeOverLife.y)/2.f;

	Vector3 dxc = dx*cosine;
	Vector3 dxs = dx*sine;
	Vector3 dyc = dy*cosine;
	Vector3 dys = dy*sine;

	// Apply offset to the current position according to the emitter position.
	UpdateCurrentParticlePosition(current);

	topLeft = currentParticlePosition+(dxs+dyc)*pivotLeft + (dxc-dys)*pivotDown;
	topRight = currentParticlePosition+(-dxc+dys)*pivotUp + (dxs+dyc)*pivotLeft;
	botLeft = currentParticlePosition+(dxc-dys)*pivotDown + (-dxs-dyc)*pivotRight;
	botRight = currentParticlePosition+(-dxs-dyc)*pivotRight + (-dxc+dys)*pivotUp;
}

void ParticleLayer3D::CalcLong(Particle* current,
							   Vector3& topLeft,
							   Vector3& topRight,
							   Vector3& botLeft,
							   Vector3& botRight)
{

	Vector3 currDirection;
	Particle* parent = emitter->GetParentParticle();		
	if ((NULL != parent)&&inheritPosition)
	{		
		currDirection = current->speed*current->velocityOverLife + parent->speed*parent->velocityOverLife;		
	}else
	{
		currDirection = current->speed;
	}
	currDirection.Normalize();

	Vector3 vecShort = currDirection.CrossProduct(direction);
	vecShort.Normalize();			
	//Vector3 vecLong = vecShort.CrossProduct(direction);
	Vector3 vecLong = -currDirection*(scaleVelocityBase+scaleVelocityFactor*current->speed);
	vecShort /= 2.f;	

	float32 widthDiv2 = sprite->GetWidth()*current->size.x*current->sizeOverLife.x/2;
	float32 heightDiv2 = sprite->GetHeight()*current->size.y*current->sizeOverLife.y/2;

	// Apply offset to the current position according to the emitter position.
	UpdateCurrentParticlePosition(current);

	topRight = currentParticlePosition + widthDiv2*vecShort - heightDiv2/2*vecLong;
	topLeft = currentParticlePosition - widthDiv2*vecShort - heightDiv2/2*vecLong;
	botRight = topRight + heightDiv2*vecLong;
	botLeft = topLeft + heightDiv2*vecLong;
}


ParticleLayer * ParticleLayer3D::Clone(ParticleLayer * dstLayer /*= 0*/)
{
	if(!dstLayer)
	{
		// YuriCoder, 2013/04/30. TODO - this part isn't supposed to work, since
		// dstLayer is always NULL here. Return to it later.
		ParticleEmitter* parentFor3DLayer = NULL;
		if (dynamic_cast<ParticleLayer3D*>(dstLayer))
		{
			parentFor3DLayer = (dynamic_cast<ParticleLayer3D*>(dstLayer))->GetParent();
		}

		ParticleLayer3D *dst = new ParticleLayer3D(parentFor3DLayer);		
		GetMaterial()->Clone(dst->GetMaterial());
		dstLayer = dst; 
		dstLayer->SetLong(this->isLong);
	}

	ParticleLayer::Clone(dstLayer);

	return dstLayer;
}

Material * ParticleLayer3D::GetMaterial()
{
	return renderBatch->GetMaterial();
}

void ParticleLayer3D::SetBlendMode(eBlendMode sFactor, eBlendMode dFactor)
{
	ParticleLayer::SetBlendMode(sFactor, dFactor);
	renderBatch->GetMaterial()->SetBlendSrc(srcBlendFactor);
	renderBatch->GetMaterial()->SetBlendDest(dstBlendFactor);
}

void ParticleLayer3D::SetFog(bool enable)
{
	ParticleLayer::SetFog(enable);
	renderBatch->GetMaterial()->SetFog(enableFog);
}

void ParticleLayer3D::SetFrameBlend(bool enable)
{
	if (enableFrameBlend==enable) return; 
	ParticleLayer::SetFrameBlend(enable);
	if (enableFrameBlend)
	{
		renderBatch->GetMaterial()->SetType(Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED_FRAME_BLEND);			
	}
	else
	{
		renderBatch->GetMaterial()->SetType(Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED);	
		renderData->RemoveStream(EVF_TEXCOORD1);
		renderData->RemoveStream(EVF_TIME);
		textures2.resize(0);
		times.resize(0);		
	}
}


bool ParticleLayer3D::IsLong()
{
	return isLong;
}

void ParticleLayer3D::SetLong(bool value)
{
	isLong = value;
	//renderBatch->GetMaterial()->SetTwoSided(isLong); now materials for particles are always to_sided
	if(innerEmitter)
	{
		innerEmitter->SetLongToAllLayers(value);
	}
}

void ParticleLayer3D::UpdateCurrentParticlePosition(Particle* particle)
{
	if (this->emitter)
	{
		// For Superemitter adjust the particle position according to the
		// current emitter position.
		if (inheritPosition)
			this->currentParticlePosition = particle->position + (emitter->GetPosition() - emitter->GetInitialTranslationVector());
		else
			this->currentParticlePosition = particle->position;
	}
	else
	{
		// For all other types just leave the particle position untouched.
		this->currentParticlePosition = particle->position;
	}
}

void ParticleLayer3D::CreateInnerEmitter()
{
	SafeRelease(this->innerEmitter);
	this->innerEmitter = new ParticleEmitter3D();
}

};
