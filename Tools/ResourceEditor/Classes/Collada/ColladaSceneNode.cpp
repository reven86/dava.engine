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


#include "stdafx.h"
#include "ColladaSceneNode.h"
#include <cmath>

namespace DAVA
{

Matrix4 ConvertMatrix( FMMatrix44 & matrix )
{
	Matrix4 result;
	for (int k = 0; k < 4; ++k)
		for (int l = 0; l < 4; ++l)
			result._data[k][l] = matrix.m[k][l];;
	return result;
}
	
Matrix4 ConvertMatrixT( FMMatrix44 & matrix )
{
	Matrix4 result;
	for (int k = 0; k < 4; ++k)
		for (int l = 0; l < 4; ++l)
			result._data[k][l] = matrix.m[l][k];;
	return result;
}


ColladaSceneNode::ColladaSceneNode(ColladaScene * _scene, FCDSceneNode * _node)
{
	isJoint = false;
	originalNode = _node;
	localTransform.Identity();
	parent = 0;
	animation = 0;
	scene = _scene;
	inverse0.Identity();
}

ColladaSceneNode::~ColladaSceneNode()
{

}


void ColladaSceneNode::AddNode( ColladaSceneNode * node )
{
	childs.push_back(node);
	node->parent = this;
}

void ColladaSceneNode::AddMeshInstance( ColladaMeshInstance * meshInstance )
{
	meshInstances.push_back(meshInstance);
}

void ColladaSceneNode::PreProcessLights(ColladaLightState & state)
{
	Matrix4 localTransformTransposed = localTransform;

	glPushMatrix();
	glMultMatrixf(localTransformTransposed.data);
		

	for (int lightIndex = 0; lightIndex < (int)lights.size(); ++lightIndex)
	{
		ColladaLight * light = lights[lightIndex];		
		light->ApplyLight(state);
	}

	for (int c = 0; c < (int)childs.size(); ++c)
		childs[c]->PreProcessLights(state);
	
	glPopMatrix();
}

void ColladaSceneNode::Render(Matrix4 currentMatrix)
{
#ifdef COLLADA_GLUT_RENDER
	worldTransform =  localTransform * currentMatrix;
	//Matrix4 worldTransformCopy = worldTransform;
	//worldTransformCopy.Transpose(); */

	Matrix4 localTransformTransposed = localTransform;
	//localTransformTransposed.Transpose();

	glPushMatrix();
	glMultMatrixf(worldTransform.data);

	if (isJoint)
	{
		glDisable(GL_LIGHTING);
		glColor3f(0.984375, 0.078125, 0.64453125);
		//glutWireCube(0.5f);
		glEnable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 1.0f);
	}

	for (int m = 0; m < (int)meshInstances.size(); ++m)
	{
		meshInstances[m]->Render();
	}
	glPopMatrix();

	for (int c = 0; c < (int)childs.size(); ++c)
		childs[c]->Render(worldTransform);

#endif
}

void ColladaSceneNode::AddLight( ColladaLight * light )
{
	lights.push_back(light);
}

void ColladaSceneNode::MarkJoint()
{
	isJoint = true;	
}

void ColladaSceneNode::UpdateTransforms( float time )
{
	/*FMMatrix44 colladaLocalMatrix;
	colladaLocalMatrix = FMMatrix44::Identity;// = FMMatrix44::Identity(); 

	for (int t = 0; t < originalNode->GetTransformCount(); ++t)
	{
		FCDTransform * transform = originalNode->GetTransform(t);
		if (transform->IsAnimated()) // process all animations to make CalculateWorldTransform work
		{
			FCDAnimated * animated = transform->GetAnimated();
			animated->Evaluate(time);
		}
		
		if (transform->GetType() == FCDTransform::TRANSLATION)
		{
			FCDTTranslation * translation = dynamic_cast<FCDTTranslation*>(transform);
			FMVector3 point = FMVector3(0.0f, 0.0f, 0.0f);
			point = translation->GetTranslation();
			if (transform->IsAnimated()) 
			{
				FCDAnimationCurve* curve;

				// look for x animation
				curve = transform->GetAnimated()->FindCurve(".X");
				if (curve != 0) 
					point.x = curve->Evaluate(time);

				// look for y animation
				curve = transform->GetAnimated()->FindCurve(".Y");
				if (curve != 0) 
					point.y = curve->Evaluate(time);

				// look for z animation
				curve = transform->GetAnimated()->FindCurve(".Z");
				if (curve != 0) 
					point.z = curve->Evaluate(time);
			}
			colladaLocalMatrix = colladaLocalMatrix * FMMatrix44::TranslationMatrix(point);
		}else if (transform->GetType() == FCDTransform::ROTATION)
		{
			FCDTRotation * rotation = dynamic_cast<FCDTRotation*>(transform);
			FMVector3 axis = FMVector3(0.0f, 0.0f, 0.0f);
			float angle = 0;
			axis = rotation->GetAxis();
			angle = rotation->GetAngle();

			if (rotation->IsAnimated()) 
			{
				FCDAnimationCurve* curve;

				// look for x animation
				curve = rotation->GetAnimated()->FindCurve(".X");
				if (curve != 0) 
					axis.x = curve->Evaluate(time);

				// look for y animation
				curve = rotation->GetAnimated()->FindCurve(".Y");
				if (curve != 0) 
					axis.y = curve->Evaluate(time);

				// look for z animation
				curve = rotation->GetAnimated()->FindCurve(".Z");
				if (curve != 0) 
					axis.z = curve->Evaluate(time);

				// look for z animation
				curve = rotation->GetAnimated()->FindCurve(".ANGLE");
				if (curve != 0) 
					angle = curve->Evaluate(time);
			}
			colladaLocalMatrix = colladaLocalMatrix * FMMatrix44::AxisRotationMatrix(axis, angle * PI / 180.0f);
		}else
		{
			colladaLocalMatrix = colladaLocalMatrix * transform->ToMatrix();
		}

	}*/

	//colladaLocalMatrix = originalNode->ToMatrix();
	//localTransform = ConvertMatrix(colladaLocalMatrix);

	if (animation)
	{
		SceneNodeAnimationKey & key = animation->Intepolate(time);
		key.GetMatrix(localTransform);
	}else 
	{
// merged:
		FMMatrix44 colladaLocalMatrix = ColladaSceneNode::CalculateTransformForTime(originalNode, time);
		localTransform = ConvertMatrix(colladaLocalMatrix);
	}
	
	for (int c = 0; c < (int)childs.size(); ++c)
		childs[c]->UpdateTransforms(time); 
}
	
bool ColladaSceneNode::IsAnimated(FCDSceneNode * originalNode)
{
	for (int t = 0; t < (int)originalNode->GetTransformCount(); ++t)
	{
		FCDTransform * transform = originalNode->GetTransform(t);
		if (transform->IsAnimated()) // process all animations to make CalculateWorldTransform work
		{
			return true;
		}
		if (transform->GetType() == FCDTransform::TRANSLATION)
		{
			FCDTTranslation * translation = dynamic_cast<FCDTTranslation*>(transform);
			if (translation->IsAnimated()) 
			{
				return true;
			}
		}else if (transform->GetType() == FCDTransform::ROTATION)
		{
			FCDTRotation * rotation = dynamic_cast<FCDTRotation*>(transform);
			if (rotation->IsAnimated()) 
			{
				return true;
			}
		}
	}
	return false;
}
	
FMMatrix44 ColladaSceneNode::CalculateTransformForTime(FCDSceneNode * originalNode, float32 time)
{
	FMMatrix44 colladaLocalMatrix;
	colladaLocalMatrix = FMMatrix44::Identity;// = FMMatrix44::Identity(); 
	
	for (int t = 0; t < (int)originalNode->GetTransformCount(); ++t)
	{
		FCDTransform * transform = originalNode->GetTransform(t);
		if (transform->IsAnimated()) // process all animations to make CalculateWorldTransform work
		{
			FCDAnimated * animated = transform->GetAnimated();
			animated->Evaluate(time);
		}
		
		if (transform->GetType() == FCDTransform::TRANSLATION)
		{
			FCDTTranslation * translation = dynamic_cast<FCDTTranslation*>(transform);
			FMVector3 point = FMVector3(0.0f, 0.0f, 0.0f);
			point = translation->GetTranslation();
			if (transform->IsAnimated()) 
			{
				FCDAnimationCurve* curve;
				
				// look for x animation
				curve = transform->GetAnimated()->FindCurve(".X");
				if (curve != 0) 
					point.x = curve->Evaluate(time);
				// look for y animation
				curve = transform->GetAnimated()->FindCurve(".Y");
				if (curve != 0) 
					point.y = curve->Evaluate(time);
						
				// look for z animation
				curve = transform->GetAnimated()->FindCurve(".Z");
				if (curve != 0) 
					point.z = curve->Evaluate(time);
			}
			colladaLocalMatrix = colladaLocalMatrix * FMMatrix44::TranslationMatrix(point);
		}else if (transform->GetType() == FCDTransform::ROTATION)
		{
			FCDTRotation * rotation = dynamic_cast<FCDTRotation*>(transform);
			FMVector3 axis = FMVector3(0.0f, 0.0f, 0.0f);
			float angle = 0;
			axis = rotation->GetAxis();
			angle = rotation->GetAngle();
			
			if (rotation->IsAnimated()) 
			{
				FCDAnimationCurve* curve;
				
				// look for x animation
				curve = rotation->GetAnimated()->FindCurve(".X");
				if (curve != 0) 
					axis.x = curve->Evaluate(time);
					
				// look for y animation
				curve = rotation->GetAnimated()->FindCurve(".Y");
				if (curve != 0) 
					axis.y = curve->Evaluate(time);
						
				// look for z animation
				curve = rotation->GetAnimated()->FindCurve(".Z");
				if (curve != 0) 
					axis.z = curve->Evaluate(time);
							
							// look for z animation
				curve = rotation->GetAnimated()->FindCurve(".ANGLE");
				if (curve != 0) 
					angle = curve->Evaluate(time);
			}
			colladaLocalMatrix = colladaLocalMatrix * FMMatrix44::AxisRotationMatrix(axis, angle * PI / 180.0f);
		}else if (transform->GetType() == FCDTransform::SCALE)
		{
			FCDTScale * scaleTransform = dynamic_cast<FCDTScale*>(transform);
			FMVector3 scale = FMVector3(1.0f, 1.0f, 1.0f);
			scale = scaleTransform->GetScale();

			if (scaleTransform->IsAnimated()) 
			{
				FCDAnimationCurve* curve;

				// look for x animation
				curve = scaleTransform->GetAnimated()->FindCurve(".X");
				if (curve != 0) 
					scale.x = curve->Evaluate(time);

				// look for y animation
				curve = scaleTransform->GetAnimated()->FindCurve(".Y");
				if (curve != 0) 
					scale.y = curve->Evaluate(time);

				// look for z animation
				curve = scaleTransform->GetAnimated()->FindCurve(".Z");
				if (curve != 0) 
					scale.z = curve->Evaluate(time);
			}
			colladaLocalMatrix = colladaLocalMatrix * FMMatrix44::ScaleMatrix(scale);
		}else if (transform->GetType() == FCDTransform::MATRIX)
		{
			FCDTMatrix * matrixTransform = dynamic_cast<FCDTMatrix*>(transform);
			FMMatrix44 matrix = transform->ToMatrix();

			if (matrixTransform->IsAnimated()) 
			{
				FCDAnimationCurve* curve;

				FCDAnimated* animated = matrixTransform->GetAnimated();

				for (int32 i = 0; i < 4; ++i)
					for (int32 j = 0; j < 4; ++j)
					{
						curve = animated->FindCurve(Format("(%i)(%i)", i, j).c_str());
						if (curve != 0) 
							matrix.m[i][j] = curve->Evaluate(time);
					}
			}
			colladaLocalMatrix = colladaLocalMatrix * matrix;
		}else
		{
			colladaLocalMatrix = colladaLocalMatrix * transform->ToMatrix();
		}
		
	}
	return colladaLocalMatrix;
}

SceneNodeAnimationKey ColladaSceneNode::ExportAnimationKey(FCDSceneNode * originalNode, float32 time)
{
	SceneNodeAnimationKey key;
	FMMatrix44 colladaLocalMatrix =  ColladaSceneNode::CalculateTransformForTime(originalNode, time);
	Matrix4 lt = ConvertMatrix(colladaLocalMatrix);
	key.time = time;
	lt.Decomposition(key.translation, key.scale, key.rotation);

	return key;
}

bool ColladaSceneNode::KeyTimeEqual(float32 first, float32 second)
{
	return fabsf(first - second) <= EPSILON;
}

SceneNodeAnimation * ColladaSceneNode::ExportNodeAnimation(FCDSceneNode * originalNode, float32 startTime, float32 endTime, float32 fps)
{
	if (!IsAnimated(originalNode))return 0;
		
	
	Vector< float32 > keyTimes;
	// collect animation key times
	for (int transformIndex = 0; transformIndex < (int)originalNode->GetTransformCount(); ++transformIndex)
	{
		FCDTransform * transform = originalNode->GetTransform(transformIndex);
		if (transform->IsAnimated())
		{
			if ((transform->GetType() == FCDTransform::MATRIX) && (originalNode->GetTransformCount() > 1))
			{
				DVASSERT_MSG(false, "Multiple matrix animations are not supported.");
				return NULL;
			}

			FCDAnimated * animated = transform->GetAnimated();
			
			const FCDAnimationCurveListList& curves = animated->GetCurves();
			for (FCDAnimationCurveListList::const_iterator curveIter = curves.begin(); curveIter != curves.end(); ++curveIter)
			{
				for (FCDAnimationCurveTrackList::const_iterator curveTrackIter = curveIter->begin(); curveTrackIter != curveIter->end(); ++curveTrackIter)
				{
					for (size_t keyIndex = 0; keyIndex < (*curveTrackIter)->GetKeyCount(); ++keyIndex)
					{
						float32 key = (*curveTrackIter)->GetKey(keyIndex)->input;
						
						if (!std::binary_search(keyTimes.begin(), keyTimes.end(), key))
						{
							keyTimes.insert(std::lower_bound(keyTimes.begin(), keyTimes.end(), key), key);
						}
					}
				}
			}
		}
	}

	Vector< float32 >::iterator last = std::unique(keyTimes.begin(), keyTimes.end(), KeyTimeEqual);
	keyTimes.erase(last, keyTimes.end());
	
	SceneNodeAnimation * anim = new SceneNodeAnimation(keyTimes.size());
	anim->SetDuration(endTime);
	for (int k = 0; k < (int)keyTimes.size(); ++k)
	{
		anim->SetKey(k, ExportAnimationKey(originalNode, keyTimes[k]));
	}
	
	printf("= keys export: keyCount:%ld\n", keyTimes.size());
		 
	return anim;
}

ColladaSceneNode * ColladaSceneNode::FindNode(const fstring & daeId)
{
	if (originalNode->GetDaeId() == daeId)return this;
	else
	{
		for (int k = 0; k < (int)childs.size(); ++k)
		{
			ColladaSceneNode * node = childs[k]->FindNode(daeId);
			if (node != 0)return node;
		}
	}
	return 0;
}

void ColladaSceneNode::SetAnimation(SceneNodeAnimation * _animation, bool recursive)
{
	animation = _animation;
	if (recursive)
	{
		for (int k = 0; k < (int)childs.size(); ++k)
		{
			ColladaSceneNode * node = childs[k];
			node->SetAnimation(_animation, recursive);
		}
	}
}
	
void ColladaSceneNode::AddCamera(ColladaCamera * cam)
{
	cameras.push_back(cam);
}
};
