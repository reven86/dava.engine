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

namespace DAVA
{
	
class RenderDataObject;
class RenderDataStream;
    
    
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
        \brief Draws line from pt1 to pt2
        \param pt1 starting point 
        \param pt2 ending point
	 */
    void DrawLine(const Vector2 & pt1, const Vector2 & pt2, UniqueHandle renderState);
	
	
    /**
	 \brief Draws line from pt1 to pt2
	 \param pt1 starting point
	 \param pt2 ending point
	 */
	void DrawLine(const Vector2 &start, const Vector2 &end, float32 lineWidth, UniqueHandle renderState);
    
	/**
        \brief Draws line in 3D from pt1 to pt2
        \param pt1 starting point 
        \param pt2 ending point
	 */
	void DrawLine(const Vector3 & pt1, const Vector3 & pt2, float32 lineWidth /*= 1.f*/, UniqueHandle renderState);
    /**
	 \brief Draws multiple lines.
	 \param linePoints list of points in the format (startX, startY, endX, endY), (startX, startY, endX, endY)...
	 */
    void DrawLines(const Vector<float32>& linePoints, UniqueHandle renderState);
    
    /**
        \brief Draws given rect in 2D space
        \param pt1 starting point 
        \param pt2 ending point
	 */    
    void DrawRect(const Rect & rect, UniqueHandle renderState);
    
    /**
        \brief Fills given rect in 2D space
        \param pt1 starting point 
        \param pt2 ending point
	 */ 
    void FillRect(const Rect & rect, UniqueHandle renderState);

    /**
     \brief Draws grid in the given rect
     \param rect rect to fill grid with
     \param gridSize distance between grid lines
     \param color grid color
	 */
    void DrawGrid(const Rect & rect, const Vector2& gridSize, const Color& color, UniqueHandle renderState);

	// point helpers
    
    /**
        \brief Draws given point in 2D space
        \param pt given point 
	 */
	void DrawPoint(const Vector2 & pt, float32 ptSize /*= 1.0f*/, UniqueHandle renderState);
    
    /**
        \brief Draws given point in 3D space
        \param pt given point 
	 */
	void DrawPoint(const Vector3 & pt, float32 ptSize /*= 1.0f*/, UniqueHandle renderState);
    
    /**
        \brief Draws circle in 2D space
        \param center center of the circle
        \param radius radius of the circle
     */
	void DrawCircle(const Vector2 & center, float32 radius, UniqueHandle renderState);
    
    
    /**
        \brief Draws circle in 3D space on XY plane
        \param center center of the circle
        \param radius radius of the circle
     */
	void DrawCircle(const Vector3 & center, float32 radius, UniqueHandle renderState);
	
	/**
        \brief Draws circle in 3D space on XYZ plane
        \param center center of the circle
		\param emVector direction vector of the circle
        \param radius radius of the circle
		\param useFilling flag that indicates if figure have to be filled with current color
		
     */	 
	void DrawCircle3D(const Vector3 & center, const Vector3 &directionVector, float32 radius, bool useFilling /*= false*/, UniqueHandle renderState);

	/**
        \brief Draws cylinder in 3D space on XY plane
        \param center center of the cylinder
        \param radius radius of the cylinder
		\param useFilling flag that indicates if figure have to be filled with current color
     */
	void DrawCylinder(const Vector3 & center, float32 radius, bool useFilling /*= false*/, UniqueHandle renderState);
	
	// polygon & line helper functions
	
    /**
        \brief Draws all concecutive lines from given polygon
        \param polygon the polygon we want to draw
        \param closed you should set this flag to true if you want to connect last point of polygon with first point
     */
    void DrawPolygon(const Polygon2 & polygon, bool closed, UniqueHandle renderState);
    /**
        \brief Draws all concecutive lines from given polygon
        \param polygon the polygon we want to draw
        \param closed you should set this flag to true if you want to connect last point of polygon with first point
     */
    void DrawPolygon(const Polygon3 & polygon, bool closed, UniqueHandle renderState);

    /**
        \brief Fill convex polygon with color. As all other draw functions this function use global color that can be set with RenderManager::Instance()->SetColor function. 
        \param polygon the polygon we want to draw
     */
    void FillPolygon(const Polygon2 & polygon, UniqueHandle renderState);
    
    /**
        \brief Fill convex polygon with color. As all other draw functions this function use global color that can be set with RenderManager::Instance()->SetColor function. 
        \param polygon the polygon we want to draw
     */
    void FillPolygon(const Polygon3 & polygon, UniqueHandle renderState);
    
    /**
        \brief Draws all concecutive lines from given polygon after transformation
        \param polygon the polygon we want to draw
        \param closed you should set this flag to true if you want to connect last point of polygon with first point
        \param transform transform that will be applied to polygon before it will be drawn
     */
	void DrawPolygonTransformed(const Polygon2 & polygon, bool closed, const Matrix3 & transform, UniqueHandle renderState);
    
    /**
        \brief Draws all points from given polygon
        \param polygon the polygon we want to draw
     */
    void DrawPolygonPoints( const Polygon2 & polygon, UniqueHandle renderState);

    /**
        \brief Draws all points from given polygon
        \param polygon the polygon we want to draw
     */
    void DrawPolygonPoints(const Polygon3 & polygon, UniqueHandle renderState);
    
    /**
        \brief Draws 2D bounding box
        \param box given bounding box
     */
	void DrawBox(const AABBox2 & box, float32 lineWidth /*= 1.f*/, UniqueHandle renderState);

    /**
        \brief Draws 3D bounding box
        \param box given bounding box
     */
	void DrawBox(const AABBox3 & box, float32 lineWidth /*= 1.f*/, UniqueHandle renderState);

    /**
        \brief Fills 3D bounding box
        \param box given bounding box
     */
	void FillBox(const AABBox3 & box, UniqueHandle renderState);

    /**
	 \brief Draws 3D bounding box with corners
	 \param box given bounding box
     */
	void DrawCornerBox(const AABBox3 & bbox, float32 lineWidth /*= 1.f*/, UniqueHandle renderState);
	
	void DrawSphere(const Vector3 &center, float32 radius, float32 lineWidth /*= 1.f*/, UniqueHandle renderState);
	void FillSphere(const Vector3 &center, float32 radius, UniqueHandle renderState);

	void DrawArrow(const Vector3 &from, const Vector3 &to, float32 arrowLength, float32 lineWidth /*= 1.f*/, UniqueHandle renderState);
	void FillArrow(const Vector3 &from, const Vector3 &to, float32 arrowLength, float32 lineWidth /*= 1.f*/, UniqueHandle renderState);

	void DrawDodecahedron(const Vector3 &center, float32 radius, float32 lineWidth /*= 1.f*/, UniqueHandle renderState);
	void FillDodecahedron(const Vector3 &center, float32 radius, UniqueHandle renderState);
	
    // Other debug functions  
	void DrawBSpline(BezierSpline3 * bSpline, int segments /*= 20*/, float ts /*= 0.0f*/, float te /*= 1.0f*/, UniqueHandle renderState);
	void DrawInterpolationFunc(Interpolation::Func func, const Rect & destRect, UniqueHandle renderState);
    //static void DrawLineWithEndPoints(const Vector3 & pt1, const Vector3 & pt2); 
	//static void DrawStrippedLine(Polygon2 & polygon, float lineLen, float spaceLen, float halfWidth, Texture * texture, float initialPos);

    void Set2DRenderTarget(Texture * renderTarget);
    void DrawTexture(Texture * texture, UniqueHandle renderState, const Rect & dstRect = Rect(0.f, 0.f, -1.f, -1.f), const Rect & srcRect = Rect(0.f, 0.f, -1.f, -1.f));

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
	void GetLineWidthRange(int32& rangeMin, int32& rangeMax);
#endif

private:
    RenderDataObject * renderDataObject;
    RenderDataStream * vertexStream;
    RenderDataStream * texCoordStream;
    float32 vertices[32];
    float32 texCoords[8];

    Matrix4 tempProjectionMatrix;
};
	
}

#endif // __DAVAENGINE_OBJC_FRAMEWORK_RENDER_HELPER_H__

