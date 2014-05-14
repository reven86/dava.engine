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


#include "Render/Highlevel/Camera.h"
#include "Render/RenderBase.h"
#include "Core/Core.h"
#include "Render/RenderManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA 
{
    

Camera::Camera()
:	orthoWidth(35.f)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	SetupPerspective(35.0f, 1.0f, 1.0f, 2500.f);
	up = Vector3(0.0f, 1.0f, 0.0f);
	left = Vector3(1.0f, 0.0f, 0.0f);
	flags = REQUIRE_REBUILD | REQUIRE_REBUILD_MODEL | REQUIRE_REBUILD_PROJECTION;
    
	cameraTransform.Identity();
    currentFrustum = new Frustum();
}
	
Camera::~Camera()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	SafeRelease(currentFrustum);
} 

void Camera::SetFOV(const float32 &fovxInDegrees)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	fovX = fovxInDegrees;
	Recalc();
}
    
void Camera::SetAspect(const float32 &_aspect)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	aspect = 1.f/_aspect;
	Recalc();
}
    
void Camera::SetZNear(const float32 &_zNear)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	znear = _zNear;
	Recalc();
}
    
void Camera::SetZFar(const float32 &_zFar)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	zfar = _zFar;
	Recalc();
}

void Camera::SetIsOrtho(const bool &_ortho)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	ortho = _ortho;
	Recalc();
}
 

void Camera::SetOrthoWidth(const float32 &width)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	orthoWidth = width;
	Recalc();
}

float32 Camera::GetOrthoWidth() const
{
    return orthoWidth;
}
    
void Camera::SetXMin(const float32 &_xmin)
{
	xmin = _xmin;
}

void Camera::SetXMax(const float32 &_xmax)
{
	xmax = _xmax;
}

void Camera::SetYMin(const float32 &_ymin)
{
	ymin = _ymin;
}

void Camera::SetYMax(const float32 &_ymax)
{
	ymax = _ymax;
}
    
float32 Camera::GetXMin() const
{
    return xmin;
}

float32 Camera::GetXMax() const
{
    return xmax;
}

float32 Camera::GetYMin() const
{
    return ymin;
}

float32 Camera::GetYMax() const
{
    return ymax;
}

    
float32 Camera::GetFOV() const
{
    return fovX;
}

float32 Camera::GetAspect() const
{
    return 1.f/aspect;
}

float32 Camera::GetZNear() const
{
    return znear;
}

float32 Camera::GetZFar() const
{
    return zfar;
}
    
bool Camera::GetIsOrtho() const
{
    return ortho;
}


void Camera::SetupPerspective(float32 fovxInDegrees, float32 aspectYdivX, float32 zNear, float32 zFar)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    this->aspect = 1.f/aspectYdivX;
    
    this->fovX = fovxInDegrees;
	this->znear = zNear;
	this->zfar = zFar;
	this->ortho = false;
	
	Recalc();
}

void Camera::SetupOrtho(float32 width, float32 aspectYdivX, float32 zNear, float32 zFar)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	this->aspect = 1.f/aspectYdivX;

	this->orthoWidth = width;
	this->znear = zNear;
	this->zfar = zFar;
	this->ortho = true;

	Recalc();
}

void Camera::Setup(float32 _xmin, float32 _xmax, float32 _ymin, float32 _ymax, float32 _znear, float32 _zfar)
{
	xmin = _xmin;
	xmax = _xmax;
	ymin = _ymin;
	ymax = _ymax;
	znear = _znear;
	zfar = _zfar;
}

void Camera::Recalc()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	flags |= REQUIRE_REBUILD_PROJECTION;


	if(ortho)
	{
		xmax = orthoWidth/2.f;
		xmin = -xmax;

		ymax = xmax * aspect;
		ymin = xmin * aspect;
	}
	else
	{
		xmax = znear * tanf(fovX * PI / 360.0f);
		xmin = -xmax;

		ymax = xmax * aspect;
		ymin = xmin * aspect;
	}

    CalculateZoomFactor();
}

Vector2 Camera::GetOnScreenPosition(const Vector3 &forPoint, const Rect & viewport)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	Vector3 v = GetOnScreenPositionAndDepth(forPoint, viewport);
	return Vector2(v.x, v.y);
}

Vector3 Camera::GetOnScreenPositionAndDepth(const Vector3 & forPoint, const Rect & viewport)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	Vector4 pv(forPoint);
	pv = pv * GetViewProjMatrix();
	//    return Vector2((viewport.dx * 0.5f) * (1.f + pv.x/pv.w) + viewport.x
	//                   , (viewport.dy * 0.5f) * (1.f + pv.y/pv.w) + viewport.y);

    return Vector3(((pv.x/pv.w)*0.5f+0.5f)*viewport.dx+viewport.x,
			(1.0f - ((pv.y/pv.w)*0.5f+0.5f))*viewport.dy+viewport.y, pv.w + pv.z);

}

const Matrix4 &Camera::GetViewProjMatrix()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    if (flags & REQUIRE_REBUILD)
    {
        RebuildCameraFromValues();
    }
    if (flags & REQUIRE_REBUILD_PROJECTION)
    {
        RebuildProjectionMatrix();
    }
    if (flags & REQUIRE_REBUILD_MODEL)
    {
        RebuildViewMatrix();
    }
    if (flags & REQUIRE_REBUILD_UNIFORM_PROJ_MODEL)
    {
        viewProjMatrix = viewMatrix * projMatrix;
        flags &= ~REQUIRE_REBUILD_UNIFORM_PROJ_MODEL;
    }
    
    return viewProjMatrix;
}

void Camera::RebuildProjectionMatrix()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    flags &= ~REQUIRE_REBUILD_PROJECTION;
    flags |= REQUIRE_REBUILD_UNIFORM_PROJ_MODEL;
    
    float32 xMinOrientation = xmin;
    float32 xMaxOrientation = xmax;
    float32 yMinOrientation = ymin;
    float32 yMaxOrientation = ymax;
    
    uint32 cullInvert = 0;
    
    if (RenderManager::Instance()->GetRenderOrientation() == Core::SCREEN_ORIENTATION_TEXTURE)
    {
        yMinOrientation = ymax;
        yMaxOrientation = ymin;
        cullInvert = 1 - cullInvert; // Invert once if we render to FBO
    }
    if (flags & INVERT_CULL)
        cullInvert = 1 - cullInvert;    // Invert twice if we want to invert the faces for rendering purpose (for example shadow maps, or water reflection)
    
    // Set correct drawing order according to FBO config + camera requirements
    if (cullInvert == 0)
        RenderManager::Instance()->SetCullOrder(ORDER_CCW);
    else
        RenderManager::Instance()->SetCullOrder(ORDER_CW);
    
    if (!ortho) 
    {
        projMatrix.glFrustum(xMinOrientation, xMaxOrientation, yMinOrientation, yMaxOrientation, znear, zfar);
    }
    else
    {
        projMatrix.glOrtho(xMinOrientation, xMaxOrientation, yMinOrientation, yMaxOrientation, znear, zfar);
    }
}

void Camera::RebuildViewMatrix()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    flags &= ~REQUIRE_REBUILD_MODEL;
    flags |= REQUIRE_REBUILD_UNIFORM_PROJ_MODEL;
    
//	if (RenderManager::Instance()->GetRenderOrientation()==Core::SCREEN_ORIENTATION_TEXTURE)
//	{
//        viewMatrix = Matrix4::IDENTITY;
//        viewMatrix.CreateScale(Vector3(1.0f, -1.0f, 1.0f));
//        viewMatrix = viewMatrix * cameraTransform;
//    }
//    else
//    {
    viewMatrix = cameraTransform;
//    }
}

void Camera::SetPosition(const Vector3 & _position)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	position = _position;
    flags |= REQUIRE_REBUILD;
}

void Camera::SetDirection(const Vector3 & _direction)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	target = position + _direction;
    flags |= REQUIRE_REBUILD;
}

void Camera::SetTarget(const Vector3 & _target)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	target = _target;
    flags |= REQUIRE_REBUILD;
}
    
void Camera::SetUp(const Vector3 & _up)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    up = _up;
    flags |= REQUIRE_REBUILD;
}
    
void Camera::SetLeft(const Vector3 & _left)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    left = _left;
    flags |= REQUIRE_REBUILD;
}

	
const Vector3 & Camera::GetTarget() const
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	return target;
}

const Vector3 & Camera::GetPosition() const
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	return position;
}

const Vector3 & Camera::GetDirection()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    direction = target - position;
    direction.Normalize(); //TODO: normalize only on target/position changes
    return direction;
}

const Vector3 & Camera::GetUp() const
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    return up;
}

const Vector3 & Camera::GetLeft() const
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    return left;
}
    
const Matrix4 & Camera::GetMatrix() const 
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    return viewMatrix;
}

void Camera::RebuildCameraFromValues()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
//    Logger::FrameworkDebug("camera rebuild: pos(%0.2f %0.2f %0.2f) target(%0.2f %0.2f %0.2f) up(%0.2f %0.2f %0.2f)",
//                  position.x, position.y, position.z, target.x, target.y, target.z, up.x, up.y, up.z);
    
    flags &= ~REQUIRE_REBUILD; 
    flags |= REQUIRE_REBUILD_MODEL;
	cameraTransform.BuildLookAtMatrixRH(position, target, up);
    
    // update left vector after rebuild
	left.x = cameraTransform._00;
	left.y = cameraTransform._10;
	left.z = cameraTransform._20;
}
	
void Camera::ExtractCameraToValues()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	position.x = cameraTransform._30;
	position.y = cameraTransform._31;
	position.z = cameraTransform._32;
	left.x = cameraTransform._00;
	left.y = cameraTransform._10;
	left.z = cameraTransform._20;
	up.x = cameraTransform._01;
	up.y = cameraTransform._11;
	up.z = cameraTransform._21;
	target.x = position.x - cameraTransform._02;
	target.y = position.y - cameraTransform._12;
	target.z = position.z - cameraTransform._22;
}


/*
void Camera::LookAt(Vector3	position, Vector3 view, Vector3 up)
{
	// compute matrix
	localTransform.BuildLookAtMatrixLH(position, view, up);
}
 */

void Camera::PrepareDynamicParameters(Vector4 *externalClipPlane)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
	flags = REQUIRE_REBUILD | REQUIRE_REBUILD_MODEL | REQUIRE_REBUILD_PROJECTION;
    if (flags & REQUIRE_REBUILD)
    {
        RebuildCameraFromValues();
    }
	// ApplyFrustum();
    if (flags & REQUIRE_REBUILD_PROJECTION)
    {
        RebuildProjectionMatrix();
       
    }

    if (flags & REQUIRE_REBUILD_MODEL)
    {
        RebuildViewMatrix();
    }
    

    

    viewMatrix.GetInverse(invViewMatrix);

    if (externalClipPlane)
    {
        Vector4 clipPlane(*externalClipPlane);
        if (RenderManager::Instance()->GetRenderOrientation() == Core::SCREEN_ORIENTATION_TEXTURE)
        {
            clipPlane = -clipPlane;
        }
        Matrix4 m;
        
        viewMatrix.GetInverse(m);
        m.Transpose();
        clipPlane  = clipPlane * m;
        
        
        projMatrix.GetInverse(m);
        m.Transpose();
        Vector4 v = Vector4 (Sign(clipPlane.x), Sign(clipPlane.y), 1, 1)*m;
        
        Vector4 scaledPlane = clipPlane * (2.0f / v.DotProduct(clipPlane));

        projMatrix.data[2] = scaledPlane.x;
        projMatrix.data[6] = scaledPlane.y;
        projMatrix.data[10] = scaledPlane.z+1;
        projMatrix.data[14] = scaledPlane.w;
       
    }

    viewProjMatrix = viewMatrix * projMatrix;

    
    flags &= ~REQUIRE_REBUILD_UNIFORM_PROJ_MODEL;

    
    viewProjMatrix.GetInverse(invViewProjMatrix);

    if (currentFrustum)
    {
        currentFrustum->Build(viewProjMatrix);
    }
}   

void Camera::SetupDynamicParameters(Vector4 *externalClipPlane)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    PrepareDynamicParameters(externalClipPlane);

	RenderManager::SetDynamicParam(PARAM_VIEW, &viewMatrix, UPDATE_SEMANTIC_ALWAYS);
    RenderManager::SetDynamicParam(PARAM_PROJ, &projMatrix, UPDATE_SEMANTIC_ALWAYS);
    RenderManager::SetDynamicParam(PARAM_VIEW_PROJ, &viewProjMatrix, UPDATE_SEMANTIC_ALWAYS);
    RenderManager::SetDynamicParam(PARAM_INV_VIEW, &invViewMatrix, UPDATE_SEMANTIC_ALWAYS);
	RenderManager::SetDynamicParam(PARAM_INV_VIEW_PROJ, &invViewProjMatrix, UPDATE_SEMANTIC_ALWAYS);

    RenderManager::SetDynamicParam(PARAM_CAMERA_POS, &position, UPDATE_SEMANTIC_ALWAYS);
	RenderManager::SetDynamicParam(PARAM_CAMERA_DIR, &direction, UPDATE_SEMANTIC_ALWAYS);
	RenderManager::SetDynamicParam(PARAM_CAMERA_UP, &up, UPDATE_SEMANTIC_ALWAYS);
    
}

BaseObject * Camera::Clone(BaseObject * dstNode)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    if (!dstNode) 
    {
		DVASSERT_MSG(IsPointerToExactClass<Camera>(this), "Can clone only Camera");
        dstNode = new Camera();
    }
    // SceneNode::Clone(dstNode);
    Camera *cnd = (Camera*)dstNode;
    cnd->znear = znear;
    cnd->zfar = zfar;
    cnd->aspect = aspect;
    cnd->fovX = fovX;
    cnd->ortho = ortho;
    cnd->orthoWidth = orthoWidth;
    
    cnd->position = position;
    cnd->target = target;
    cnd->up = up;
    cnd->left = left;
    //cnd->rotation = rotation;
    cnd->cameraTransform = cameraTransform;
    cnd->flags = flags;
    cnd->zoomFactor = zoomFactor;

    cnd->Recalc();
    return dstNode;
}
    
Frustum * Camera::GetFrustum() const
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    return currentFrustum;
}
    
void Camera::CalculateZoomFactor()
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    zoomFactor = tanf(DegToRad(fovX * 0.5f));
}

float32 Camera::GetZoomFactor() const
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    return zoomFactor;
}

void Camera::SetCullInvert(bool enabled)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    if (enabled)
        flags |= INVERT_CULL;
    else
        flags &= ~INVERT_CULL;
}

    
void Camera::Draw()
{

}

Vector3 Camera::UnProject(float32 winx, float32 winy, float32 winz, const Rect & viewport)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
//	Matrix4 finalMatrix = modelMatrix * projMatrix;//RenderManager::Instance()->GetUniformMatrix(RenderManager::UNIFORM_MATRIX_MODELVIEWPROJECTION);
    
    Matrix4 finalMatrix = GetViewProjMatrix();
	finalMatrix.Inverse();		

	Vector4 in(winx, winy, winz, 1.0f);

	/* Map x and y from window coordinates */

    in.x = (in.x - viewport.x) / viewport.dx;
    in.y = 1.0f - (in.y - viewport.y) / viewport.dy;
    

	/* Map to range -1 to 1 */
	in.x = in.x * 2 - 1;
	in.y = in.y * 2 - 1;
	in.z = in.z * 2 - 1;

	Vector4 out = in * finalMatrix;
	
	Vector3 result(0,0,0);
	if (out.w == 0.0) return result;
	
	result.x = out.x / out.w;
	result.y = out.y / out.w;
	result.z = out.z / out.w;
	return result;
}
    
/*
     float32 xmin, xmax, ymin, ymax, znear, zfar, aspect, fovx;
     bool ortho;
     
     
     Vector3 position;		//
     Vector3 target;		//	
     Vector3 up;
     Vector3 left;
     
     Vector3 direction;  // right now this variable updated only when you call GetDirection.
     
     //Quaternion rotation;	// 
     Matrix4 cameraTransform;
     
     Matrix4 modelMatrix;
     Matrix4 projMatrix;
     Matrix4 uniformProjModelMatrix;
     
     uint32 flags;
*/
    
void Camera::Save(KeyedArchive * archive)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    BaseObject::Save(archive);
    
    archive->SetFloat("cam.orthoWidth", orthoWidth);
    archive->SetFloat("cam.znear", znear);
    archive->SetFloat("cam.zfar", zfar);
    archive->SetFloat("cam.aspect", aspect);
    archive->SetFloat("cam.fov", fovX);
    archive->SetBool("cam.isOrtho", ortho);
    archive->SetInt32("cam.flags", flags);
    
    archive->SetByteArrayAsType("cam.position", position);
    archive->SetByteArrayAsType("cam.target", target);
    archive->SetByteArrayAsType("cam.up", up);
    archive->SetByteArrayAsType("cam.left", left);
    archive->SetByteArrayAsType("cam.direction", direction);

    archive->SetByteArrayAsType("cam.cameraTransform", cameraTransform);

    archive->SetByteArrayAsType("cam.modelMatrix", viewMatrix);
    archive->SetByteArrayAsType("cam.projMatrix", projMatrix);
}

void Camera::Load(KeyedArchive * archive)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    BaseObject::Load(archive);
    
    // todo add default values
    orthoWidth = archive->GetFloat("cam.orthoWidth");
    znear = archive->GetFloat("cam.znear");
    zfar = archive->GetFloat("cam.zfar");
    aspect = archive->GetFloat("cam.aspect");
    fovX = archive->GetFloat("cam.fov");
    ortho = archive->GetBool("cam.isOrtho");
    flags = archive->GetInt32("cam.flags");
    
    position = archive->GetByteArrayAsType("cam.position", position);
    target = archive->GetByteArrayAsType("cam.target", target);
    up = archive->GetByteArrayAsType("cam.up", up);
    left = archive->GetByteArrayAsType("cam.left", left);
    direction = archive->GetByteArrayAsType("cam.direction", direction);

    cameraTransform = archive->GetByteArrayAsType("cam.cameraTransform", cameraTransform);
    viewMatrix = archive->GetByteArrayAsType("cam.modelMatrix", viewMatrix);
    projMatrix = archive->GetByteArrayAsType("cam.projMatrix", projMatrix);

    Recalc();
}

	
//SceneNode* Camera::Clone()
//{
//    return CopyDataTo(new Camera(scene));
//}
//
//SceneNode* Camera::CopyDataTo(SceneNode *dstNode)
//{
//    SceneNode::CopyDataTo(dstNode);
//    dstNode->xmin = xmin;
//    dstNode->xmax = xmax;
//    dstNode->ymin = ymin;
//    dstNode->ymax = ymax;
//    dstNode->znear = znear;
//    dstNode->zfar = zfar;
//    dstNode->aspect = aspect;
//    dstNode->fovx = fovx;
//	dstNode->ortho = ortho;
//    
//	dstNode->position = position;
//	dstNode->target = target;
//	dstNode->up = up;
//	dstNode->left = left;
//	dstNode->rotation = rotation;
//	dstNode->cameraTransform = cameraTransform;
//    return dstNode;
//}


void Camera::CopyMathOnly(const Camera & c)
{
    TAG_SWITCH(MemoryManager::TAG_CAMERA)
    
    *currentFrustum = *c.currentFrustum;
    zoomFactor = c.zoomFactor;

    xmin = c.xmin;
    xmax = c.xmax;
    ymin = c.ymin;
    ymax = c.ymax;
    znear = c.znear;
    zfar = c.zfar;
    aspect = c.aspect;
    fovX = c.fovX;
    ortho = c.ortho;

    position = c.position;
    target = c.target;
    up = c.up;
    left = c.left;

    direction = c.direction;
    
    cameraTransform = c.cameraTransform;
    viewMatrix = c.viewMatrix;
    projMatrix = c.projMatrix;
    viewProjMatrix = c.viewProjMatrix;
    flags = c.flags;
}




} // ns



