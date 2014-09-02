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


#ifndef __DAVAENGINE_AABBOX3_H__
#define __DAVAENGINE_AABBOX3_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Math/Vector.h"
#include "Math/Ray.h"
#include "Base/Introspection.h"

namespace DAVA
{

#define AABBOX_INFINITY 1000000.0f	

/**
	\brief class to work with axial-aligned bounding boxes
	This class can be used for clipping together with Frustum 
 */
class AABBox3
{
public:
	
	Vector3 min;
	Vector3 max;
	

/*	union
	{
		struct 
		{
			struct
			{
				float32 x, y, z;				
			}min, max;
		};
		struct
		{
			float32 x,y,z;
		} verts[2];
	};*/
		
	
	
	//! \brief construct empty bounding box
	inline AABBox3();

	//! \brief construct bounding box from another bounding box
	//! \param _box another bouding box
	inline AABBox3(const AABBox3 & _box);
	

	//! \brief construct bounding box from 2 points
	//! \param _min min point of bounding box
	//! \param _max max point of bounding box
	inline AABBox3(const Vector3 & _min, const Vector3 & _max);

	
	//! \brief construct bounding box from center & box size
	//! \param center center point of bounding box
	//! \param size size of bounding box
	inline AABBox3(const Vector3 & center, float32 size);

	//! \brief add point to bounding box
	//! if point inside bounding box bounding box not changed
	//! in another case bounding box become larger
	//! \param pt point to add
	inline void AddPoint(const Vector3 & pt);
	inline void AddAABBox(const AABBox3 & bbox);
	
	//! \brief check if bounding box intersect other bounding box
	//! \param box another bounding box
	//! \return true if intersect, false otherwise
	inline bool IntersectsWithBox(const AABBox3 & box) const;

	//! \brief make bounding box empty
	inline void Empty();

	//! \brief Checks is bounding box is empty
	inline bool IsEmpty() const;

	//! \brief check if bounding box intersect line
	inline bool IsIntersectLine(const Vector3 & l1, const Vector3 &l2);
	
	//! \brief check if bounding box intersect ray
	bool IsIntersectsWithRay(Ray3 & r, float32 & tmin, float32 & tmax, float32 t0 = 0.0f, float32 t1 = 1.0f);

	//! \brief check if point inside bbox.
	inline bool IsInside(const Vector3 & pt) const;

	/** 
        \brief Function checks if testBox is fully inside this bounding box. 
        \returns true if testBox is fully inside current bounding box, false otherwise. 
     */
	inline bool IsInside(const AABBox3 & testBox) const;
	
	//! \brief get center
	inline Vector3 GetCenter() const;

    //! \brief get size
    inline Vector3 GetSize() const;

	//! \brief copy operator of bounding box class
	inline AABBox3 & operator =(const AABBox3 & _bbox);

	//! \brief compare operator of bounding box class
	bool operator == (const AABBox3 & _bbox) const { return (min == _bbox.min) && (max == _bbox.max); }


    void GetTransformedBox(const Matrix4 & transform, AABBox3 & result) const;
    void GetCorners(Vector3 * cornersArray) const;
    
public:
	//Dizz: introspection changes    
    //INTROSPECTION(AABBox3,
    //    MEMBER(min, "Min", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    //    MEMBER(max, "Max", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    //);
};

//! \brief construct empty bounding box
inline AABBox3::AABBox3()
{
	min = Vector3(AABBOX_INFINITY, AABBOX_INFINITY, AABBOX_INFINITY);
	max = Vector3(-AABBOX_INFINITY, -AABBOX_INFINITY, -AABBOX_INFINITY);
};

//! \brief construct bounding box from another bounding box
//! \param _box another bouding box
inline AABBox3::AABBox3(const AABBox3 & _box)
{
	min = _box.min;
	max = _box.max;
}

inline AABBox3::AABBox3(const Vector3 & _min, const Vector3 & _max)
{
	min = _min;
	max = _max;
}

inline AABBox3::AABBox3(const Vector3 & center, float32 size)
{
	min = center - Vector3(size / 2.0f, size / 2.0f, size / 2.0f);
	max = center + Vector3(size / 2.0f, size / 2.0f, size / 2.0f);
}
	
inline void AABBox3::AddPoint(const Vector3 & pt)
{
	if (pt.x < min.x)
		min.x = pt.x;
	if (pt.y < min.y)
		min.y = pt.y;
	if (pt.z < min.z)
		min.z = pt.z;

	if (pt.x > max.x)
		max.x = pt.x;
	if (pt.y > max.y)
		max.y = pt.y;
	if (pt.z > max.z)
		max.z = pt.z;
}
	
inline void AABBox3::AddAABBox(const AABBox3 & bbox)
{
	if (bbox.min.x < min.x)
		min.x = bbox.min.x;
	if (bbox.min.y < min.y)
		min.y = bbox.min.y;
	if (bbox.min.z < min.z)
		min.z = bbox.min.z;

	if (bbox.max.x > max.x)
		max.x = bbox.max.x;
	if (bbox.max.y > max.y)
		max.y = bbox.max.y;
	if (bbox.max.z > max.z)
		max.z = bbox.max.z;
}

//! \brief check if bounding box intersect other bounding box
//! \param box another bounding box
//! \return true if intersect, false otherwise
inline bool AABBox3::IntersectsWithBox(const AABBox3 & other) const
{
	return (min.x <= other.max.x && min.y <= other.max.y && min.z <= other.max.z &&
            max.x >= other.min.x && max.y >= other.min.y && max.z >= other.min.z);
};

//! \brief make bounding box empty
inline void AABBox3::Empty()
{
	min = Vector3(AABBOX_INFINITY, AABBOX_INFINITY, AABBOX_INFINITY);
	max = Vector3(-AABBOX_INFINITY, -AABBOX_INFINITY, -AABBOX_INFINITY);
}

inline bool AABBox3::IsEmpty() const
{
	return (min.x > max.x || min.y > max.y || min.z > max.z);
}

//! \brief check if bounding box intersect line
inline bool IsIntersectLine(const Vector3 & /*l1*/, const Vector3 & /*l2*/)
{
	//float32 tmin[3];
	//float32 tmax[3];
	//
	//Vector3 center = (min + max) / 2.0f;
	//Vector3 p = center  - l1;
	//
	//for (int i = 0; i < 3; ++i)
	//{
	//	float32 e = 
	//	float32 d = 
	//}
	return false;
}

inline bool AABBox3::IsInside(const Vector3 & pt) const
{
	if (
		(min.x <= pt.x)
		&&(min.y <= pt.y)
		&&(min.z <= pt.z)
		&&(pt.x <= max.x)
		&&(pt.y <= max.y)
		&&(pt.z <= max.z))return true;

	return false;
}
    
inline bool AABBox3::IsInside(const AABBox3 & testBox) const
{
	if (
            (min.x <= testBox.min.x)
		&&  (min.y <= testBox.min.y)
		&&  (min.z <= testBox.min.z)
		&&  (testBox.max.x <= max.x)
		&&  (testBox.max.y <= max.y)
		&&  (testBox.max.z <= max.z))return true;
    
    return false;
}

    
    

//! \brief copy operator of bounding box class
inline AABBox3 & AABBox3::operator =(const AABBox3 & _bbox)
{
	min = _bbox.min;
	max = _bbox.max;
	return *this;
}

//! \brief get center
inline Vector3 AABBox3::GetCenter() const
{
	return (min + max) / 2.0f;
}

inline Vector3 AABBox3::GetSize() const
{
    return Vector3(Abs(max.x - min.x), Abs(max.y - min.y), Abs(max.z - min.z));
}


};

#endif // __DAVAENGINE_AABBOX3_H__

