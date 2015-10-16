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


/*
Copyright (c) 2003-2006 Gino van den Bergen / Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/



#ifndef BT_VECTOR3_H
#define BT_VECTOR3_H


#include "btScalar.h"
#include "btMinMax.h"

#ifdef BT_USE_DOUBLE_PRECISION
#define btVector3Data btVector3DoubleData
#define btVector3DataName "btVector3DoubleData"
#else
#define btVector3Data btVector3FloatData
#define btVector3DataName "btVector3FloatData"
#endif //BT_USE_DOUBLE_PRECISION



/**@brief btVector3 can be used to represent 3D points and vectors.
 * It has an un-used w component to suit 16-byte alignment when btVector3 is stored in containers. This extra component can be used by derived classes (Quaternion?) or by user
 * Ideally, this class should be replaced by a platform optimized SIMD version that keeps the data in registers
 */
ATTRIBUTE_ALIGNED16(class) btVector3
{
public:

#if defined (__SPU__) && defined (__CELLOS_LV2__)
		btScalar	m_floats[4];
public:
	SIMD_FORCE_INLINE const vec_float4&	get128() const
	{
		return *((const vec_float4*)&m_floats[0]);
	}
public:
#else //__CELLOS_LV2__ __SPU__
#ifdef BT_USE_SSE
    union {
		__m128 mVec128;
        btScalar m_floats[4];
    };
	SIMD_FORCE_INLINE	__m128	get128() const
	{
		return mVec128;
	}
	SIMD_FORCE_INLINE	void	set128(__m128 v128)
	{
		mVec128 = v128;
	}
#else
	btScalar	m_floats[4];
#endif
#endif //__CELLOS_LV2__ __SPU__

	public:

  /**@brief No initialization constructor */
	SIMD_FORCE_INLINE btVector3() {}

 
	
  /**@brief Constructor from scalars 
   * @param x X value
   * @param y Y value 
   * @param z Z value 
   */
	SIMD_FORCE_INLINE btVector3(const btScalar& x, const btScalar& y, const btScalar& z)
	{
#ifdef BT_USE_SSE
        mVec128 = _mm_set_ps(0.0f, z, y, x);
#else
        m_floats[0] = x;
		m_floats[1] = y;
		m_floats[2] = z;
		m_floats[3] = btScalar(0.);
#endif
    }

    SIMD_FORCE_INLINE btVector3(const btScalar& x, const btScalar& y, const btScalar& z, const btScalar& w)
    {
#ifdef BT_USE_SSE
        mVec128 = _mm_set_ps(w, z, y, x);
#else
        m_floats[0] = x;
        m_floats[1] = y;
        m_floats[2] = z;
        m_floats[3] = w;
#endif
    }

#ifdef BT_USE_SSE
    SIMD_FORCE_INLINE btVector3(const __m128& v)
        : mVec128(v)
    {
    }
#endif

/**@brief Add a vector to this one 
 * @param The vector to add to this one */
	SIMD_FORCE_INLINE btVector3& operator+=(const btVector3& v)
	{
#ifdef BT_USE_SSE
        mVec128 = _mm_add_ps(mVec128, v.mVec128);
#else
        m_floats[0] += v.m_floats[0]; m_floats[1] += v.m_floats[1];m_floats[2] += v.m_floats[2];
#endif
        return *this;
	}


  /**@brief Subtract a vector from this one
   * @param The vector to subtract */
	SIMD_FORCE_INLINE btVector3& operator-=(const btVector3& v) 
	{
#ifdef BT_USE_SSE
        mVec128 = _mm_sub_ps(mVec128, v.mVec128);
#else
        m_floats[0] -= v.m_floats[0]; m_floats[1] -= v.m_floats[1];m_floats[2] -= v.m_floats[2];
#endif
        return *this;
	}
  /**@brief Scale the vector
   * @param s Scale factor */
	SIMD_FORCE_INLINE btVector3& operator*=(const btScalar& s)
	{
		m_floats[0] *= s; m_floats[1] *= s;m_floats[2] *= s;
		return *this;
	}

  /**@brief Inversely scale the vector 
   * @param s Scale factor to divide by */
	SIMD_FORCE_INLINE btVector3& operator/=(const btScalar& s) 
	{
		btFullAssert(s != btScalar(0.0));
		return *this *= btScalar(1.0) / s;
	}

  /**@brief Return the dot product
   * @param v The other vector in the dot product */
	SIMD_FORCE_INLINE btScalar dot(const btVector3& v) const
	{
#ifdef BT_USE_SSE
        auto m = _mm_mul_ps(mVec128, v.mVec128);
        auto t = _mm_add_ps(m, _mm_shuffle_ps(m, m, _MM_SHUFFLE(0, 3, 2, 1)));
        return _mm_cvtss_f32(_mm_add_ps(t, _mm_shuffle_ps(t, t, _MM_SHUFFLE(1, 0, 3, 2))));
#else
        return m_floats[0] * v.m_floats[0] + m_floats[1] * v.m_floats[1] +m_floats[2] * v.m_floats[2];
#endif

//			// Note that this inline assembly does NOT work correctly though it is *identical* to the dedicated assembly
//			// version and has (in theory) properly assigned register constraints and clobbers. In fact, it produces a
//			// correct numerical result, but gcc seems to not properly manage registers and this causes trouble downstream.
//			// Forum discussions indicate gcc is not great at supporting inline assembly using NEON registers. Even the
//			// two different but theoretically equivalent clobber lists produce different simulation results!
//		float __attribute__((__aligned__((64)))) result[2]; // space for 2 floats since neon vst1.32 wants to store a double register
//		asm volatile
//		(
//		 // Note this code is explicitly using register q0-q2 (and corresponding d/s registers)
//		 // so that we can avoid the need to preserve them. We are applying instructions directly
//		 // to s registers, which prevents us from using q8 and above, and would rather not
//		 // use registers that gcc expects us to preserve, to avoid the cost of doing that.
//		 "vld1.32 {d0,d1}, [%1]		@ input <x2,y2,z2,?> = d0,d1\n\t"
//		 "vld1.32 {d2,d3}, [%0]		@ input <x1,y1,z1,?> = d2,d3\n\t"
//		 "vmul.f32 d4, d0, d2		@ d4 = <x1*x2,y1*y2>\n\t"
//		 "vpadd.f32 d4, d4, d4		@ d4 = <x1*x2 + y1*y2, x1*x2 + y1*y2>\n\t"
//		 "vmla.f32 s8, s2, s6		@ s8 = <x1*x2 + y1*y2 + z1*z2\n\t"
//		 "vst1.32 {d4}, [%2]			@ save result to memory. supports double/quad word only. We only care about first word\n\t"
//		 : // NO explicit outputs!
//		 : "r" (this), "r" (&v), "+r" (&result[0])	// inputs. output directly stored in result
//		 //		: "memory", "d0", "d1", "d2", "d3", "d4"	// clobbers - this version causes chaos
//		 : "memory", "q0", "q1", "d4"				// clobbers - this version is stable but simulation is still incorrect
//		 );
//		
//		return(result[0]);
	}

  /**@brief Return the length of the vector squared */
	SIMD_FORCE_INLINE btScalar length2() const
	{
		return dot(*this);
	}

  /**@brief Return the length of the vector */
	SIMD_FORCE_INLINE btScalar length() const
	{
		return btSqrt(length2());
	}

  /**@brief Return the distance squared between the ends of this and another vector
   * This is symantically treating the vector like a point */
	SIMD_FORCE_INLINE btScalar distance2(const btVector3& v) const;

  /**@brief Return the distance between the ends of this and another vector
   * This is symantically treating the vector like a point */
	SIMD_FORCE_INLINE btScalar distance(const btVector3& v) const;

	SIMD_FORCE_INLINE btVector3& safeNormalize() 
	{
		btVector3 absVec = this->absolute();
		int maxIndex = absVec.maxAxis();
		if (absVec[maxIndex]>0)
		{
			*this /= absVec[maxIndex];
			return *this /= length();
		}
		setValue(1,0,0);
		return *this;
	}

  /**@brief Normalize this vector 
   * x^2 + y^2 + z^2 = 1 */
	SIMD_FORCE_INLINE btVector3& normalize() 
	{
		return *this /= length();
	}

  /**@brief Return a normalized version of this vector */
	SIMD_FORCE_INLINE btVector3 normalized() const;

  /**@brief Return a rotated version of this vector
   * @param wAxis The axis to rotate about 
   * @param angle The angle to rotate by */
	SIMD_FORCE_INLINE btVector3 rotate( const btVector3& wAxis, const btScalar angle ) const;

  /**@brief Return the angle between this and another vector
   * @param v The other vector */
	SIMD_FORCE_INLINE btScalar angle(const btVector3& v) const 
	{
		btScalar s = btSqrt(length2() * v.length2());
		btFullAssert(s != btScalar(0.0));
		return btAcos(dot(v) / s);
	}
  /**@brief Return a vector will the absolute values of each element */
	SIMD_FORCE_INLINE btVector3 absolute() const 
	{
		return btVector3(
			btFabs(m_floats[0]), 
			btFabs(m_floats[1]), 
			btFabs(m_floats[2]));
	}
  /**@brief Return the cross product between this and another vector 
   * @param v The other vector */
	SIMD_FORCE_INLINE btVector3 cross(const btVector3& v) const
	{
#ifdef BT_USE_SSE
        __m128 result = _mm_sub_ps(
        _mm_mul_ps(mVec128, _mm_shuffle_ps(v.mVec128, v.mVec128, _MM_SHUFFLE(3, 0, 2, 1))),
        _mm_mul_ps(v.mVec128, _mm_shuffle_ps(mVec128, mVec128, _MM_SHUFFLE(3, 0, 2, 1))));
        return btVector3(_mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 0, 2, 1)));
#else
        return btVector3(
        m_floats[1] * v.m_floats[2] - m_floats[2] * v.m_floats[1],
        m_floats[2] * v.m_floats[0] - m_floats[0] * v.m_floats[2],
        m_floats[0] * v.m_floats[1] - m_floats[1] * v.m_floats[0]);
#endif
//			// Note that this inline assembly function does seem to work okay;
//			// however, given trouble produced elsewhere (e.g., see Vector3
//			// dot product in this file), it is probably best to call the pure
//			// assembly function
//		btVector3 __attribute__((__aligned__((64)))) result;
//		asm volatile
//		(
//		 "vld1.32 {d18,d19}, [%1]	@ input <x2,y2,z2,w2> = d18,d19\n\t"
//		 "vld1.32 {d16,d17}, [%0]	@ input <x1,y1,z1,w1> = d16,d17\n\t"
//		 "vtrn.32 d18,d19			@  q9 = <x2,z2,y2,w2> = d18,d19\n\t"
//		 "vrev64.32 d16,d16			@  q8 = <y1,x1,z1,w1> = d16,d17\n\t"
//		 "vrev64.32 d18,d18			@  q9 = <z2,x2,y2,w2> = d18,d19\n\t"
//		 "vtrn.32 d16,d17			@  q8 = <y1,z1,x1,w1> = d16,d17\n\t"
//		 "vmul.f32 q10, q8, q9		@ q10 = <y1*z2,z1*x2,x1*y2,w1*w2>\n\t"
//		 "vtrn.32 d18,d19			@  q9 = <z2,y2,x2,w2> = d18,d19\n\t"
//		 "vrev64.32 d16,d16			@  q8 = <z1,y1,x1,w1> = d16,d17\n\t"
//		 "vrev64.32 d18,d18			@  q9 = <y2,z2,x2,w2> = d18,d19\n\t"
//		 "vtrn.32 d16,d17			@  q8 = <z1,x1,y1,w1> = d16,d17\n\t"
//		 "vmls.f32 q10, q8, q9		@ q10 = <y1*z2-y2*z1,z1*x2-z2*x1,x1*y2-x2*y1,w1*w2-w2*w1>\n\t"
//		 "vst1.32 {q10}, [%2]\n\t"
//		 : // NO explicit outputs!
//		 : "r" (this), "r" (&v), "r" (&result) //inputs. output indirectly stored in result
//		 : "memory", "q8", "q9", "q10" // clobbers
//		 );
//		return result;
	}

	SIMD_FORCE_INLINE btScalar triple(const btVector3& v1, const btVector3& v2) const
	{
		return m_floats[0] * (v1.m_floats[1] * v2.m_floats[2] - v1.m_floats[2] * v2.m_floats[1]) + 
			m_floats[1] * (v1.m_floats[2] * v2.m_floats[0] - v1.m_floats[0] * v2.m_floats[2]) + 
			m_floats[2] * (v1.m_floats[0] * v2.m_floats[1] - v1.m_floats[1] * v2.m_floats[0]);
	}

  /**@brief Return the axis with the smallest value 
   * Note return values are 0,1,2 for x, y, or z */
	SIMD_FORCE_INLINE int minAxis() const
	{
		return m_floats[0] < m_floats[1] ? (m_floats[0] <m_floats[2] ? 0 : 2) : (m_floats[1] <m_floats[2] ? 1 : 2);
	}

  /**@brief Return the axis with the largest value 
   * Note return values are 0,1,2 for x, y, or z */
	SIMD_FORCE_INLINE int maxAxis() const 
	{
		return m_floats[0] < m_floats[1] ? (m_floats[1] <m_floats[2] ? 2 : 1) : (m_floats[0] <m_floats[2] ? 2 : 0);
	}

	SIMD_FORCE_INLINE int furthestAxis() const
	{
		return absolute().minAxis();
	}

	SIMD_FORCE_INLINE int closestAxis() const 
	{
		return absolute().maxAxis();
	}

	SIMD_FORCE_INLINE void setInterpolate3(const btVector3& v0, const btVector3& v1, btScalar rt)
	{
		btScalar s = btScalar(1.0) - rt;
		m_floats[0] = s * v0.m_floats[0] + rt * v1.m_floats[0];
		m_floats[1] = s * v0.m_floats[1] + rt * v1.m_floats[1];
		m_floats[2] = s * v0.m_floats[2] + rt * v1.m_floats[2];
		//don't do the unused w component
		//		m_co[3] = s * v0[3] + rt * v1[3];
	}

  /**@brief Return the linear interpolation between this and another vector 
   * @param v The other vector 
   * @param t The ration of this to v (t = 0 => return this, t=1 => return other) */
	SIMD_FORCE_INLINE btVector3 lerp(const btVector3& v, const btScalar& t) const 
	{
		return btVector3(m_floats[0] + (v.m_floats[0] - m_floats[0]) * t,
			m_floats[1] + (v.m_floats[1] - m_floats[1]) * t,
			m_floats[2] + (v.m_floats[2] -m_floats[2]) * t);
	}

  /**@brief Elementwise multiply this vector by the other 
   * @param v The other vector */
	SIMD_FORCE_INLINE btVector3& operator*=(const btVector3& v)
	{
#ifdef BT_USE_SSE
        mVec128 = _mm_mul_ps(mVec128, v.mVec128);
#else
        m_floats[0] *= v.m_floats[0]; m_floats[1] *= v.m_floats[1];m_floats[2] *= v.m_floats[2];
#endif
        return *this;
	}

	 /**@brief Return the x value */
		SIMD_FORCE_INLINE const btScalar& getX() const { return m_floats[0]; }
  /**@brief Return the y value */
		SIMD_FORCE_INLINE const btScalar& getY() const { return m_floats[1]; }
  /**@brief Return the z value */
		SIMD_FORCE_INLINE const btScalar& getZ() const { return m_floats[2]; }
  /**@brief Set the x value */
		SIMD_FORCE_INLINE void	setX(btScalar x) { m_floats[0] = x;};
  /**@brief Set the y value */
		SIMD_FORCE_INLINE void	setY(btScalar y) { m_floats[1] = y;};
  /**@brief Set the z value */
		SIMD_FORCE_INLINE void	setZ(btScalar z) {m_floats[2] = z;};
  /**@brief Set the w value */
		SIMD_FORCE_INLINE void	setW(btScalar w) { m_floats[3] = w;};
  /**@brief Return the x value */
		SIMD_FORCE_INLINE const btScalar& x() const { return m_floats[0]; }
  /**@brief Return the y value */
		SIMD_FORCE_INLINE const btScalar& y() const { return m_floats[1]; }
  /**@brief Return the z value */
		SIMD_FORCE_INLINE const btScalar& z() const { return m_floats[2]; }
  /**@brief Return the w value */
		SIMD_FORCE_INLINE const btScalar& w() const { return m_floats[3]; }

	//SIMD_FORCE_INLINE btScalar&       operator[](int i)       { return (&m_floats[0])[i];	}      
	//SIMD_FORCE_INLINE const btScalar& operator[](int i) const { return (&m_floats[0])[i]; }
	///operator btScalar*() replaces operator[], using implicit conversion. We added operator != and operator == to avoid pointer comparisons.
	SIMD_FORCE_INLINE	operator       btScalar *()       { return &m_floats[0]; }
	SIMD_FORCE_INLINE	operator const btScalar *() const { return &m_floats[0]; }

	SIMD_FORCE_INLINE	bool	operator==(const btVector3& other) const
	{
		return ((m_floats[3]==other.m_floats[3]) && (m_floats[2]==other.m_floats[2]) && (m_floats[1]==other.m_floats[1]) && (m_floats[0]==other.m_floats[0]));
	}

	SIMD_FORCE_INLINE	bool	operator!=(const btVector3& other) const
	{
		return !(*this == other);
	}

	 /**@brief Set each element to the max of the current values and the values of another btVector3
   * @param other The other btVector3 to compare with 
   */
		SIMD_FORCE_INLINE void	setMax(const btVector3& other)
		{
			btSetMax(m_floats[0], other.m_floats[0]);
			btSetMax(m_floats[1], other.m_floats[1]);
			btSetMax(m_floats[2], other.m_floats[2]);
			btSetMax(m_floats[3], other.w());
		}
  /**@brief Set each element to the min of the current values and the values of another btVector3
   * @param other The other btVector3 to compare with 
   */
		SIMD_FORCE_INLINE void	setMin(const btVector3& other)
		{
			btSetMin(m_floats[0], other.m_floats[0]);
			btSetMin(m_floats[1], other.m_floats[1]);
			btSetMin(m_floats[2], other.m_floats[2]);
			btSetMin(m_floats[3], other.w());
		}

		SIMD_FORCE_INLINE void 	setValue(const btScalar& x, const btScalar& y, const btScalar& z)
		{
			m_floats[0]=x;
			m_floats[1]=y;
			m_floats[2]=z;
			m_floats[3] = btScalar(0.);
		}

		void	getSkewSymmetricMatrix(btVector3* v0,btVector3* v1,btVector3* v2) const
		{
			v0->setValue(0.		,-z()		,y());
			v1->setValue(z()	,0.			,-x());
			v2->setValue(-y()	,x()	,0.);
		}

		void	setZero()
		{
			setValue(btScalar(0.),btScalar(0.),btScalar(0.));
		}

		SIMD_FORCE_INLINE bool isZero() const 
		{
			return m_floats[0] == btScalar(0) && m_floats[1] == btScalar(0) && m_floats[2] == btScalar(0);
		}

		SIMD_FORCE_INLINE bool fuzzyZero() const 
		{
			return length2() < SIMD_EPSILON;
		}

		SIMD_FORCE_INLINE	void	serialize(struct	btVector3Data& dataOut) const;

		SIMD_FORCE_INLINE	void	deSerialize(const struct	btVector3Data& dataIn);

		SIMD_FORCE_INLINE	void	serializeFloat(struct	btVector3FloatData& dataOut) const;

		SIMD_FORCE_INLINE	void	deSerializeFloat(const struct	btVector3FloatData& dataIn);

		SIMD_FORCE_INLINE	void	serializeDouble(struct	btVector3DoubleData& dataOut) const;

		SIMD_FORCE_INLINE	void	deSerializeDouble(const struct	btVector3DoubleData& dataIn);

};

/**@brief Return the sum of two vectors (Point symantics)*/
SIMD_FORCE_INLINE btVector3 
operator+(const btVector3& v1, const btVector3& v2) 
{
#ifdef BT_USE_SSE
    return btVector3(_mm_add_ps(v1.mVec128, v2.mVec128));
#else
    return btVector3(v1.m_floats[0] + v2.m_floats[0], v1.m_floats[1] + v2.m_floats[1], v1.m_floats[2] + v2.m_floats[2]);
#endif
}

/**@brief Return the elementwise product of two vectors */
SIMD_FORCE_INLINE btVector3 
operator*(const btVector3& v1, const btVector3& v2) 
{
#ifdef BT_USE_SSE
    return btVector3(_mm_mul_ps(v1.mVec128, v2.mVec128));
#else
    return btVector3(v1.m_floats[0] * v2.m_floats[0], v1.m_floats[1] * v2.m_floats[1], v1.m_floats[2] * v2.m_floats[2]);
#endif
}

/**@brief Return the difference between two vectors */
SIMD_FORCE_INLINE btVector3 
operator-(const btVector3& v1, const btVector3& v2)
{
#ifdef BT_USE_SSE
    return btVector3(_mm_sub_ps(v1.mVec128, v2.mVec128));
#else
    return btVector3(v1.m_floats[0] - v2.m_floats[0], v1.m_floats[1] - v2.m_floats[1], v1.m_floats[2] - v2.m_floats[2]);
#endif
}
/**@brief Return the negative of the vector */
SIMD_FORCE_INLINE btVector3 
operator-(const btVector3& v)
{
	return btVector3(-v.m_floats[0], -v.m_floats[1], -v.m_floats[2]);
}

/**@brief Return the vector scaled by s */
SIMD_FORCE_INLINE btVector3 
operator*(const btVector3& v, const btScalar& s)
{
	return btVector3(v.m_floats[0] * s, v.m_floats[1] * s, v.m_floats[2] * s);
}

/**@brief Return the vector scaled by s */
SIMD_FORCE_INLINE btVector3 
operator*(const btScalar& s, const btVector3& v)
{ 
	return v * s; 
}

/**@brief Return the vector inversely scaled by s */
SIMD_FORCE_INLINE btVector3
operator/(const btVector3& v, const btScalar& s)
{
	btFullAssert(s != btScalar(0.0));
	return v * (btScalar(1.0) / s);
}

/**@brief Return the vector inversely scaled by s */
SIMD_FORCE_INLINE btVector3
operator/(const btVector3& v1, const btVector3& v2)
{
#ifdef BT_USE_SSE
    return btVector3(_mm_div_ps(v1.mVec128, v2.mVec128));
#else
    return btVector3(v1.m_floats[0] / v2.m_floats[0],v1.m_floats[1] / v2.m_floats[1],v1.m_floats[2] / v2.m_floats[2]);
#endif
}

/**@brief Return the dot product between two vectors */
SIMD_FORCE_INLINE btScalar 
btDot(const btVector3& v1, const btVector3& v2) 
{ 
	return v1.dot(v2); 
}


/**@brief Return the distance squared between two vectors */
SIMD_FORCE_INLINE btScalar
btDistance2(const btVector3& v1, const btVector3& v2) 
{ 
	return v1.distance2(v2); 
}


/**@brief Return the distance between two vectors */
SIMD_FORCE_INLINE btScalar
btDistance(const btVector3& v1, const btVector3& v2) 
{ 
	return v1.distance(v2); 
}

/**@brief Return the angle between two vectors */
SIMD_FORCE_INLINE btScalar
btAngle(const btVector3& v1, const btVector3& v2) 
{ 
	return v1.angle(v2); 
}

/**@brief Return the cross product of two vectors */
SIMD_FORCE_INLINE btVector3 
btCross(const btVector3& v1, const btVector3& v2) 
{ 
	return v1.cross(v2); 
}

SIMD_FORCE_INLINE btScalar
btTriple(const btVector3& v1, const btVector3& v2, const btVector3& v3)
{
	return v1.triple(v2, v3);
}

/**@brief Return the linear interpolation between two vectors
 * @param v1 One vector 
 * @param v2 The other vector 
 * @param t The ration of this to v (t = 0 => return v1, t=1 => return v2) */
SIMD_FORCE_INLINE btVector3 
lerp(const btVector3& v1, const btVector3& v2, const btScalar& t)
{
	return v1.lerp(v2, t);
}



SIMD_FORCE_INLINE btScalar btVector3::distance2(const btVector3& v) const
{
	return (v - *this).length2();
}

SIMD_FORCE_INLINE btScalar btVector3::distance(const btVector3& v) const
{
	return (v - *this).length();
}

SIMD_FORCE_INLINE btVector3 btVector3::normalized() const
{
	return *this / length();
} 

SIMD_FORCE_INLINE btVector3 btVector3::rotate( const btVector3& wAxis, const btScalar angle ) const
{
	// wAxis must be a unit lenght vector

	btVector3 o = wAxis * wAxis.dot( *this );
	btVector3 x = *this - o;
	btVector3 y;

	y = wAxis.cross( *this );

	return ( o + x * btCos( angle ) + y * btSin( angle ) );
}

class btVector4 : public btVector3
{
public:

	SIMD_FORCE_INLINE btVector4() {}


	SIMD_FORCE_INLINE btVector4(const btScalar& x, const btScalar& y, const btScalar& z,const btScalar& w) 
		: btVector3(x,y,z)
	{
		m_floats[3] = w;
	}


	SIMD_FORCE_INLINE btVector4 absolute4() const 
	{
		return btVector4(
			btFabs(m_floats[0]), 
			btFabs(m_floats[1]), 
			btFabs(m_floats[2]),
			btFabs(m_floats[3]));
	}



	btScalar	getW() const { return m_floats[3];}


		SIMD_FORCE_INLINE int maxAxis4() const
	{
		int maxIndex = -1;
		btScalar maxVal = btScalar(-BT_LARGE_FLOAT);
		if (m_floats[0] > maxVal)
		{
			maxIndex = 0;
			maxVal = m_floats[0];
		}
		if (m_floats[1] > maxVal)
		{
			maxIndex = 1;
			maxVal = m_floats[1];
		}
		if (m_floats[2] > maxVal)
		{
			maxIndex = 2;
			maxVal =m_floats[2];
		}
		if (m_floats[3] > maxVal)
		{
			maxIndex = 3;
//			maxVal = m_floats[3];
		}
		
		
		

		return maxIndex;

	}


	SIMD_FORCE_INLINE int minAxis4() const
	{
		int minIndex = -1;
		btScalar minVal = btScalar(BT_LARGE_FLOAT);
		if (m_floats[0] < minVal)
		{
			minIndex = 0;
			minVal = m_floats[0];
		}
		if (m_floats[1] < minVal)
		{
			minIndex = 1;
			minVal = m_floats[1];
		}
		if (m_floats[2] < minVal)
		{
			minIndex = 2;
			minVal =m_floats[2];
		}
		if (m_floats[3] < minVal)
		{
			minIndex = 3;
//			minVal = m_floats[3];
		}
		
		return minIndex;

	}


	SIMD_FORCE_INLINE int closestAxis4() const 
	{
		return absolute4().maxAxis4();
	}

	
 

  /**@brief Set x,y,z and zero w 
   * @param x Value of x
   * @param y Value of y
   * @param z Value of z
   */
		

/*		void getValue(btScalar *m) const 
		{
			m[0] = m_floats[0];
			m[1] = m_floats[1];
			m[2] =m_floats[2];
		}
*/
/**@brief Set the values 
   * @param x Value of x
   * @param y Value of y
   * @param z Value of z
   * @param w Value of w
   */
		SIMD_FORCE_INLINE void	setValue(const btScalar& x, const btScalar& y, const btScalar& z,const btScalar& w)
		{
			m_floats[0]=x;
			m_floats[1]=y;
			m_floats[2]=z;
			m_floats[3]=w;
		}


};


///btSwapVector3Endian swaps vector endianness, useful for network and cross-platform serialization
SIMD_FORCE_INLINE void	btSwapScalarEndian(const btScalar& sourceVal, btScalar& destVal)
{
	#ifdef BT_USE_DOUBLE_PRECISION
	unsigned char* dest = (unsigned char*) &destVal;
	unsigned char* src  = (unsigned char*) &sourceVal;
	dest[0] = src[7];
    dest[1] = src[6];
    dest[2] = src[5];
    dest[3] = src[4];
    dest[4] = src[3];
    dest[5] = src[2];
    dest[6] = src[1];
    dest[7] = src[0];
#else
	unsigned char* dest = (unsigned char*) &destVal;
	unsigned char* src  = (unsigned char*) &sourceVal;
	dest[0] = src[3];
    dest[1] = src[2];
    dest[2] = src[1];
    dest[3] = src[0];
#endif //BT_USE_DOUBLE_PRECISION
}

///btSwapVector3Endian swaps vector endianness, useful for network and cross-platform serialization
SIMD_FORCE_INLINE void	btSwapVector3Endian(const btVector3& sourceVec, btVector3& destVec)
{
	for (int i=0;i<4;i++)
	{
		btSwapScalarEndian(sourceVec[i],destVec[i]);
	}

}

///btUnSwapVector3Endian swaps vector endianness, useful for network and cross-platform serialization
SIMD_FORCE_INLINE void	btUnSwapVector3Endian(btVector3& vector)
{

	btVector3	swappedVec;
	for (int i=0;i<4;i++)
	{
		btSwapScalarEndian(vector[i],swappedVec[i]);
	}
	vector = swappedVec;
}

template <class T>
SIMD_FORCE_INLINE void btPlaneSpace1 (const T& n, T& p, T& q)
{
  if (btFabs(n[2]) > SIMDSQRT12) {
    // choose p in y-z plane
    btScalar a = n[1]*n[1] + n[2]*n[2];
    btScalar k = btRecipSqrt (a);
    p[0] = 0;
	p[1] = -n[2]*k;
	p[2] = n[1]*k;
    // set q = n x p
    q[0] = a*k;
	q[1] = -n[0]*p[2];
	q[2] = n[0]*p[1];
  }
  else {
    // choose p in x-y plane
    btScalar a = n[0]*n[0] + n[1]*n[1];
    btScalar k = btRecipSqrt (a);
    p[0] = -n[1]*k;
	p[1] = n[0]*k;
	p[2] = 0;
    // set q = n x p
    q[0] = -n[2]*p[1];
	q[1] = n[2]*p[0];
	q[2] = a*k;
  }
}


struct	btVector3FloatData
{
	float	m_floats[4];
};

struct	btVector3DoubleData
{
	double	m_floats[4];

};

SIMD_FORCE_INLINE	void	btVector3::serializeFloat(struct	btVector3FloatData& dataOut) const
{
	///could also do a memcpy, check if it is worth it
	for (int i=0;i<4;i++)
		dataOut.m_floats[i] = float(m_floats[i]);
}

SIMD_FORCE_INLINE void	btVector3::deSerializeFloat(const struct	btVector3FloatData& dataIn)
{
	for (int i=0;i<4;i++)
		m_floats[i] = btScalar(dataIn.m_floats[i]);
}


SIMD_FORCE_INLINE	void	btVector3::serializeDouble(struct	btVector3DoubleData& dataOut) const
{
	///could also do a memcpy, check if it is worth it
	for (int i=0;i<4;i++)
		dataOut.m_floats[i] = double(m_floats[i]);
}

SIMD_FORCE_INLINE void	btVector3::deSerializeDouble(const struct	btVector3DoubleData& dataIn)
{
	for (int i=0;i<4;i++)
		m_floats[i] = btScalar(dataIn.m_floats[i]);
}


SIMD_FORCE_INLINE	void	btVector3::serialize(struct	btVector3Data& dataOut) const
{
	///could also do a memcpy, check if it is worth it
	for (int i=0;i<4;i++)
		dataOut.m_floats[i] = m_floats[i];
}

SIMD_FORCE_INLINE void	btVector3::deSerialize(const struct	btVector3Data& dataIn)
{
	for (int i=0;i<4;i++)
		m_floats[i] = dataIn.m_floats[i];
}



#endif //BT_VECTOR3_H
