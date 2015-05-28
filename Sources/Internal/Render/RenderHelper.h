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


#ifndef __DAVAENGINE_RENDER_HELPER_H__
#define __DAVAENGINE_RENDER_HELPER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "Render/RenderBase.h"

#include "Render/Texture.h"
#include "Animation/Interpolation.h"
#include "Render/Renderer.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{	
    
/**
    \brief You can use this class to perform various important drawing operations in 2D & 3D.
    In most cases we use these functions for debug rendering, but in some cases it can be usefull in production. 
    Keep in mind that output of all line-drawing functions can depend on hardware and look differently on different systems
 */

class RenderHelper : public Singleton<RenderHelper>
{
public:
	RenderHelper();
    ~RenderHelper();

    /**
        \brief Draws line in 3D from pt1 to pt2
        \param pt1 starting point 
        \param pt2 ending point
	 */
    void DrawLine(const Vector3 & pt1, const Vector3 & pt2, float32 lineWidth /*= 1.f*/, NMaterial* material);

    /**

    /**
        \brief Draws circle in 3D space on XY plane
        \param center center of the circle
        \param radius radius of the circle
     */
    void DrawCircle(const Vector3 & center, float32 radius, NMaterial* material);
	
	/**
        \brief Draws circle in 3D space on XYZ plane
        \param center center of the circle
		\param emVector direction vector of the circle
        \param radius radius of the circle
		\param useFilling flag that indicates if figure have to be filled with current color
     */	 
    void DrawCircle3D(const Vector3 & center, const Vector3 &directionVector, float32 radius, bool useFilling /*= false*/, NMaterial* material);

	/**
        \brief Draws cylinder in 3D space on XY plane
        \param center center of the cylinder
        \param radius radius of the cylinder
		\param useFilling flag that indicates if figure have to be filled with current color
     */
    void DrawCylinder(const Vector3 & center, float32 radius, bool useFilling /*= false*/, NMaterial* material);
	
	// polygon & line helper functions
	
    /**
        \brief Draws all concecutive lines from given polygon
        \param polygon the polygon we want to draw
        \param closed you should set this flag to true if you want to connect last point of polygon with first point
     */
    void DrawPolygon(const Polygon3 & polygon, bool closed, NMaterial* material);

    /**
        \brief Fill convex polygon with color. As all other draw functions this function use global color that can be set with RenderSystem2D::Instance()->SetColor function. 
        \param polygon the polygon we want to draw
     */
    void FillPolygon(const Polygon3 & polygon, NMaterial* material);
    
    /**
        \brief Draws all points from given polygon
        \param polygon the polygon we want to draw
     */
    void DrawPolygonPoints(const Polygon3 & polygon, NMaterial* material);
    
    /**
        \brief Draws 2D bounding box
        \param box given bounding box
     */
    void DrawBox(const AABBox2 & box, float32 lineWidth /*= 1.f*/, NMaterial* material);

    /**
        \brief Draws 3D bounding box
        \param box given bounding box
     */
    void DrawBox(const AABBox3 & box, float32 lineWidth /*= 1.f*/, NMaterial* material);

    /**
        \brief Fills 3D bounding box
        \param box given bounding box
     */
    void FillBox(const AABBox3 & box, NMaterial* material);

    /**
	 \brief Draws 3D bounding box with corners
	 \param box given bounding box
     */
    void DrawCornerBox(const AABBox3 & bbox, float32 lineWidth /*= 1.f*/, NMaterial* material);
	
    void DrawSphere(const Vector3 &center, float32 radius, float32 lineWidth /*= 1.f*/, NMaterial* material);
    void FillSphere(const Vector3 &center, float32 radius, NMaterial* material);

	void DrawArrow(const Vector3 &from, const Vector3 &to, float32 arrowLength, float32 lineWidth /*= 1.f*/, NMaterial* material);
	void FillArrow(const Vector3 &from, const Vector3 &to, float32 arrowLength, float32 lineWidth /*= 1.f*/, NMaterial* material);

	void DrawDodecahedron(const Vector3 &center, float32 radius, float32 lineWidth /*= 1.f*/, NMaterial* material);
	void FillDodecahedron(const Vector3 &center, float32 radius, NMaterial* material);
	
    // Other debug functions  
	void DrawBSpline(BezierSpline3 * bSpline, int segments /*= 20*/, float ts /*= 0.0f*/, float te /*= 1.0f*/, NMaterial* material);
	void DrawInterpolationFunc(Interpolation::Func func, const Rect & destRect, NMaterial* material);
    //static void DrawLineWithEndPoints(const Vector3 & pt1, const Vector3 & pt2); 
	//static void DrawStrippedLine(Polygon2 & polygon, float lineLen, float spaceLen, float halfWidth, Texture * texture, float initialPos);

    void Set2DRenderTarget(Texture * renderTarget);

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
	void GetLineWidthRange(int32& rangeMin, int32& rangeMax);
#endif

private:
    
    float32 vertices[32];
    float32 texCoords[8];

    Matrix4 tempProjectionMatrix;
};
	
}

#endif // __DAVAENGINE_OBJC_FRAMEWORK_RENDER_HELPER_H__

