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


#ifndef __DAVAENGINE_RECT_H__
#define __DAVAENGINE_RECT_H__

#include <math.h>

#include "Vector.h"

namespace DAVA
{
/**	
	\ingroup math
	\brief Rect in 2D space. This class is used for all rectangles along all SDK subsystems.
 */
struct Rect
{
	float32 x;
	float32 y;
	float32 dx;
	float32 dy;

	inline Rect();
	inline Rect(float32 _x, float32 _y, float32 _dx, float32 _dy);
	inline Rect(const Rect & rect);
	inline Rect(const Vector2 & point, const Vector2 & size);

	inline bool PointInside(const Vector2 & point) const; 
	inline Rect Intersection(const Rect & rect) const;
	inline bool RectIntersects(const Rect & rect) const;
    inline bool RectContains(const Rect& rect) const;
    inline void ClampToRect(Vector2& point) const;
    inline void ClampToRect(Rect& rect) const;
    inline Rect Combine(const Rect& rect) const;

    inline Vector2 GetCenter() const;
    inline Vector2 GetPosition() const;
    inline Vector2 GetSize() const;

    inline void SetCenter(const Vector2& center);
    inline void SetPosition(const Vector2& position);
    inline void SetSize(const Vector2 & size);

	inline bool operator == (const Rect  & _s)const;
	inline bool operator != (const Rect  & _s)const;

	inline Rect &	operator += (const Vector2 & point);
	inline Rect &	operator -= (const Vector2 & point);

	inline Rect 	operator + (const Vector2 & Point) const;
	inline Rect 	operator - (const Vector2 & Point) const;
};

inline Rect::Rect()
{
	x = y = dx = dy = 0;
};

inline Rect::Rect(float32 _x, float32 _y, float32 _dx, float32 _dy)
{
	x = _x;
	y = _y;
	dx = _dx;
	dy = _dy;
};

inline Rect::Rect(const Rect & rect)
{
	x = rect.x;
	y = rect.y;
	dx = rect.dx;
	dy = rect.dy;
};

inline Rect::Rect( const Vector2 & point, const Vector2 & size )
{
    x = point.x;
    y = point.y;
    dx = size.x;
    dy = size.y;
}

inline bool Rect::PointInside(const Vector2 & point) const
{	
    if ((point.x >= x) && (point.x < x + dx) 
		&& (point.y >= y) && (point.y < y + dy))
			return true;
	return false;
}

inline bool Rect::RectIntersects(const Rect & rect) const
{
	float32 top1 = y;
	float32 top2 = rect.y;
	float32 bottom1 = y + dy;
	float32 bottom2 = rect.y + rect.dy;
    if (top1 > bottom1 || top2 > bottom2 || bottom1 < top2 || top1 > bottom2)
    {
        return false;
    }
    float32 left1 = x;
    float32 left2 = rect.x;
    float32 right1 = x + dx;
    float32 right2 = rect.x + rect.dx;
    if (left1 > right1 || left2 > right2 || right1 < left2 || left1 > right2)
    {
        return false;
    }

    return true;
}

//realization from Qt QRect.cpp: bool QRectF::contains(const QRectF &r) const
inline bool Rect::RectContains(const Rect& rect) const
{
    float32 top1 = y;
    float32 bottom1 = y + dy;
    float32 top2 = rect.y;
    float32 bottom2 = rect.y + rect.dy;

    if (top1 > bottom1 || top2 > bottom2 || top2 < top1 || bottom2 > bottom1)
    {
        return false;
    }

    float32 left1 = x;
    float32 right1 = x + dx;
    float32 left2 = rect.x;
    float32 right2 = rect.x + rect.dx;

    if (left1 > right1 || left2 > right2 || left2 < left1 || right2 > right1)
    {
        return false;
    }

    return true;
}

inline Rect Rect::Intersection(const Rect & rect) const
{
	float32 nx = Max(x, rect.x);
	float32 ny = Max(y, rect.y);
	float32 ndx = Min((dx + x) - nx, (rect.dx + rect.x) - nx);
	float32 ndy = Min((dy + y) - ny, (rect.dy + rect.y) - ny);
	if (ndx <= 0 || ndy <= 0)
	{
		ndx = 0;
		ndy = 0;
	}
	
	return Rect(nx, ny, ndx, ndy);
}
	
inline void Rect::ClampToRect(Vector2 & point) const
{
	if (point.x < x)point.x = x;
	if (point.y < y)point.y = y;
	if (point.x > x + dx)point.x = x + dx;
	if (point.y > y + dy)point.y = y + dy;
}

inline void Rect::ClampToRect(Rect& rect) const
{
	Vector2 topLeft(rect.x, rect.y);
	Vector2 bottomRight = topLeft + Vector2(rect.dx, rect.dy);

	ClampToRect(topLeft);
	ClampToRect(bottomRight);

	rect = Rect(topLeft, bottomRight - topLeft);
}
	
inline Rect Rect::Combine(const Rect& rect) const
{
	Vector2 topLeft(x, y);
	Vector2 bottomRight(x + dx, y + dy);
	topLeft.x = Min(topLeft.x, rect.x);
	topLeft.y = Min(topLeft.y, rect.y);
	bottomRight.x = Max(bottomRight.x, rect.x + rect.dx);
	bottomRight.y = Max(bottomRight.y, rect.y + rect.dy);
	return Rect(topLeft, bottomRight - topLeft);
}

inline Vector2 Rect::GetCenter() const
{
	return Vector2(x + (dx * .5f), y + (dy * .5f));
}

inline Vector2 Rect::GetPosition() const
{
	return Vector2(x, y);
}

inline Vector2 Rect::GetSize() const
{
	return Vector2(dx, dy);
}

inline void Rect::SetCenter(const Vector2 & center)
{
	x = center.x - dx * 0.5f;
	y = center.y - dy * 0.5f;
}

inline void Rect::SetPosition(const Vector2 & position)
{
	x = position.x;
	y = position.y;
}

inline void Rect::SetSize(const Vector2 & size)
{
	dx = size.dx;
	dy = size.dy;
}

inline bool Rect::operator == (const Rect & _r)const
{
	return (x == _r.x && y == _r.y && dx == _r.dx && dy == _r.dy);
}

inline bool Rect::operator != (const Rect & _r)const
{
	return (!Rect::operator==(_r));
}

inline Rect & Rect::operator += (const Vector2 & pt)
{
	x += pt.x;
	y += pt.y;
	return *this;
}

inline Rect & Rect::operator -= (const Vector2 & pt)
{
	x -= pt.x;
	y -= pt.y;
	return *this;
}

inline Rect	Rect::operator + (const Vector2 & pt) const
{
	return Rect( x + pt.x, y + pt.y, dx, dy);
}

inline Rect Rect::operator - (const Vector2 & pt) const
{
	return Rect( x - pt.x, y - pt.y, dx, dy);
}

}; // end of namespace DAVA
#endif // __DAVAENGINE_MATH2DBASE_H__
