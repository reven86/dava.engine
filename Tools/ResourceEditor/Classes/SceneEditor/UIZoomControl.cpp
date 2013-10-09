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




#include "UIZoomControl.h"

REGISTER_CLASS(UIZoomControl);

static const float32 SCROLL_BEGIN_PIXELS = 8.0f;

UIZoomControl::UIZoomControl(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates),
	contentSize(),
	state(STATE_NONE),
	positionIndex(0),
	zoomScale(1.0f),
	scrollOrigin(0, 0),
	scrollZero(0, 0),
	lastTapTime(0),
	touchStartTime(0)
{
	inputEnabled = true;
	multiInput = true;
	SetClipContents(true);
	
	prevZoomScale = minScale = maxScale = zoomScale;
    
    minScale = 0.5f;
    maxScale = 1.5f;
    
    contentControl = new UIControl(rect, rectInAbsoluteCoordinates);
    contentControl->SetInputEnabled(false);
    UIControl::AddControl(contentControl);

    SetScale(0.5);
}

UIZoomControl::~UIZoomControl()
{
}

void UIZoomControl::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    UIControl::LoadFromYamlNode(node, loader);
    
    //TODO: load zoom control specific params (content size, input style?)
    const YamlNode * contentSizeNode = node->Get("contentSize");
    
    if(contentSizeNode)
    {
        SetContentSize(contentSizeNode->AsVector2());
    }
    else
    {
        Rect rc = GetRect();
        SetContentSize(Vector2(rc.dx, rc.dy));
    }
    
    const YamlNode * minScaleNode  = node->Get("minScale");
    minScale = minScaleNode->AsFloat();
    
    const YamlNode * maxScaleNode  = node->Get("maxScale");
    maxScale = maxScaleNode->AsFloat();
    
    const YamlNode * zoomScaleNode  = node->Get("zoomScale");
    SetScale(zoomScaleNode->AsFloat());
}

void UIZoomControl::SetContentSize(const Vector2 &_contentSize)
{
    contentSize = _contentSize;
    contentControl->SetSize(contentSize);
}

const Vector2 &UIZoomControl::GetContentSize()
{
    return contentControl->GetSize();
}

void UIZoomControl::SetScales(float32 _minScale, float32 _maxScale)
{
	minScale = _minScale;
	maxScale = _maxScale;
}

void UIZoomControl::SetOffset(const Vector2 &offset)
{
	scrollOrigin = offset;
	contentControl->SetPosition(scrollOrigin);
//	
//	LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &scrollOrigin, _position, time, interpolationFunc);
//	animation->Start(track);
}

Vector2 UIZoomControl::GetOffset()
{
    if(STATE_SCROLL == state)
    {
        return scrollOrigin + scrollCurrentShift;
    }
    return scrollOrigin;
}


Animation *	UIZoomControl::ScrollOffsetAnimation(const Vector2 & _position, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
{
	LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &scrollOrigin, _position, time, interpolationFunc);
	animation->Start(track);
	
    return animation;
}

void UIZoomControl::SetScale(float currentScale)
{ 
	zoomScale = currentScale;
    
    contentControl->scale = Vector2(zoomScale, zoomScale);
	
	scrollZero = Vector2(0.f, 0.f);
	
	float32 adjW = contentSize.dx*zoomScale;
	float32 adjH = contentSize.dy*zoomScale;
	
	if(adjW < size.x)
	{
		scrollZero.x = (size.x - adjW)/2.f;
	}
	
	if(adjH < size.y)
	{
		scrollZero.y = (size.y - adjH)/2.f;
	}
}

float32 UIZoomControl::GetScale()
{
    return zoomScale;
}

	
void UIZoomControl::PerformScroll()
{
	Vector2 clickEndPosition = clickStartPosition;
	clickEndPosition.x -= (scrollOrigin.x);
	clickEndPosition.y -= (scrollOrigin.y);
	
	clickEndPosition.x /= zoomScale;
	clickEndPosition.y /= zoomScale;
	
	UIEvent modifiedTouch = scrollTouch;
	modifiedTouch.phase = UIEvent::PHASE_ENDED;
	modifiedTouch.point = clickEndPosition;
	
	ScrollTouch(&modifiedTouch);
}

void UIZoomControl::Update(float32 timeElapsed)
{
	if(state == STATE_DECCELERATION)
	{
		scrollOrigin.x += deccelerationSpeed.x * scrollPixelsPerSecond * timeElapsed;
		scrollOrigin.y += deccelerationSpeed.y * scrollPixelsPerSecond * timeElapsed;
		
        contentControl->SetPosition(scrollOrigin);
        
		scrollPixelsPerSecond *= 0.7f;
		
		if (scrollPixelsPerSecond <= 0.1f)
		{
			scrollPixelsPerSecond = 0.f;
			state = STATE_NONE;
		}
	}
	
    //scrolls back to 0.0
	if(state != STATE_ZOOM && state != STATE_SCROLL) 
	{
		//hcenter
		if((scrollOrigin.x > 0) && ((scrollOrigin.x + contentSize.dx * zoomScale) < (size.x)))
		{
			float32 delta = scrollOrigin.x-scrollZero.x;
			scrollOrigin.x -= delta * timeElapsed * 5;
		}
		else if((scrollOrigin.x > 0) && ((scrollOrigin.x + contentSize.dx * zoomScale) > (size.x)))
		{
			scrollOrigin.x -=  (scrollOrigin.x) * timeElapsed * 5;
		}
		else if((scrollOrigin.x < 0) && ((scrollOrigin.x + contentSize.dx * zoomScale) < (size.x)))
		{
			scrollOrigin.x += (size.x - (scrollOrigin.x + contentSize.dx * zoomScale)) * timeElapsed * 5;
		}
		
		//vcenter
		if((scrollOrigin.y > 0) && ((scrollOrigin.y + contentSize.dy * zoomScale) < (size.y)))
		{
			float32 delta = scrollOrigin.y-scrollZero.y;
			scrollOrigin.y -= delta * timeElapsed * 5;            
		}
		else if((scrollOrigin.y > 0) && ((scrollOrigin.y + contentSize.dy * zoomScale) >= (size.y)))
		{
			scrollOrigin.y -= (scrollOrigin.y) * timeElapsed * 5;
		}
		else if((scrollOrigin.y < 0) && ((scrollOrigin.y + contentSize.dy * zoomScale) < (size.y)))
		{
			scrollOrigin.y += ((size.y) - (scrollOrigin.y + contentSize.dy * zoomScale)) * timeElapsed * 5;
		}
        
        contentControl->SetPosition(scrollOrigin);
	} 
	
	float32 invScale = 1.0f / zoomScale;
	scrollPosition.x = (scrollOrigin.x + scrollCurrentShift.x) * invScale;
	scrollPosition.y = (scrollOrigin.y + scrollCurrentShift.y) * invScale;
	
	//scrolling over the edge
	drawScrollPos.x = scrollCurrentShift.x;
	drawScrollPos.y = scrollCurrentShift.y;
}

void UIZoomControl::StartScroll(Vector2 _startScrollPosition)
{
	scrollStartInitialPosition = _startScrollPosition;
	scrollStartPosition = _startScrollPosition;
	scrollCurrentPosition = _startScrollPosition;
	scrollStartMovement = false;
	
	lastMousePositions[positionIndex & (MAX_MOUSE_POSITIONS - 1)] = _startScrollPosition;
	positionIndex++;
	positionIndex &= (MAX_MOUSE_POSITIONS - 1);
}

void UIZoomControl::ProcessScroll(Vector2 _currentScrollPosition)
{
	scrollCurrentPosition = _currentScrollPosition;

	if (LineLength(Point2f(scrollCurrentPosition.x, scrollCurrentPosition.y), Point2f(scrollStartInitialPosition.x, scrollStartInitialPosition.y)) >= SCROLL_BEGIN_PIXELS)
	{
		touchStartTime = 0;
		if (!scrollStartMovement)scrollStartPosition = scrollCurrentPosition;
		scrollCurrentShift = Vector2((scrollCurrentPosition.x - scrollStartPosition.x),  (scrollCurrentPosition.y - scrollStartPosition.y));
		scrollStartMovement = true;
	}

	lastMousePositions[positionIndex & (MAX_MOUSE_POSITIONS - 1)] = _currentScrollPosition;
	positionIndex++;
	positionIndex &= (MAX_MOUSE_POSITIONS - 1);
}

void UIZoomControl::EndScroll()
{
	scrollOrigin.x += scrollCurrentShift.x;
	scrollOrigin.y += scrollCurrentShift.y;
    
    contentControl->SetPosition(scrollOrigin);
    
	scrollCurrentShift.x = 0;
	scrollCurrentShift.y = 0;
}

void UIZoomControl::Input(UIEvent * currentTouch)
{
//  int32 saveState = state;
	Vector<UIEvent> touches = UIControlSystem::Instance()->GetAllInputs();
#if defined(__DAVAENGINE_WIN32__) || defined (__DAVAENGINE_MACOS__)
//    if(InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT))
//    {
//        if(1 == touches.size())
//        {
//            switch(currentTouch->phase) 
//            {
//                case UIEvent::PHASE_BEGAN:
//                {
//                    if (state == STATE_SCROLL)
//                    {
//                        EndScroll();
//                    }
//                    
//                    // init zoom parameters
//                    state = STATE_ZOOM;
//                    
//                    zoomTouches[0] = touches[0];
//                    zoomTouches[1] = touches[0];
//                    
//                    zoomStartPositions[0] = zoomTouches[0].point;
//                    zoomStartPositions[1] = zoomTouches[1].point;
//                    
//                    prevZoomScale = zoomScale; // save current scale to perform scaling in zoom mode
//                }
//                    break;
//                case UIEvent::PHASE_DRAG:
//                    if(state == STATE_ZOOM)
//                    {
//                        zoomTouches[1] = touches[0];
//                        zoomStartPositions[1] = zoomTouches[1].point;
//                        
//                        float32 distance = (zoomStartPositions[1].y - zoomStartPositions[0].y);
//                        
//                        float32 saveZoomScale = zoomScale;
//                        float32 changeCoeff = distance / GetRect().dx;
//                        float32 newScale = zoomScale + changeCoeff;
//                        
//                        if(newScale > maxScale)newScale = maxScale;
//                        if(newScale < minScale)newScale = minScale; 
//                        
//                        SetScale(newScale);
//                        
//                        Vector2 center = Vector2((zoomStartPositions[0].x + zoomStartPositions[1].x) / 2,
//                                                 (zoomStartPositions[0].y + zoomStartPositions[1].y) / 2);
//                        
//                        Vector2 scaleVectorOriginal = Vector2(center.x - scrollOrigin.x, center.y - scrollOrigin.y);
//                        Vector2 scaleVectorNew = scaleVectorOriginal;
//                        scaleVectorNew.x *= zoomScale / saveZoomScale;
//                        scaleVectorNew.y *= zoomScale / saveZoomScale;
//                        
//                        scaleVectorNew.x -= scaleVectorOriginal.x;
//                        scaleVectorNew.y -= scaleVectorOriginal.y;
//                        
//                        scrollOrigin.x -= scaleVectorNew.x;
//                        scrollOrigin.y -= scaleVectorNew.y;
//                        
//                        zoomTouches[0] = zoomTouches[1];
//                        zoomStartPositions[0] = zoomStartPositions[1];
//                    }
//                    break;
//                case UIEvent::PHASE_ENDED:
//                {
//                    //                    if(state == STATE_ZOOM)
//                    //                    {
//                    //                        zoomTouches[1] = touches[0];
//                    //                        zoomCurrentPositions[1] = zoomTouches[1].point;
//                    //                        
//                    //                        float32 distance = (zoomStartPositions[1].y - zoomStartPositions[0].y);
//                    //                        
//                    //                        float32 saveZoomScale = zoomScale;
//                    //                        float32 changeCoeff = distance / GetRect().dx;
//                    //                        float32 newScale = prevZoomScale + changeCoeff;
//                    //                        
//                    //                        if(newScale > maxScale)newScale = maxScale;
//                    //                        if(newScale < minScale)newScale = minScale; 
//                    //                        
//                    //                        SetScale(newScale);
//                    //                        
//                    //                        Vector2 center = Vector2((zoomStartPositions[0].x + zoomStartPositions[1].x) / 2,
//                    //                                                 (zoomStartPositions[0].y + zoomStartPositions[1].y) / 2);
//                    //                        
//                    //                        Vector2 scaleVectorOriginal = Vector2(center.x - scrollOrigin.x, center.y - scrollOrigin.y);
//                    //                        Vector2 scaleVectorNew = scaleVectorOriginal;
//                    //                        scaleVectorNew.x *= zoomScale / saveZoomScale;
//                    //                        scaleVectorNew.y *= zoomScale / saveZoomScale;
//                    //                        
//                    //                        scaleVectorNew.x -= scaleVectorOriginal.x;
//                    //                        scaleVectorNew.y -= scaleVectorOriginal.y;
//                    //                        
//                    //                        scrollOrigin.x -= scaleVectorNew.x;
//                    //                        scrollOrigin.y -= scaleVectorNew.y;
//                    //                        
//                    //                        //                        zoomTouches[0] = zoomTouches[1];
//                    //                        //                        zoomCurrentPositions[0] = zoomCurrentPositions[1];
//                    //                    }
//
//                    
//                    
//                    Vector<UIEvent>::iterator it = touches.begin();
//                    bool zoomToScroll = false;
//                    for(; it != touches.end(); ++it)
//                    {
//                        if((*it).phase == UIEvent::PHASE_DRAG)
//                        {
//                            zoomToScroll = true;
//                            scrollTouch = *it;
//                        }
//                    }
//                    
//                    if(zoomToScroll)
//                    {
//                        Vector2 start = scrollTouch.point;
//                        StartScroll(start);
//                        state = STATE_SCROLL;
//                    }
//                    else
//                    {
//                        if (state != STATE_DECCELERATION) 
//                        {
//                            state = STATE_NONE;
//                        }
//                    }
//                }
//                    break;
//            }
//
//        }
//    }
//    else
#endif //#if defined(__DAVAENGINE_WIN32__) || defined (__DAVAENGINE_MACOS__)
	if(1 == touches.size())
	{
		switch(currentTouch->phase) 
		{
			case UIEvent::PHASE_BEGAN:
			{
				scrollTouch = *currentTouch;
				
				Vector2 start = currentTouch->point;
				StartScroll(start);

				// init scrolling speed parameters
				scrollPixelsPerSecond = 0.0f;
				scrollStartTime = (float64)SystemTimer::Instance()->FrameStampTimeMS();//to avoid cast from uint64 to float64 SystemTimer::Instance()->AbsoluteMS();

				touchStartTime = SystemTimer::Instance()->FrameStampTimeMS();
				state = STATE_SCROLL;

				clickStartPosition = start;
			}
				break;
			case UIEvent::PHASE_DRAG:
			{
				if(state == STATE_SCROLL) 
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						// scrolling speed get parameters
						float64 scrollCurrentTime = (float64)SystemTimer::Instance()->FrameStampTimeMS();//SystemTimer::Instance()->AbsoluteMS();
						Vector2 scrollPrevPosition = scrollCurrentPosition;
						
						// perform scrolling
						ProcessScroll(currentTouch->point);
						
						// calculate scrolling speed
						float64 tmp = scrollCurrentTime;
						if(fabs(scrollCurrentTime - scrollStartTime) < 1e-9)
						{
							tmp += .1f;
						}

                        float64 length = LineLength(Point2f(scrollCurrentPosition.x, scrollCurrentPosition.y), Point2f(scrollPrevPosition.x, scrollPrevPosition.y));

						scrollPixelsPerSecond = (float32)(length / (tmp - scrollStartTime));
						scrollStartTime = scrollCurrentTime;
					}
				}
			}
				break;
			case UIEvent::PHASE_ENDED:
			{
				if(state == STATE_SCROLL)
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						Vector2 scrollPrevPos = lastMousePositions[(positionIndex - 3) & (MAX_MOUSE_POSITIONS - 1)];
						Vector2 currentTouchPos = currentTouch->point; 
						
						deccelerationSpeed = Vector2(currentTouchPos.x, currentTouchPos.y);
						deccelerationSpeed.x -= scrollPrevPos.x;
						deccelerationSpeed.y -= scrollPrevPos.y;
						
						float32 deccelerationSpeedLen = sqrtf(deccelerationSpeed.x * deccelerationSpeed.x + deccelerationSpeed.y * deccelerationSpeed.y);
						if (deccelerationSpeedLen >= 0.00001f)
						{
							deccelerationSpeed.x /= deccelerationSpeedLen;
							deccelerationSpeed.y /= deccelerationSpeedLen;
						}
						else
						{
							deccelerationSpeed.x = 0.0f;
							deccelerationSpeed.y = 0.0f;
						}
						
						EndScroll();

						if(scrollStartMovement)
						{
							state = STATE_DECCELERATION;
						}
						
						clickEndPosition = currentTouchPos;
					}
					
					if(touchStartTime)
					{
						touchStartTime = 0;
						PerformScroll();
					}
				}
			}
				break;
		}
	}
	else if(2 == touches.size())
	{
		switch(currentTouch->phase) 
		{
			case UIEvent::PHASE_BEGAN:
			{
				if (state == STATE_SCROLL)
				{
					EndScroll();
				}
				
				// init zoom parameters
				state = STATE_ZOOM;
				
				zoomTouches[0] = touches[0];
				zoomTouches[1] = touches[1];
				
				zoomStartPositions[0] = zoomTouches[0].point;
				zoomStartPositions[1] = zoomTouches[1].point;
				
				prevZoomScale = zoomScale; // save current scale to perform scaling in zoom mode
			}
			break;
			case UIEvent::PHASE_DRAG:
				if(state == STATE_ZOOM)
				{
					zoomTouches[0] = touches[0];
					zoomTouches[1] = touches[1];
					
					zoomCurrentPositions[0] = zoomTouches[0].point;
					zoomCurrentPositions[1] = zoomTouches[1].point;
					
					float initialDistance = sqrtf(
									  (zoomStartPositions[0].x - zoomStartPositions[1].x) * 
									  (zoomStartPositions[0].x - zoomStartPositions[1].x) + 
									  
									  (zoomStartPositions[0].y - zoomStartPositions[1].y) * 
									  (zoomStartPositions[0].y - zoomStartPositions[1].y));
					
					float currentDistance = sqrtf(
									  (zoomCurrentPositions[0].x - zoomCurrentPositions[1].x) * 
									  (zoomCurrentPositions[0].x - zoomCurrentPositions[1].x) + 
									  
									  (zoomCurrentPositions[0].y - zoomCurrentPositions[1].y) * 
									  (zoomCurrentPositions[0].y - zoomCurrentPositions[1].y));
					
					float32 saveZoomScale = zoomScale;
					float32 changeCoeff = initialDistance/currentDistance;
					float32 newScale = prevZoomScale * ((1.f - changeCoeff)/2.5f + 1.f);

					if(newScale > maxScale)newScale = maxScale;
					if(newScale < minScale)newScale = minScale; 
					
					SetScale(newScale);
					
					Vector2 center = Vector2((zoomStartPositions[0].x + zoomStartPositions[1].x) / 2,
									(zoomStartPositions[0].y + zoomStartPositions[1].y) / 2);
					
					Vector2 scaleVectorOriginal = Vector2(center.x - scrollOrigin.x, center.y - scrollOrigin.y);
					Vector2 scaleVectorNew = scaleVectorOriginal;
					scaleVectorNew.x *= zoomScale / saveZoomScale;
					scaleVectorNew.y *= zoomScale / saveZoomScale;
					
					scaleVectorNew.x -= scaleVectorOriginal.x;
					scaleVectorNew.y -= scaleVectorOriginal.y;
					
					scrollOrigin.x -= scaleVectorNew.x;
					scrollOrigin.y -= scaleVectorNew.y;
				}
			break;
			case UIEvent::PHASE_ENDED:
			{
				Vector<UIEvent>::iterator it = touches.begin();
				bool zoomToScroll = false;
				for(; it != touches.end(); ++it)
				{
					if((*it).phase == UIEvent::PHASE_DRAG)
					{
						zoomToScroll = true;
						scrollTouch = *it;
					}
				}
				
				if(zoomToScroll)
				{
					Vector2 start = scrollTouch.point;
					StartScroll(start);
					state = STATE_SCROLL;
				}
				else
				{
					if (state != STATE_DECCELERATION) 
					{
						state = STATE_NONE;
					}
				}
			}
			break;
		}
	}
}

void UIZoomControl::SystemDraw(const UIGeometricData &geometricData)
{
	Vector2 drawPos;
	drawPos.x = (drawScrollPos.x);
	drawPos.y = (drawScrollPos.y);
	
	UIGeometricData drawData;
	drawData.position = relativePosition;
	drawData.size = size;
	drawData.pivotPoint = pivotPoint;
	drawData.scale = scale;
	drawData.angle = angle;
	drawData.AddToGeometricData(geometricData);
	
	if(clipContents)
	{//WARNING: for now clip contents don't work for rotating controls if you have any ideas you are welcome
		RenderManager::Instance()->ClipPush();
		RenderManager::Instance()->ClipRect(drawData.GetUnrotatedRect());
	}
	
	drawData.position = relativePosition + drawPos;
	drawData.size = size;
	drawData.pivotPoint = pivotPoint;
	drawData.scale = scale;
	drawData.angle = angle;
	drawData.AddToGeometricData(geometricData);
	
	if(visible)
	{
		Draw(geometricData);
	}
	
	if (debugDrawEnabled)
	{
		RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        RenderHelper::Instance()->DrawRect(GetRect());
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	
    contentControl->SystemDraw(drawData);
	

	if(visible)
	{
		DrawAfterChilds(geometricData);
	}
	if(clipContents)
	{
		RenderManager::Instance()->ClipPop();
	}
}

bool UIZoomControl::IsScrolling()
{
    return (STATE_NONE != state);
}


const List<UIControl*>& UIZoomControl::GetChildren()
{
    return contentControl->GetChildren();
}


void UIZoomControl::AddControl(UIControl *control)
{
    contentControl->AddControl(control);
}

void UIZoomControl::RemoveControl(UIControl *control)
{
    contentControl->RemoveControl(control);    
}

void UIZoomControl::RemoveAllControls()
{
    contentControl->RemoveAllControls();
}

void UIZoomControl::BringChildFront(UIControl *_control)
{
    contentControl->BringChildFront(_control);
}

void UIZoomControl::BringChildBack(UIControl *_control)
{
    contentControl->BringChildBack(_control);
}

void UIZoomControl::InsertChildBelow(UIControl * _control, UIControl * _belowThisChild)
{
    contentControl->InsertChildBelow(_control, _belowThisChild);
}

void UIZoomControl::InsertChildAbove(UIControl * _control, UIControl * _aboveThisChild)
{
    contentControl->InsertChildAbove(_control, _aboveThisChild);
}

void UIZoomControl::SendChildBelow(UIControl * _control, UIControl * _belowThisChild)
{
    contentControl->SendChildAbove(_control, _belowThisChild);
}

void UIZoomControl::SendChildAbove(UIControl * _control, UIControl * _aboveThisChild)
{
    contentControl->SendChildAbove(_control, _aboveThisChild);
}



