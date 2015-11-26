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


#ifndef GIM_CLIP_POLYGON_H_INCLUDED
#define GIM_CLIP_POLYGON_H_INCLUDED

/*! \file gim_tri_collision.h
\author Francisco Leon Najera
*/
/*
-----------------------------------------------------------------------------
This source file is part of GIMPACT Library.

For the latest info, see http://gimpact.sourceforge.net/

Copyright (c) 2006 Francisco Leon Najera. C.C. 80087371.
email: projectileman@yahoo.com

 This library is free software; you can redistribute it and/or
 modify it under the terms of EITHER:
   (1) The GNU Lesser General Public License as published by the Free
       Software Foundation; either version 2.1 of the License, or (at
       your option) any later version. The text of the GNU Lesser
       General Public License is included with this library in the
       file GIMPACT-LICENSE-LGPL.TXT.
   (2) The BSD-style license that is included with this library in
       the file GIMPACT-LICENSE-BSD.TXT.
   (3) The zlib/libpng license that is included with this library in
       the file GIMPACT-LICENSE-ZLIB.TXT.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 GIMPACT-LICENSE-LGPL.TXT, GIMPACT-LICENSE-ZLIB.TXT and GIMPACT-LICENSE-BSD.TXT for more details.

-----------------------------------------------------------------------------
*/


//! This function calcs the distance from a 3D plane
class DISTANCE_PLANE_3D_FUNC
{
public:
	template<typename CLASS_POINT,typename CLASS_PLANE>
	inline GREAL operator()(const CLASS_PLANE & plane, const CLASS_POINT & point)
	{
		return DISTANCE_PLANE_POINT(plane, point);
	}
};



template<typename CLASS_POINT>
SIMD_FORCE_INLINE void PLANE_CLIP_POLYGON_COLLECT(
						const CLASS_POINT & point0,
						const CLASS_POINT & point1,
						GREAL dist0,
						GREAL dist1,
						CLASS_POINT * clipped,
						GUINT & clipped_count)
{
	GUINT _prevclassif = (dist0>G_EPSILON);
	GUINT _classif = (dist1>G_EPSILON);
	if(_classif!=_prevclassif)
	{
		GREAL blendfactor = -dist0/(dist1-dist0);
		VEC_BLEND(clipped[clipped_count],point0,point1,blendfactor);
		clipped_count++;
	}
	if(!_classif)
	{
		VEC_COPY(clipped[clipped_count],point1);
		clipped_count++;
	}
}


//! Clips a polygon by a plane
/*!
*\return The count of the clipped counts
*/
template<typename CLASS_POINT,typename CLASS_PLANE, typename DISTANCE_PLANE_FUNC>
SIMD_FORCE_INLINE GUINT PLANE_CLIP_POLYGON_GENERIC(
						const CLASS_PLANE & plane,
						const CLASS_POINT * polygon_points,
						GUINT polygon_point_count,
						CLASS_POINT * clipped,DISTANCE_PLANE_FUNC distance_func)
{
    GUINT clipped_count = 0;


    //clip first point
	GREAL firstdist = distance_func(plane,polygon_points[0]);;
	if(!(firstdist>G_EPSILON))
	{
		VEC_COPY(clipped[clipped_count],polygon_points[0]);
		clipped_count++;
	}

	GREAL olddist = firstdist;
	for(GUINT _i=1;_i<polygon_point_count;_i++)
	{		
		GREAL dist = distance_func(plane,polygon_points[_i]);

		PLANE_CLIP_POLYGON_COLLECT(
						polygon_points[_i-1],polygon_points[_i],
						olddist,
						dist,
						clipped,
						clipped_count);


		olddist = dist;		
	}

	//RETURN TO FIRST  point	

	PLANE_CLIP_POLYGON_COLLECT(
					polygon_points[polygon_point_count-1],polygon_points[0],
					olddist,
					firstdist,
					clipped,
					clipped_count);

	return clipped_count;
}

//! Clips a polygon by a plane
/*!
*\return The count of the clipped counts
*/
template<typename CLASS_POINT,typename CLASS_PLANE, typename DISTANCE_PLANE_FUNC>
SIMD_FORCE_INLINE GUINT PLANE_CLIP_TRIANGLE_GENERIC(
						const CLASS_PLANE & plane,
						const CLASS_POINT & point0,
						const CLASS_POINT & point1,
						const CLASS_POINT & point2,
						CLASS_POINT * clipped,DISTANCE_PLANE_FUNC distance_func)
{
    GUINT clipped_count = 0;

    //clip first point
	GREAL firstdist = distance_func(plane,point0);;
	if(!(firstdist>G_EPSILON))
	{
		VEC_COPY(clipped[clipped_count],point0);
		clipped_count++;
	}

	// point 1
	GREAL olddist = firstdist;
	GREAL dist = distance_func(plane,point1);

	PLANE_CLIP_POLYGON_COLLECT(
					point0,point1,
					olddist,
					dist,
					clipped,
					clipped_count);

	olddist = dist;


	// point 2
	dist = distance_func(plane,point2);

	PLANE_CLIP_POLYGON_COLLECT(
					point1,point2,
					olddist,
					dist,
					clipped,
					clipped_count);
	olddist = dist;



	//RETURN TO FIRST  point
	PLANE_CLIP_POLYGON_COLLECT(
					point2,point0,
					olddist,
					firstdist,
					clipped,
					clipped_count);

	return clipped_count;
}


template<typename CLASS_POINT,typename CLASS_PLANE>
SIMD_FORCE_INLINE GUINT PLANE_CLIP_POLYGON3D(
						const CLASS_PLANE & plane,
						const CLASS_POINT * polygon_points,
						GUINT polygon_point_count,
						CLASS_POINT * clipped)
{
	return PLANE_CLIP_POLYGON_GENERIC<CLASS_POINT,CLASS_PLANE>(plane,polygon_points,polygon_point_count,clipped,DISTANCE_PLANE_3D_FUNC());
}


template<typename CLASS_POINT,typename CLASS_PLANE>
SIMD_FORCE_INLINE GUINT PLANE_CLIP_TRIANGLE3D(
						const CLASS_PLANE & plane,
						const CLASS_POINT & point0,
						const CLASS_POINT & point1,
						const CLASS_POINT & point2,
						CLASS_POINT * clipped)
{
	return PLANE_CLIP_TRIANGLE_GENERIC<CLASS_POINT,CLASS_PLANE>(plane,point0,point1,point2,clipped,DISTANCE_PLANE_3D_FUNC());
}



#endif // GIM_TRI_COLLISION_H_INCLUDED
