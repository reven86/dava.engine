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


#include "Render/RenderHelper.h"
#include "Render/RenderManager.h"
#include "Render/Texture.h"
#include "Render/RenderDataObject.h"
#include "Render/2D/Systems/RenderSystem2D.h"

namespace DAVA
{
	/*
	static Vector3 DodecVertexes[20] = {
		Vector3( 0.607f,  0.000f,  0.795f),
		Vector3( 0.188f,  0.577f,  0.795f),
		Vector3(-0.491f,  0.357f,  0.795f),
		Vector3(-0.491f, -0.357f,  0.795f),
		Vector3( 0.188f, -0.577f,  0.795f),
		Vector3( 0.982f,  0.000f,  0.188f),
		Vector3( 0.304f,  0.934f,  0.188f),
		Vector3(-0.795f,  0.577f,  0.188f),
		Vector3(-0.795f, -0.577f,  0.188f),
		Vector3( 0.304f, -0.934f,  0.188f),
		Vector3( 0.795f,  0.577f, -0.188f),
		Vector3(-0.304f,  0.934f, -0.188f),
		Vector3(-0.982f,  0.000f, -0.188f),
		Vector3(-0.304f, -0.934f, -0.188f),
		Vector3( 0.795f, -0.577f, -0.188f),
		Vector3( 0.491f,  0.357f, -0.795f),
		Vector3(-0.188f,  0.577f, -0.795f),
		Vector3(-0.607f,  0.000f, -0.795f),
		Vector3(-0.188f, -0.577f, -0.795f),
		Vector3( 0.491f, -0.357f, -0.795f)
	};

	static int DodecIndexes[12][5] = { 
		0, 1, 2, 3, 4,
		0, 1, 6, 10, 5,
		1, 2, 7, 11, 6,
		2, 3, 8, 12, 7,
		3, 4, 9, 13, 8,
		4, 0, 5, 14, 9,
		15, 16, 11, 6, 10,
		16, 17, 12, 7, 11,
		17, 18, 13, 8, 12,
		18, 19, 14, 9, 13,
		19, 15, 10, 5, 14,
		15, 16, 17, 18, 19
	};
	*/

	#define isoX 0.525731f 
	#define isoZ 0.850650f

	static Vector3 gDodecVertexes[12] = {
		Vector3(-isoX, 0.0, isoZ),
		Vector3(isoX, 0.0, isoZ),
		Vector3(-isoX, 0.0, -isoZ),
		Vector3(isoX, 0.0, -isoZ),
		Vector3(0.0, isoZ, isoX),
		Vector3(0.0, isoZ, -isoX),
		Vector3(0.0, -isoZ, isoX),
		Vector3(0.0, -isoZ, -isoX),
		Vector3(isoZ, isoX, 0.0),
		Vector3(-isoZ, isoX, 0.0),
		Vector3(isoZ, -isoX, 0.0),
		Vector3(-isoZ, -isoX, 0.0)
	};

	static DAVA::uint16 gDodecIndexes[60] = {
		0, 4, 1,
		0, 9, 4,
		9, 5, 4,
		4, 5, 8,
		4, 8, 1,
		8, 10, 1,
		8, 3, 10,
		5, 3, 8,
		5, 2, 3,
		2, 7, 3,
		7, 10, 3,
		7, 6, 10,
		7, 11, 6,
		11, 0, 6,
		0, 1, 6,
		6, 1, 10,
		9, 0, 11,
		9, 11, 2,
		9, 2, 5,
		7, 2, 11
	};

	static RenderDataObject *gDodecObject;
	
	const float32 SEGMENT_LENGTH = 15.0f;
	
RenderHelper::RenderHelper()
{
    renderDataObject = new RenderDataObject();
    vertexStream = renderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
    texCoordStream = renderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);

	gDodecObject = new RenderDataObject();
	gDodecObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, gDodecVertexes);
	gDodecObject->SetIndices(EIF_16, (DAVA::uint8 *) gDodecIndexes, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]));
}
RenderHelper::~RenderHelper()
{
    SafeRelease(renderDataObject);
	SafeRelease(gDodecObject);
}
    
void RenderHelper::FillRect(const Rect & rect, UniqueHandle renderState)
{
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    vertices[0] = rect.x;						
    vertices[1] = rect.y;
    vertices[2] = rect.x + rect.dx;
    vertices[3] = rect.y;
    vertices[4] = rect.x;						
    vertices[5] = rect.y + rect.dy;
    vertices[6] = rect.x + rect.dx;			
    vertices[7] = rect.y + rect.dy;

    vertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
    
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
}

void RenderHelper::DrawRect(const Rect & rect, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    vertices[0] = rect.x;						
    vertices[1] = rect.y;
    vertices[2] = rect.x + rect.dx;
    vertices[3] = rect.y;
    vertices[4] = rect.x + rect.dx;						
    vertices[5] = rect.y + rect.dy;
    vertices[6] = rect.x;			
    vertices[7] = rect.y + rect.dy;
    vertices[8] = rect.x;						
    vertices[9] = rect.y;

    vertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
    
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, 5);
}

void RenderHelper::DrawGrid(const Rect & rect, const Vector2& gridSize, const Color& color, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    // TODO! review with Ivan/Victor whether it is not performance problem!
    Vector<float32> gridVertices;
    int32 verLinesCount = (int32)ceilf(rect.dx / gridSize.x);
    int32 horLinesCount = (int32)ceilf(rect.dy / gridSize.y);
    gridVertices.resize((horLinesCount + verLinesCount) * 4);
    
    float32 curPos = 0;
    int32 curVertexIndex = 0;
    for (int i = 0; i < horLinesCount; i ++)
    {
        gridVertices[curVertexIndex ++] = rect.x;
        gridVertices[curVertexIndex ++] = rect.y + curPos;
        gridVertices[curVertexIndex ++] = rect.x + rect.dx;
        gridVertices[curVertexIndex ++] = rect.y + curPos;
        
        curPos += gridSize.x;
    }

    curPos = 0.0f;
    for (int i = 0; i < verLinesCount; i ++)
    {
        gridVertices[curVertexIndex ++] = rect.x + curPos;
        gridVertices[curVertexIndex ++] = rect.y;
        gridVertices[curVertexIndex ++] = rect.x + curPos;
        gridVertices[curVertexIndex ++] = rect.y + rect.dy;

        curPos += gridSize.y;
    }

    vertexStream->Set(TYPE_FLOAT, 2, 0, gridVertices.data());

    RenderManager::Instance()->SetRenderState(renderState);
    Color oldColor = RenderManager::Instance()->GetColor();
    RenderManager::Instance()->SetColor(color);
    
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINELIST, 0, curVertexIndex / 2);
    
    RenderManager::Instance()->SetColor(oldColor);
}

void RenderHelper::DrawLine(const Vector2 &start, const Vector2 &end, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    vertices[0] = start.x;						
    vertices[1] = start.y;
    vertices[2] = end.x;
    vertices[3] = end.y;
    
    vertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
    
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, 2);
}

	void RenderHelper::DrawLine(const Vector2 &start, const Vector2 &end, float32 lineWidth, UniqueHandle renderState)
    {
        RenderSystem2D::Instance()->Flush();

        RenderSystem2D::Instance()->UpdateClip();

		vertices[0] = start.x;
		vertices[1] = start.y;
		vertices[2] = end.x;
		vertices[3] = end.y;
		
		vertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
		
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(renderDataObject);
#ifdef __DAVAENGINE_OPENGL__
		glLineWidth(lineWidth);
#endif
		RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, 2);
#ifdef __DAVAENGINE_OPENGL__
		glLineWidth(1.f);
#endif
	}

	
    
void RenderHelper::DrawLine(const Vector3 & start, const Vector3 & end, float32 lineWidth, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    vertices[0] = start.x;						
    vertices[1] = start.y;
    vertices[2] = start.z;
    
    vertices[3] = end.x;
    vertices[4] = end.y;
    vertices[5] = end.z;

    
    vertexStream->Set(TYPE_FLOAT, 3, 0, vertices);
    
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);

#ifdef __DAVAENGINE_OPENGL__
	glLineWidth(lineWidth);
#endif
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, 2);
#ifdef __DAVAENGINE_OPENGL__
	glLineWidth(1.f);
#endif
}

void RenderHelper::DrawLines(const Vector<float32>& linePoints, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    vertexStream->Set(TYPE_FLOAT, 2, 0, linePoints.data());

    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);

    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINELIST, 0, static_cast<int32>(linePoints.size() / 2));
}


void RenderHelper::DrawPoint(const Vector2 & pt, float32 ptSize, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

#if defined (__DAVAENGINE_OPENGL__)
    glPointSize(ptSize);
#endif 
    vertexStream->Set(TYPE_FLOAT, 2, 0, (void*)&pt);
    
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_POINTLIST, 0, 1);
#if defined (__DAVAENGINE_OPENGL__)
    glPointSize(1.0f);
#endif
}
	
void RenderHelper::DrawPoint(const Vector3 & pt, float32 ptSize, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

#if defined (__DAVAENGINE_OPENGL__)
    glPointSize(ptSize);
#endif 
    vertexStream->Set(TYPE_FLOAT, 3, 0, (void*)&pt);
    
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_POINTLIST, 0, 1);
#if defined (__DAVAENGINE_OPENGL__)
    glPointSize(1.0f);
#endif		
}
	
void RenderHelper::DrawCircle(const Vector2 & center, float32 radius, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	Polygon2 pts;
    float32 angle = Min(PI/6.0f, SEGMENT_LENGTH / radius);// maximum angle 30 degrees
	int ptsCount = (int)(2 * PI / angle) + 1;
	
    pts.points.reserve(ptsCount);
	for (int k = 0; k < ptsCount; ++k)
	{
		float32 angle = ((float)k / (ptsCount - 1)) * 2 * PI;
		float32 sinA = sinf(angle);
		float32 cosA = cosf(angle);
		Vector2 pos = center - Vector2(sinA * radius, cosA * radius);
		
		pts.AddPoint(pos);
	}
	
    DrawPolygon(pts, false, renderState);
}

void RenderHelper::DrawCircle(const Vector3 & center, float32 radius, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	Polygon3 pts;
    float32 angle = Min(PI/6.0f, SEGMENT_LENGTH / radius);// maximum angle 30 degrees
	int ptsCount = (int)(2 * PI / (DegToRad(angle))) + 1;

    pts.points.reserve(ptsCount);
	for (int k = 0; k < ptsCount; ++k)
	{
		float32 angle = ((float)k / (ptsCount - 1)) * 2 * PI;
		float32 sinA = sinf(angle);
		float32 cosA = cosf(angle);
		Vector3 pos = center - Vector3(sinA * radius, cosA * radius, 0);

		pts.AddPoint(pos);
	}
    DrawPolygon(pts, false, renderState);
}

void RenderHelper::DrawCircle3D(const Vector3 & center, const Vector3 &emissionVector, float32 radius, bool useFilling, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	Polygon3 pts;
    float32 angle = Min(PI/6.0f, SEGMENT_LENGTH / radius);// maximum angle 30 degrees
	int ptsCount = (int)(PI_2 / (DegToRad(angle))) + 1;

    pts.points.reserve(ptsCount);
	for (int k = 0; k < ptsCount; ++k)
	{
		float32 angleA = ((float)k / (ptsCount - 1)) * PI_2;
		float sinAngle = 0.0f;
		float cosAngle = 0.0f;
		SinCosFast(angleA, sinAngle, cosAngle);

		Vector3 directionVector(radius * cosAngle,
								radius * sinAngle,
								0.0f);
		
		// Rotate the direction vector according to the current emission vector value.
		Vector3 zNormalVector(0.0f, 0.0f, 1.0f);
		Vector3 curEmissionVector = emissionVector;
        if (FLOAT_EQUAL(curEmissionVector.Length(), 0.f) == false)
        {
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
            if (FLOAT_EQUAL(axisRot.Length(), 0.f) == false)
            {
                axisRot.Normalize();
            }
			Matrix3 planeRotMatrix;
			planeRotMatrix.CreateRotation(axisRot, angleRot);
			Vector3 rotatedVector = directionVector * planeRotMatrix;
			directionVector = rotatedVector;
		}
		
		Vector3 pos = center - directionVector;
		pts.AddPoint(pos);
	}
	
	if (useFilling)
	{
		FillPolygon(pts, renderState);
	}
	else
	{
    	DrawPolygon(pts, false, renderState);
	}
}

void RenderHelper::DrawCylinder(const Vector3 & center, float32 radius, bool useFilling, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	Polygon3 pts;
    float32 angle = Min(PI/6.0f, SEGMENT_LENGTH / radius);// maximum angle 30 degrees
	int32 ptsCount = (int32)(PI_2 / (DegToRad(angle))) + 1;

	Vector<Vector2> vertexes;
    vertexes.reserve(ptsCount + 1);
	for(int32 i = 0; i <= ptsCount; i++)
 	{
		float32 seta = i * 360.0f / (float32)ptsCount;
  		float32 x = sin(DegToRad(seta)) * radius;
  		float32 y = cos(DegToRad(seta)) * radius;

		vertexes.push_back(Vector2(x, y));
	}
	
    pts.points.reserve(ptsCount * 6);
	for(int32 i = 0; i < ptsCount; ++i)
	{
		pts.AddPoint((Vector3(vertexes[i].x,  vertexes[i].y,  1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i].x,  vertexes[i].y,  -1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i+1].x, vertexes[i+1].y,  -1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i].x,  vertexes[i].y,  1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i+1].x, vertexes[i+1].y,  1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i+1].x, vertexes[i+1].y,  -1) * radius) + center);
	}
	
	if (useFilling)
	{
		FillPolygon(pts, renderState);
	}
	else
	{
		DrawPolygon(pts, true, renderState);
	}
}

void RenderHelper::DrawPolygonPoints(const Polygon2 & polygon, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	int ptCount = polygon.pointCount;
	if (ptCount >= 1)
	{
#if defined (__DAVAENGINE_OPENGL__)
        glPointSize(3.0f);
#endif 
        
		vertexStream->Set(TYPE_FLOAT, 2, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(renderDataObject);
		RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_POINTLIST, 0, ptCount);
#if defined (__DAVAENGINE_OPENGL__)
		glPointSize(1.0f);
#endif		
	}
}
	
void RenderHelper::DrawPolygonPoints(const Polygon3 & polygon, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	int ptCount = polygon.pointCount;
	if (ptCount >= 1)
	{
#if defined (__DAVAENGINE_OPENGL__)
        glPointSize(3.0f);
#endif 
		vertexStream->Set(TYPE_FLOAT, 3, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(renderDataObject);
		RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_POINTLIST, 0, ptCount);
#if defined (__DAVAENGINE_OPENGL__)
		glPointSize(1.0f);
#endif		
	}
	
}
	
void RenderHelper::DrawPolygon(const Polygon3 & polygon, bool closed, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    int ptCount = polygon.pointCount;
	if (ptCount >= 2)
	{		
		vertexStream->Set(TYPE_FLOAT, 3, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(renderDataObject);
		RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, ptCount);
		
		if (closed)
		{
		    Vector3 line[2] = {Vector3(polygon.GetPoints()[0]), Vector3(polygon.GetPoints()[ptCount-1])};
		    vertexStream->Set(TYPE_FLOAT, 3, 0, line);
		    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, 2);
		}
    }

	
}


void RenderHelper::DrawPolygon( const Polygon2 & polygon, bool closed, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	int ptCount = polygon.pointCount;
	if (ptCount >= 2)
	{		
		vertexStream->Set(TYPE_FLOAT, 2, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(renderDataObject);
		RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, ptCount);
		
		if (closed)
		{
		    Vector2 line[2] = {Vector2(polygon.GetPoints()[0]), Vector2(polygon.GetPoints()[ptCount-1])};
		    vertexStream->Set(TYPE_FLOAT, 2, 0, line);
		    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, 2);
		}
	}
}
    
void RenderHelper::FillPolygon(const Polygon2 & polygon, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    int ptCount = polygon.pointCount;
	if (ptCount >= 3)
	{		
		vertexStream->Set(TYPE_FLOAT, 2, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(renderDataObject);
		RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLEFAN, 0, ptCount);
    }
}

void RenderHelper::FillPolygon(const Polygon3 & polygon, UniqueHandle renderState)
{
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    int ptCount = polygon.pointCount;
	if (ptCount >= 3)
	{		
		vertexStream->Set(TYPE_FLOAT, 3, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(renderDataObject);
		RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLEFAN, 0, ptCount);
    }

}

void RenderHelper::DrawPolygonTransformed(const Polygon2 & polygon, bool closed, const Matrix3 & transform, UniqueHandle renderState)
{
	Polygon2 copyPoly = polygon;
	copyPoly.Transform(transform);
	RenderHelper::Instance()->DrawPolygon(copyPoly, closed, renderState);
}

#if 0
void RenderHelper::DrawLineWithEndPoints(const Vector3 & pt1, const Vector3 & pt2)
{
	RenderManager::Instance()->EnableTexturing(false);
	RenderManager::Instance()->FlushState();
	
	Vector3 line[2] = {pt1, pt2};

	glVertexPointer(3, GL_FLOAT, 0, line);
	:(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);
	
	glPointSize(5.0f);
	glDrawArrays(GL_POINTS, 0, 2);
	glPointSize(1.0f);
	RenderManager::Instance()->EnableTexturing(true);
}

static Vector2 tmpPolyline[600];	
static Vector2 tmpPolylineTexCoords[600];

void RenderHelper::DrawStrippedLine(Polygon2 & polygon, float lineLen, float spaceLen, float halfWidth, Texture * texture, float initialPos)
{	
//	Vector2 texCoords[4] = 
//	{		
//			Vector2(0.0f, 0.0f)
//		,	Vector2(0.0f, 5.0f / 8.0f)
//		,	Vector2(3.0f / 8.0f, 0.0f)
//		,	Vector2(3.0f / 8.0f, 5.0f / 8.0f)
//	};
	Vector2 texCoords[4] = 
	{		
			Vector2(0.0f, 0.0f)
		,	Vector2(3.0f / 8.0f, 0.0f)
		,	Vector2(0.0f, 8.0f / 8.0f)
		,	Vector2(3.0f / 8.0f, 8.0f / 8.0f)
	};		
	float positionOnPoly = 0.0f;
	Vector2 p1, p2;
	int s1, s2;
	int polyCount = 0;
	while(1)
	{
		polygon.InterpolatePositionFromDistance(positionOnPoly, 0, p1, s1);	
		if (s1 == -1)break;
		polygon.InterpolatePositionFromDistance(positionOnPoly + lineLen, 0, p2, s2);	
		if (s2 == -1)break;
/*
		polygon.InterpolatePositionFromDistanceReverse(positionOnPoly, polygon.pointCount - 1, p1, s1);	
		if (s1 == -1)break;
		polygon.InterpolatePositionFromDistanceReverse(positionOnPoly + lineLen, polygon.pointCount - 1, p2, s2);	
		if (s2 == -1)break;
*/
		
		positionOnPoly += lineLen + spaceLen;
		
		Vector2 dir = p1 - p2;
		Vector2 n(dir.y, -dir.x);
		n.Normalize();
		Vector2 pf0 = p1 + n * halfWidth;
		Vector2 pf1 = p1 - n * halfWidth;
		Vector2 pf2 = p2 + n * halfWidth;
		Vector2 pf3 = p2 - n * halfWidth;
		
		
		tmpPolyline[polyCount * 6 + 0] = pf0;
		tmpPolyline[polyCount * 6 + 1] = pf1;
		tmpPolyline[polyCount * 6 + 2] = pf2;
		
		tmpPolyline[polyCount * 6 + 3] = pf1;
		tmpPolyline[polyCount * 6 + 4] = pf2;
		tmpPolyline[polyCount * 6 + 5] = pf3;
		
		tmpPolylineTexCoords[polyCount * 6 + 0] = texCoords[0];
		tmpPolylineTexCoords[polyCount * 6 + 1] = texCoords[1];
		tmpPolylineTexCoords[polyCount * 6 + 2] = texCoords[2];
		
		tmpPolylineTexCoords[polyCount * 6 + 3] = texCoords[1];
		tmpPolylineTexCoords[polyCount * 6 + 4] = texCoords[2];
		tmpPolylineTexCoords[polyCount * 6 + 5] = texCoords[3];
		
		polyCount++;
		if (polyCount >= 100)break;
	}	
	//RenderManager_disableTexturing();
	//RenderManager_flushState();
	RenderManager::Instance()->SetTexture(texture);
	RenderManager::Instance()->FlushState();
	glVertexPointer(2, GL_FLOAT, 0, tmpPolyline);
	glTexCoordPointer(2, GL_FLOAT, 0, tmpPolylineTexCoords);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glDisableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_TRIANGLES, 0, polyCount * 6);
	//RenderManager_enableTexturing();
}
#endif 
    
void RenderHelper::DrawBSpline(BezierSpline3 * bSpline, int segments, float ts, float te, UniqueHandle renderState)
{
	Polygon3 pts;
    pts.points.reserve(segments);
	for (int k = 0; k < segments; ++k)
	{
		pts.AddPoint(bSpline->Evaluate(0, ts + (te - ts) * ((float)k / (float)(segments - 1))));
	}
    DrawPolygon(pts, false, renderState);
}
	
void RenderHelper::DrawInterpolationFunc(Interpolation::Func func, const Rect & destRect, UniqueHandle renderState)
{
	Polygon3 pts;
	int segmentsCount = 20;
    pts.points.reserve(segmentsCount);
	for (int k = 0; k < segmentsCount; ++k)
	{
		Vector3 v;
		v.x = destRect.x + ((float)k / (float)(segmentsCount - 1)) * destRect.dx;
		v.y = destRect.y + func(((float)k / (float)(segmentsCount - 1))) * destRect.dy;
		v.z = 0.0f;
		pts.AddPoint(v);
	}
	DrawPolygon(pts, false, renderState);
}
	
void RenderHelper::DrawBox(const AABBox2 & box, float32 lineWidth, UniqueHandle renderState)
{
    RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, 0), Vector3(box.max.x, box.min.y, 0), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.min.y, 0), Vector3(box.max.x, box.max.y, 0), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.max.y, 0), Vector3(box.min.x, box.max.y, 0), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, 0), Vector3(box.min.x, box.min.y, 0), lineWidth, renderState);
}
	
void RenderHelper::DrawBox(const AABBox3 & box, float32 lineWidth, UniqueHandle renderState)
{
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, box.min.z), Vector3(box.min.x, box.min.y, box.max.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, box.min.z), Vector3(box.min.x, box.max.y, box.min.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, box.max.z), Vector3(box.min.x, box.min.y, box.max.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, box.max.z), Vector3(box.min.x, box.max.y, box.min.z), lineWidth, renderState);
	
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.min.y, box.min.z), Vector3(box.max.x, box.min.y, box.max.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.min.y, box.min.z), Vector3(box.max.x, box.max.y, box.min.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.max.y, box.max.z), Vector3(box.max.x, box.min.y, box.max.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.max.y, box.max.z), Vector3(box.max.x, box.max.y, box.min.z), lineWidth, renderState);
	
	
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, box.min.z), Vector3(box.max.x, box.min.y, box.min.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, box.min.z), Vector3(box.max.x, box.max.y, box.min.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, box.max.z), Vector3(box.max.x, box.min.y, box.max.z), lineWidth, renderState);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, box.max.z), Vector3(box.max.x, box.max.y, box.max.z), lineWidth, renderState);
}
	
void RenderHelper::DrawCornerBox(const AABBox3 & bbox, float32 lineWidth, UniqueHandle renderState)
{
	float32 offs = ((bbox.max - bbox.min).Length()) * 0.1f + 0.1f;
    
    //1
    Vector3 point = bbox.min;
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, 0, offs), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, offs, 0), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(offs, 0, 0), lineWidth, renderState);
    
    //2
    point = bbox.max;
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, 0, offs), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, offs, 0), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(offs, 0, 0), lineWidth, renderState);
    
    //3
    point = Vector3(bbox.min.x, bbox.max.y, bbox.min.z);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, 0, offs), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, offs, 0), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(offs, 0, 0), lineWidth, renderState);
    
    //4
    point = Vector3(bbox.max.x, bbox.max.y, bbox.min.z);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, 0, offs), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, offs, 0), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(offs, 0, 0), lineWidth, renderState);
    
    //5
    point = Vector3(bbox.max.x, bbox.min.y, bbox.min.z);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, 0, offs), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, offs, 0), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(offs, 0, 0), lineWidth, renderState);
    
    //6
    point = Vector3(bbox.min.x, bbox.max.y, bbox.max.z);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, 0, offs), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, offs, 0), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(offs, 0, 0), lineWidth, renderState);
    
    //7
    point = Vector3(bbox.min.x, bbox.min.y, bbox.max.z);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, 0, offs), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, offs, 0), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(offs, 0, 0), lineWidth, renderState);
    
    //8
    point = Vector3(bbox.max.x, bbox.min.y, bbox.max.z);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, 0, offs), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, offs, 0), lineWidth, renderState);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(offs, 0, 0), lineWidth, renderState);
}
	
	void RenderHelper::DrawSphere(const Vector3 &center, float32 radius, float32 lineWidth, UniqueHandle renderState)
	{
		int32 n = 2;
        Vector<Vector3> points;
        Vector<int32> triangleIndices;

		int32 e;
		float32 segmentRad = PI / (2.0f * ((float32)(n + 1)));
		int32 numberOfSeparators = 4 * n + 4;
				
		for (e = -n; e <= n; e++)
		{
			float32 r_e = radius * cosf(segmentRad * e);
			float32 y_e = radius * sinf(segmentRad * e);
			
			for (int s = 0; s < numberOfSeparators; s++)
			{
				float32 z_s = r_e * sinf(segmentRad * s) * (-1.0f);
				float32 x_s = r_e * cosf(segmentRad * s);
				points.push_back(Vector3(x_s, y_e, z_s));
			}
		}
		points.push_back(Vector3(0, radius, 0));
		points.push_back(Vector3(0, -radius, 0));
		
		for (e = 0; e < 4 * n ; e++)
		{
			for (int i = 0; i < numberOfSeparators; i++)
			{
				triangleIndices.push_back(e * numberOfSeparators + i);
				triangleIndices.push_back(e * numberOfSeparators + i + 
									numberOfSeparators);
				triangleIndices.push_back(e * numberOfSeparators + (i + 1) % 
									numberOfSeparators + numberOfSeparators);
				
				triangleIndices.push_back(e * numberOfSeparators + (i + 1) % 
									numberOfSeparators + numberOfSeparators);
				triangleIndices.push_back(e * numberOfSeparators + 
									(i + 1) % numberOfSeparators);
				triangleIndices.push_back(e * numberOfSeparators + i);
			}
		}
		
//		for (int i = 0; i < numberOfSeparators; i++)
//		{
//			triangleIndices.push_back(e * numberOfSeparators + i);
//			triangleIndices.push_back(numberOfSeparators * (2 * n + 1));
//			triangleIndices.push_back(e * numberOfSeparators + (i + 1) %
//								numberOfSeparators);
//		}
		
//		for (int i = 0; i < numberOfSeparators; i++)
//		{
//			triangleIndices.push_back(i);
//			triangleIndices.push_back((i + 1) % numberOfSeparators);
//			triangleIndices.push_back(numberOfSeparators * (2 * n + 1) + 1);
//		}
		
		
		
		
		
		
		//draw
		
		int32 size = static_cast<int32>(triangleIndices.size()/3);
		for (int i = 0; i < size; i++)
		{
			Vector3 p1 = points[triangleIndices[i]] + center;
			Vector3 p2 = points[triangleIndices[i + 1]] + center;
			Vector3 p3 = points[triangleIndices[i + 2]] + center;
						
			RenderHelper::Instance()->DrawLine(p1, p2, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p1, p3, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p2, p3, lineWidth, renderState);
		
			p1.y = -p1.y;
			p2.y = -p2.y;
			p3.y = -p3.y;
		
			RenderHelper::Instance()->DrawLine(p1, p2, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p1, p3, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p2, p3, lineWidth, renderState);
		}			
	}

	void RenderHelper::FillSphere(const Vector3 &center, float32 radius, UniqueHandle renderState)
	{
		int32 n = 2;
		Vector<Vector3> points;
		Vector<int32> triangleIndices;

		int32 e;
		float32 segmentRad = PI / (2.0f * ((float32)(n + 1)));
		int32 numberOfSeparators = 4 * n + 4;

		for (e = -n; e <= n; e++)
		{
			float32 r_e = radius * cosf(segmentRad * e);
			float32 y_e = radius * sinf(segmentRad * e);

			for (int s = 0; s < numberOfSeparators; s++)
			{
				float32 z_s = r_e * sinf(segmentRad * s) * (-1.0f);
				float32 x_s = r_e * cosf(segmentRad * s);
				points.push_back(Vector3(x_s, y_e, z_s));
			}
		}
		points.push_back(Vector3(0, radius, 0));
		points.push_back(Vector3(0, -radius, 0));

		for (e = 0; e < 4 * n ; e++)
		{
			for (int i = 0; i < numberOfSeparators; i++)
			{
				triangleIndices.push_back(e * numberOfSeparators + i);
				triangleIndices.push_back(e * numberOfSeparators + i + 
					numberOfSeparators);
				triangleIndices.push_back(e * numberOfSeparators + (i + 1) % 
					numberOfSeparators + numberOfSeparators);

				triangleIndices.push_back(e * numberOfSeparators + (i + 1) % 
					numberOfSeparators + numberOfSeparators);
				triangleIndices.push_back(e * numberOfSeparators + 
					(i + 1) % numberOfSeparators);
				triangleIndices.push_back(e * numberOfSeparators + i);
			}
		}

		//fill

		int32 size = static_cast<int32>(triangleIndices.size()/3);
		for (int i = 0; i < size; i++)
		{
			Vector3 p1 = points[triangleIndices[i]] + center;
			Vector3 p2 = points[triangleIndices[i + 1]] + center;
			Vector3 p3 = points[triangleIndices[i + 2]] + center;

			Polygon3 poly;
			poly.AddPoint(p1);
			poly.AddPoint(p3);
			poly.AddPoint(p2);
			RenderHelper::Instance()->FillPolygon(poly, renderState);

			p1.y = 2 * center.y - p1.y;
			p2.y = 2 * center.y - p2.y;
			p3.y = 2 * center.y - p3.y;

			poly.Clear();
			poly.AddPoint(p1);
			poly.AddPoint(p3);
			poly.AddPoint(p2);
			RenderHelper::Instance()->FillPolygon(poly, renderState);
		}			
	}

	void RenderHelper::DrawArrow(const Vector3 &from, const Vector3 &to, float32 arrowLength, float32 lineWidth, UniqueHandle renderState)
	{
		if(0 != lineWidth && from != to)
		{
			Vector3 d = to - from;
			float32 ln = Min(arrowLength, d.Length());

			Vector3 c;
			if(ln < 1)
			{
				c = to - d * ln;
			}
			else
			{
				c = to - d / ln;
			}

			DAVA::float32 k = (to - c).Length() / 4;

			Vector3 n = c.CrossProduct(to);
			n.Normalize();
			n *= k;

			Vector3 p1 = c + n;
			Vector3 p2 = c - n;

			Vector3 nd = d.CrossProduct(n);
			nd.Normalize();
			nd *= k;

			Vector3 p3 = c + nd;
			Vector3 p4 = c - nd;

			RenderHelper::Instance()->DrawLine(from, c, lineWidth, renderState);

			RenderHelper::Instance()->DrawLine(p1, p3, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p2, p3, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p1, p4, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p2, p4, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p1, to, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p2, to, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p3, to, lineWidth, renderState);
			RenderHelper::Instance()->DrawLine(p4, to, lineWidth, renderState);
		}
	}

	void RenderHelper::FillArrow(const Vector3 &from, const Vector3 &to, float32 arrowLength, float32 lineWidth, UniqueHandle renderState)
	{
		Vector3 d = to - from;
		Vector3 c = to - (d * arrowLength / d.Length());

		DAVA::float32 k = arrowLength / 4;

		Vector3 n = c.CrossProduct(to);

		if(n.IsZero())
		{
			if(0 == to.x) n = Vector3(1, 0, 0);
			else if(0 == to.y) n = Vector3(0, 1, 0);
			else if(0 == to.z) n = Vector3(0, 0, 1);
		}

		n.Normalize();
		n *= k;

		Vector3 p1 = c + n;
		Vector3 p2 = c - n;

		Vector3 nd = d.CrossProduct(n);
		nd.Normalize();
		nd *= k;

		Vector3 p3 = c + nd;
		Vector3 p4 = c - nd;

		Polygon3 poly;
        poly.points.reserve(3);
        
		poly.AddPoint(p1);
		poly.AddPoint(p3);
		poly.AddPoint(p2);
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(p1);
		poly.AddPoint(p4);
		poly.AddPoint(p2);
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(p1);
		poly.AddPoint(p3);
		poly.AddPoint(to);
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(p1);
		poly.AddPoint(p4);
		poly.AddPoint(to);
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(p2);
		poly.AddPoint(p3);
		poly.AddPoint(to);
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(p2);
		poly.AddPoint(p4);
		poly.AddPoint(to);
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		if(0 != lineWidth)
		{
			RenderHelper::Instance()->DrawLine(from, c, lineWidth, renderState);
		}
	}

	void RenderHelper::FillBox(const AABBox3 & box, UniqueHandle renderState)
	{
		DAVA::Vector3 min = box.min;
		DAVA::Vector3 max = box.max;

		DAVA::Polygon3 poly;
        poly.points.reserve(4);
        
		poly.AddPoint(min);
		poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
		poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
		poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(min);
		poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
		poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
		poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(min);
		poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
		poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
		poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(max);
		poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
		poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
		poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(max);
		poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
		poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
		poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
		RenderHelper::Instance()->FillPolygon(poly, renderState);

		poly.Clear();
		poly.AddPoint(max);
		poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
		poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
		poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
		RenderHelper::Instance()->FillPolygon(poly, renderState);
	}

	void RenderHelper::DrawDodecahedron(const Vector3 &center, float32 radius, float32 lineWidth /* = 1.f */, UniqueHandle renderState)
    {
        RenderSystem2D::Instance()->Flush();

        RenderSystem2D::Instance()->UpdateClip();

        if (gDodecObject->GetIndexBufferID() != 0)
        {
            gDodecObject->BuildVertexBuffer(sizeof(gDodecVertexes) / sizeof(gDodecVertexes[0]));
            gDodecObject->BuildIndexBuffer();
        }
        
		Matrix4 drawMatrix;
		drawMatrix.CreateScale(DAVA::Vector3(radius, radius, radius));
		drawMatrix.SetTranslationVector(center);

		RenderManager::Instance()->SetDynamicParam(PARAM_WORLD, &drawMatrix, UPDATE_SEMANTIC_ALWAYS);
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(gDodecObject);
		RenderManager::Instance()->AttachRenderData();
        RenderManager::Instance()->FlushState();

		if(gDodecObject->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_LINELIST, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]), EIF_16, 0);
		}
		else
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_LINELIST, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]), EIF_16, gDodecIndexes);
		}
	}

	void RenderHelper::FillDodecahedron(const Vector3 &center, float32 radius, UniqueHandle renderState)
    {
        RenderSystem2D::Instance()->UpdateClip();

        if (gDodecObject->GetIndexBufferID() != 0)
        {
            gDodecObject->BuildVertexBuffer(sizeof(gDodecVertexes) / sizeof(gDodecVertexes[0]));
            gDodecObject->BuildIndexBuffer();
        }

		Matrix4 drawMatrix;
		drawMatrix.CreateScale(DAVA::Vector3(radius, radius, radius));
		drawMatrix.SetTranslationVector(center);

		RenderManager::Instance()->SetDynamicParam(PARAM_WORLD, &drawMatrix, UPDATE_SEMANTIC_ALWAYS);
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(gDodecObject);
		RenderManager::Instance()->AttachRenderData();
        RenderManager::Instance()->FlushState();
        
		if(gDodecObject->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]), EIF_16, 0);
		}
		else
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]), EIF_16, gDodecIndexes);
		}
	}

    void RenderHelper::Set2DRenderTarget(Texture * renderTarget)
    {
        if (!renderTarget)
            return;

        RenderManager::Instance()->SetRenderTarget(renderTarget);
        RenderManager::Instance()->SetViewport(Rect(0.f, 0.f, (float32)renderTarget->GetWidth(), (float32)renderTarget->GetHeight()));

        tempProjectionMatrix.glOrtho(0.0f, (float32)renderTarget->GetWidth(),  0.0f, (float32)renderTarget->GetHeight(), -1.0f, 1.0f);
        RenderManager::SetDynamicParam(PARAM_PROJ, &tempProjectionMatrix, UPDATE_SEMANTIC_ALWAYS);
    }

    void RenderHelper::DrawTexture(Texture * texture, UniqueHandle renderState, const Rect & _dstRect /* = Rect(0.f, 0.f, -1.f, -1.f) */, const Rect & _srcRect /* = Rect(0.f, 0.f, -1.f, -1.f) */)
    {
        if (!texture)
            return;

        RenderSystem2D::Instance()->Flush();
        RenderSystem2D::Instance()->UpdateClip();

        Rect destRect(_dstRect);
        if (destRect.dx < 0.f || destRect.dy < 0.f)
        {
            Size2i targetSize;
            Texture * currentRenderTarget = RenderManager::Instance()->GetRenderTarget();
            if (currentRenderTarget)
            {
                targetSize = Size2i(currentRenderTarget->GetWidth(), currentRenderTarget->GetHeight());
            }
            else
            {
                targetSize = RenderManager::Instance()->GetFramebufferSize();
            }
            destRect.dx = (float32)targetSize.dx;
            destRect.dy = (float32)targetSize.dy;
        }

        vertices[0] = vertices[4] = destRect.x;//x1
        vertices[5] = vertices[7] = destRect.y;//y2
        vertices[1] = vertices[3] = destRect.y + destRect.dy;//y1
        vertices[2] = vertices[6] = destRect.x + destRect.dx;//x2

        Vector2 textureSize = Vector2((float32)texture->GetWidth(), (float32)texture->GetHeight());

        Rect relativeSrcRect;
        relativeSrcRect.x = _srcRect.x / textureSize.dx;
        relativeSrcRect.y = _srcRect.y / textureSize.dy;
        relativeSrcRect.dx = (_srcRect.dx < 0.f) ? 1.f : _srcRect.dx / textureSize.dx;
        relativeSrcRect.dy = (_srcRect.dy < 0.f) ? 1.f : _srcRect.dy / textureSize.dy;

        texCoords[0] = texCoords[4] = relativeSrcRect.x;//x1
        texCoords[5] = texCoords[7] = relativeSrcRect.y;//y2
        texCoords[1] = texCoords[3] = relativeSrcRect.y + relativeSrcRect.dy;//y1
        texCoords[2] = texCoords[6] = relativeSrcRect.x + relativeSrcRect.dx;//x2

        vertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
        texCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords);

        TextureStateData textureStateData;
        textureStateData.SetTexture(0, texture);
        UniqueHandle textureState = RenderManager::Instance()->CreateTextureState(textureStateData);

        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::TEXTURE_MUL_FLAT_COLOR);
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetTextureState(textureState);
        RenderManager::Instance()->SetRenderData(renderDataObject);
        RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);

        RenderManager::Instance()->ReleaseTextureState(textureState);
    }

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
	void RenderHelper::GetLineWidthRange(int32& rangeMin, int32& rangeMax)
	{
		int32 lineWidthMin = 1;
		int32 lineWidthMax = 1;

#if defined (__DAVAENGINE_OPENGL__)
		GLint range[2];
		glGetIntegerv(GL_LINE_WIDTH_RANGE, range);
		lineWidthMin = range[0];
		lineWidthMax = range[1];
#endif

		rangeMin = lineWidthMin;
		rangeMax = lineWidthMax;
	}
#endif
};
