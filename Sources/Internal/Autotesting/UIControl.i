%module UIControl
%{
#include "UI/UIControl.h"
%}

%import Vector.i
%import Rect.i

namespace DAVA
{
	
class UIGeometricData
{
	
public:
	UIGeometricData()
	{
		cosA = 1.0f;
		sinA = 0;
		oldAngle = 0;
		angle = 0;
	}
	Vector2 position;
	Vector2 size;
	
	Vector2 pivotPoint;
	Vector2	scale;
	float32	angle;
	
	float32 cosA;
	float32 sinA;

	const Rect &GetUnrotatedRect() const
	{
		return unrotatedRect;
	}
};

class UIControl
{
public:

	UIControl(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	
	inline const Rect GetRect() const;
    
	inline const Vector2 &GetPosition() const;
	
	virtual const Vector2 &GetSize() const;

	virtual const UIGeometricData &GetGeometricData();

	virtual bool GetVisible() const;

	virtual bool GetInputEnabled() const;

	virtual void SetInputEnabled(bool isEnabled, bool hierarchic = true);
	
	virtual bool GetDisabled() const;

	virtual bool GetSelected() const;

	const String & GetName() const;
	
	bool IsOnScreen() const;

	int32 GetTag() const;
    
	UIControl *GetParent();

protected:
	virtual ~UIControl();

};
};
