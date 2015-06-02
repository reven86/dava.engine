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



#include "UI/ScrollHelper.h"
#include "FileSystem/Logger.h"
#include "Math/Math2D.h"
#include <math.h>


namespace DAVA 
{
    const float32 ScrollHelper::maxDeltaTime = 0.1f;

	ScrollHelper::ScrollHelper()
	:	BaseObject()
	,	position(0.f)
	,	elementSize(0.f)
    ,   viewSize(0.f)
    ,   virtualViewSize(0.f)
	,	totalDeltaTime(0.f)
	,	totalDeltaMove(0.f)
	,   speed(0.f)
    ,   scrollToPos(0.f)
    ,   scrollToAcc(0.f)
    ,   scrollToTopSpeed(0.f)
	{
		slowDown = 0.25f;
		backward = 0.3f;
	}

    void ScrollHelper::CopyDataFrom(const ScrollHelper *src)
    {
        position = src->position;
        elementSize = src->elementSize;
        viewSize = src->viewSize;
        virtualViewSize = src->virtualViewSize;
        
        slowDown = src->slowDown;
        backward = src->backward;
        
        speed = src->speed;
        
        scrollToPos = src->scrollToPos;
        scrollToAcc = src->scrollToAcc;
        scrollToTopSpeed = src->scrollToTopSpeed;
    }

	void ScrollHelper::SetPosition(float32 pos)
	{
		position = pos;
        position = Min(position, 0.f);
        position = Max(position, -elementSize);
        scrollToTopSpeed = 0.f;
	}
	void ScrollHelper::SetElementSize(float32 newSize)
	{
		elementSize = newSize;
		virtualViewSize = viewSize;
		if(viewSize > elementSize)
		{
			virtualViewSize = elementSize;
		}
	}
	
	float ScrollHelper::GetPosition() const
	{
		return position;
	}
	
	void ScrollHelper::SetViewSize(float32 size)
	{
		viewSize = size;
		virtualViewSize = viewSize;
		if(viewSize > elementSize)
		{
			virtualViewSize = elementSize;
		}
	}
    
    float32 ScrollHelper::GetViewSize() const
    {
        return viewSize;
    }

    float32 ScrollHelper::GetElementSize() const
    {
        return elementSize;
    }

	float32 ScrollHelper::GetCurrentSpeed() const
	{
		return speed;
	}

	void ScrollHelper::SetSlowDownTime(float newValue)
	{
		slowDown = newValue;
	}
	
	void ScrollHelper::SetBorderMoveModifer(float newValue)
	{
		backward = newValue;
	}

	void ScrollHelper::Impulse(float impulseSpeed)
	{
		if((position > 0.f && impulseSpeed > 0.f) || (position + elementSize < virtualViewSize && impulseSpeed < 0.f))
		{
			return;
		}
		speed = impulseSpeed;
	}

	
	float ScrollHelper::GetPosition(float positionDelta, float timeDelta, bool isPositionLocked)
	{
        if (virtualViewSize == 0.0f)
        {
            return 0.0f;
        }
        
		if(isPositionLocked)
		{
			if(position + positionDelta > 0)
			{
				positionDelta *= (1.0f - position / virtualViewSize) * backward;
			}
			if(position + elementSize + positionDelta  < virtualViewSize)
			{
                
				positionDelta *= (1.0f - (virtualViewSize - (position + elementSize)) / virtualViewSize) * backward;
			}
			position += positionDelta;
			speed = 0.f;
            scrollToTopSpeed = 0.f;
			MovesDelta m;
			m.deltaMove = positionDelta;
			m.deltaTime = timeDelta;
			moves.push_back(m);
			totalDeltaTime += timeDelta;
			totalDeltaMove += positionDelta;
			if(totalDeltaTime >= maxDeltaTime)
			{
				List<MovesDelta>::iterator it = moves.begin();
				totalDeltaTime -= it->deltaTime;
				totalDeltaMove -= it->deltaMove;
				moves.erase(it);
			}
		}
		else
		{
			if(totalDeltaMove != 0.f)
			{
				speed = totalDeltaMove / totalDeltaTime;
				speed = Min(speed, virtualViewSize * 2.f);
				speed = Max(speed, -virtualViewSize * 2.f);
			}
			if (scrollToTopSpeed != 0.f)
            {
                if (scrollToTopSpeed < 0.f)
                {
                    if (speed <= scrollToTopSpeed)
                    {
                        float32 dist = (scrollToPos - position);
                        scrollToAcc = (speed * speed) / (2.f * dist);
                    }
                }
                else
                {
                    if (speed >= scrollToTopSpeed)
                    {
                        float32 dist = (scrollToPos - position);
                        scrollToAcc = (speed * speed) / (2.f * dist);
                    }
                }
                speed = speed + scrollToAcc * timeDelta;
                float32 oldPos = position;
                position += speed * timeDelta;
                if ((oldPos <= scrollToPos && position >= scrollToPos)
                    || (oldPos >= scrollToPos && position <= scrollToPos))
                {
                    position = scrollToPos;
                    scrollToTopSpeed = 0.f;
                    speed = 0.f;
                }
            }
			else if(position > 0.f)
			{
				if(backward != 0.f && slowDown != 0.f)
				{
					if(slowDown != 0.f)
					{
						speed -= virtualViewSize * timeDelta / slowDown / backward;
					}
					else
					{
						speed -= virtualViewSize * timeDelta * 4 / backward;
					}
					position += speed * timeDelta;
					if(position < 0.f)
					{
						position = 0.f;
						speed = 0.f;
					}
				}
				else
				{
					position = 0.f;
					speed = 0.f;
				}
			}
			else if(position + elementSize < virtualViewSize)
			{
				if(backward != 0.f)
				{
					if(slowDown != 0.f)
					{
						speed += virtualViewSize * timeDelta / slowDown / backward;
					}
					else
					{
						speed += virtualViewSize * timeDelta * 4.f / backward;
					}
					position += speed * timeDelta;
					if(position + elementSize > virtualViewSize)
					{
						position = virtualViewSize - elementSize;
						speed = 0.f;
					}
				}
				else
				{
					position = virtualViewSize - elementSize;
					speed = 0.f;
				}
			}
			else if(speed != 0.f)
			{
				if(slowDown != 0.f)
				{
					float oldSpd = speed;
					speed = speed - speed / slowDown * timeDelta;
					if((oldSpd > 0.f && speed < 1.0f) || (oldSpd < 0.f && speed > -1.0f))
					{
						speed = 0.f;
					}
					position += speed * timeDelta;
				}
				else
				{
					speed = 0.f;
				}
			}
			
			moves.clear();
			totalDeltaTime = 0.f;
			totalDeltaMove = 0.f;
		}
		
		return position;
	}
    
    void ScrollHelper::ScrollToPosition(float32 newPos, float32 scrollTimeSec/* = 0.3f*/)
    {
        if (FLOAT_EQUAL_EPS(newPos, position, 1.f))
        {
            return;
        }
        scrollToPos = newPos;
        float32 halfTime = scrollTimeSec * 0.5f;
        float32 dist = (newPos - position) * 0.5f;
        speed = 0.f;
        totalDeltaMove = 0.f;
        scrollToAcc = (dist * 2.f) / (halfTime * halfTime);
        scrollToTopSpeed = sqrtf(2.f * scrollToAcc * dist);
        if (scrollToAcc < 0.f)
        {
            scrollToTopSpeed = -scrollToTopSpeed;
        }
    }
}





