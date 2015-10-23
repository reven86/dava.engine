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


#ifndef __DAVAENGINE_FRUSTUM_H__
#define __DAVAENGINE_FRUSTUM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Math/AABBox3.h"
#include "Math/Plane.h"

namespace DAVA
{
	
/** 
    \brief Frustum class to detect objects visibility
    This class is supposed for testing objects for visibility
    This class is multiplane frustum
 
    Main logical question is why frustum is related to render, and not in Math or Scene3D.
    The answer is simple: I assume that culling code can differ for OGL, DX matrices. Let's see when we'll add DirectX am I right.  
*/
class RenderHelper;
class Frustum : public BaseObject
{	

public:

    enum eFrustumPlane 
    {
        EFP_LEFT = 0,
        EFP_RIGHT,
        EFP_BOTTOM,
        EFP_TOP,
        EFP_NEAR,
        EFP_FAR,
    };

	enum eFrustumResult
	{
		EFR_INSIDE = 0x0,
		EFR_OUTSIDE = 0x1,
		EFR_INTERSECT = 0x2,
	};

protected:
	~Frustum();
public:
	Frustum();

 
    //! \brief Set view frustum from matrix information
	//! \param viewProjection view * projection matrix
    void Build(const Matrix4 & viewProjection);
        
	//! \brief Check axial aligned bounding box visibility
	//! \param min bounding box minimum point
	//! \param max bounding box maximum point
	//! \return true if inside
	bool IsInside(const Vector3 & min, const Vector3 &max) const;

	//! \brief Check axial aligned bounding box visibility
	//! \param box bounding box
	bool IsInside(const AABBox3 & box)const;

	//! \brief Check axial aligned bounding box visibility
	//! \param box bounding box
	bool IsInside(const AABBox3 * box)const;

	//! \brief Check axial aligned bounding box visibility with plane mask and prefferd plane
	//! \param box bounding box
	// unlike Classify this function do not modify plane masking as, though still modify startClippingPlane
	bool IsInside(const AABBox3 & box, uint8 planeMask, uint8& startClippingPlane)const;

    //! \brief Check axial aligned bounding box visibility
	//! \param box bounding box
	bool IsFullyInside(const AABBox3 & box)const;

    // *********************************************
    // All above require detailed testing !!! Never tested in real project!!!
    // *********************************************
    
	//! \brief Check axial aligned bounding box visibility
	//! \param min bounding box minimum point
	//! \param max bounding box maximum point
	//! \return \ref eFrustumResult to classify intersection
	eFrustumResult Classify(const Vector3 & min, const Vector3 &max) const;
	
	//! \brief Check axial aligned bounding box visibility
	//! \param min bounding box minimum point
	//! \param max bounding box maximum point
	//! \return \ref eFrustumResult to classify intersection
	eFrustumResult Classify(const AABBox3 & box) const;

	//checks only planes mentioned in [io]planeMask starting with [io]startId
	//if box is completely inside planes subspace - plane is removed from planeMask
	//if box is clipped by plane startId is set to this plane
	eFrustumResult Classify(const AABBox3 & box, uint8 &planeMask, uint8 &startId) const;

	//! \brief check bounding sphere visibility against frustum
	//! \param point sphere center point
	//! \param radius sphere radius
	bool IsInside(const Vector3 & point, const float32 radius) const;


	//! \brief function return real plane count in this frustum
	inline int32 GetPlaneCount()
    {
        return planeCount;
    }
	
	//! \brief function return plane with index 
	//! \param i index of plane we want to get
	inline Plane & GetPlane(int32 i)
    {
        return planeArray[i];
    }


	// 
	void DebugDraw(RenderHelper * drawer);
	

private:
	int32					planeCount;
	uint32 planeAccesBits;
	Plane  planeArray[6];	

};

};

#endif // __DAVAENGINE_FRUSTUM_H__

