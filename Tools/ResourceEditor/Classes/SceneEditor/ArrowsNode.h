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



#ifndef __RESOURCEEDITORQT__ARROWSNODE__
#define __RESOURCEEDITORQT__ARROWSNODE__

#include "DAVAEngine.h"
#include "BulletObject.h"

using namespace DAVA;

class ArrowsNode;

class ArrowsRenderBatch: public RenderBatch
{
protected:
	enum eAxisColors
	{
		COLOR_X = 0,
		COLOR_Y,
		COLOR_Z,
		COLOR_XY_X,
		COLOR_XY_Y,
		COLOR_YZ_Y,
		COLOR_YZ_Z,
		COLOR_XZ_X,
		COLOR_XZ_Z,
		
		COLORS_COUNT
	};

public:
	ArrowsRenderBatch(ArrowsNode* node);

	const FastName & GetOwnerLayerName();
	virtual void Draw(Camera * camera);

protected:
	ArrowsNode* node;

	void PrepareColors(Color* colors);
	void DrawPrism(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4, const Vector3& p5);
};

class ArrowsNode: public Entity
{
public:
	enum eModAxis
    {
        AXIS_X = 0,
        AXIS_Y,
        AXIS_Z,
        AXIS_XY,
        AXIS_YZ,
        AXIS_XZ,
		AXIS_NONE,
		AXIS_COUNT = AXIS_NONE
	};

public:
	ArrowsNode();
	virtual ~ArrowsNode();

	virtual void ProcessMouse(UIEvent * event, const Vector3& cursorPos, const Vector3& cursorDir);

	void SetActive(bool active);
	bool IsActive();

	eModAxis GetModAxis();

private:
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* collisionDispatcher;
	btAxisSweep3* axisSweep;
	btCollisionWorld* collisionWorld;

	uint32 selected;
	bool active;
};

#endif /* defined(__RESOURCEEDITORQT__ARROWSNODE__) */
