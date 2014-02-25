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



#include "LodDistanceControl.h"
#include "ControlsFactory.h"

#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../EditorScene.h"

LodDistanceControl::LodDistanceControl(LodDistanceControlDelegate *newDelegate, const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
{
    count = 0;

    maxDistance = 0;
    
    zones = NULL;
    sliders = NULL;
    
    delegate = newDelegate;
    
    mainTouch = -1;
    oldPos = newPos = 0.f;
    
    activeSlider = NULL;
    leftZone = NULL;
    rightZone = NULL;
    
    activeLodIndex = -1;
    
    for(int32 iDist = 0; iDist < LodComponent::MAX_LOD_LAYERS; ++iDist)
    {
        distanceText[iDist] = new UIStaticText(Rect(0, (float32)((iDist+1) * ControlsFactory::BUTTON_HEIGHT), 
                                                    rect.dx / 2.f, (float32)ControlsFactory::BUTTON_HEIGHT));
        
        distanceText[iDist]->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
        distanceText[iDist]->SetFont(ControlsFactory::GetFont12());
		distanceText[iDist]->SetTextColor(ControlsFactory::GetColorLight());
        
        
        distanceTextValues[iDist] = new UITextField(Rect(rect.dx / 2.f, (float32)((iDist+1) * ControlsFactory::BUTTON_HEIGHT), 
                                                         rect.dx / 2.f, (float32)(ControlsFactory::BUTTON_HEIGHT)));
        ControlsFactory::CustomizeEditablePropertyCell(distanceTextValues[iDist]);
        distanceTextValues[iDist]->SetFont(ControlsFactory::GetFont12());
		distanceTextValues[iDist]->SetTextColor(ControlsFactory::GetColorLight());
        distanceTextValues[iDist]->SetDelegate(this);
        distanceTextValues[iDist]->SetInputEnabled((0 != iDist), false);
        
        
        trianglesText[iDist] = new UIStaticText(Rect(0, 0,
                                                    rect.dx / 2.f, (float32)ControlsFactory::BUTTON_HEIGHT));
        
        trianglesText[iDist]->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
        trianglesText[iDist]->SetFont(ControlsFactory::GetFont12());
		trianglesText[iDist]->SetTextColor(ControlsFactory::GetColorLight());
        
        trianglesTextValues[iDist] = new UIStaticText(Rect(rect.dx / 2.f, 0,
                                                     rect.dx / 2.f, (float32)ControlsFactory::BUTTON_HEIGHT));
        
        trianglesTextValues[iDist]->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
        trianglesTextValues[iDist]->SetFont(ControlsFactory::GetFont12());
		trianglesTextValues[iDist]->SetTextColor(ControlsFactory::GetColorLight());
    }
    
    distanceToCameraText = new UIStaticText(Rect(0, 0, rect.dx / 2.f, (float32)ControlsFactory::BUTTON_HEIGHT));
    distanceToCameraText->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    distanceToCameraText->SetFont(ControlsFactory::GetFont12());
	distanceToCameraText->SetTextColor(ControlsFactory::GetColorLight());
    distanceToCameraText->SetText(WideString(L"From Camera:"));
    AddControl(distanceToCameraText);
    
    distanceToCameraValue = new UIStaticText(Rect(rect.dx / 2.f, 0, rect.dx / 2.f, (float32)ControlsFactory::BUTTON_HEIGHT));
    distanceToCameraValue->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    distanceToCameraValue->SetFont(ControlsFactory::GetFont12());
	distanceToCameraValue->SetTextColor(ControlsFactory::GetColorLight());
    AddControl(distanceToCameraValue);
}

LodDistanceControl::~LodDistanceControl()
{
    for(int32 iDist = 0; iDist < LodComponent::MAX_LOD_LAYERS; ++iDist)
    {
        SafeRelease(distanceText[iDist]);
        SafeRelease(distanceTextValues[iDist]);
        
        SafeRelease(trianglesText[iDist]);
        SafeRelease(trianglesTextValues[iDist]);
    }
    
    SafeRelease(distanceToCameraText);
    SafeRelease(distanceToCameraValue);
    
    ReleaseControls();
}


void LodDistanceControl::WillDisappear()
{
    ReleaseControls();    

    UIControl::WillDisappear();
}


void LodDistanceControl::Update(float32 timeElapsed)
{
    UpdateDistanceToCamera();
    UIControl::Update(timeElapsed);
}

void LodDistanceControl::UpdateDistanceToCamera()
{
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    DVASSERT(activeScene);

    EditorScene *scene = activeScene->GetScene();
    DVASSERT(scene);

    Entity *selection = scene->GetSelection();
    if(selection)
    {
        Camera *activeCamera = scene->GetCurrentCamera();
        
        Vector3 cameraPosition = activeCamera->GetPosition();
        Vector3 selectionCenter = selection->GetWTMaximumBoundingBoxSlow().GetCenter();
        
        float32 distanceToCamera = (cameraPosition-selectionCenter).Length();
        distanceToCameraValue->SetText(Format(L"%f", distanceToCamera));
    }
}

void LodDistanceControl::ReleaseControls()
{
    for(int32 iZone = 0; iZone < count; ++iZone)
    {
        RemoveControl(zones[iZone]);
        SafeRelease(zones[iZone]);
    }
    SafeDeleteArray(zones);
    
    for(int32 iSlider = 0; iSlider < count - 1; ++iSlider)
    {
        RemoveControl(sliders[iSlider]);
        SafeRelease(sliders[iSlider]);
    }
    SafeDeleteArray(sliders);
    
    count = 0;
    
    activeSlider = NULL;
    leftZone = NULL;
    rightZone = NULL;
    
    activeLodIndex = -1;
}

void LodDistanceControl::SetDistances(float32 *newDistances, int32 *newTriangles, int32 newCount)
{
    Vector2 newSize = GetSize();
    newSize.y = GetControlHeightForLodCount(newCount);
    SetSize(newSize);
    
    ReleaseControls();
    
    if(1 < newCount)
    {
        count = newCount;
        Memcpy(distances, newDistances, count * sizeof(float32));
        Memcpy(triangles, newTriangles, count * sizeof(int32));
        
        zones = new UIControl*[count];
        
        maxDistance = (distances[count - 1] < LodComponent::MAX_LOD_DISTANCE) 
                                        ? LodComponent::MAX_LOD_DISTANCE 
                                        : (LodComponent::MAX_LOD_DISTANCE * 1.1f);
        
        Rect fullRect = GetRect();
        
        Color initialColor(0.8f, 0.5f, 0.1f, 1.0f);
        
        for(int32 iZone = 0; iZone < count; ++iZone)
        {
            float32 x = (iZone) ? distances[iZone] : 0.f;
            float32 dx = (iZone != count - 1) ? (distances[iZone + 1] - x) : (maxDistance - distances[iZone]);

            x = x * fullRect.dx / maxDistance;
            dx = dx * fullRect.dx / maxDistance;
            
            
            zones[iZone] = new UIControl(Rect(x, 0, dx, ControlsFactory::BUTTON_HEIGHT));
            zones[iZone]->SetInputEnabled(false, false);
            zones[iZone]->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
            
            float32 color = distances[iZone] / maxDistance;
            
            Color zoneColor;
            zoneColor.r = initialColor.r + color;
            zoneColor.g = initialColor.g + color;
            zoneColor.b = initialColor.b + color;
            zoneColor.a = 1.0f;
            
            if(1.0f < zoneColor.r) zoneColor.r -= (int32)zoneColor.r;
            if(1.0f < zoneColor.g) zoneColor.g -= (int32)zoneColor.g;
            if(1.0f < zoneColor.b) zoneColor.b -= (int32)zoneColor.b;
            
            zones[iZone]->GetBackground()->SetColor(zoneColor);
            
            AddControl(zones[iZone]);
        }
        
        sliders = new UIControl*[count - 1];
        for(int32 iSlider = 0; iSlider < count - 1; ++iSlider)
        {
            float32 x = distances[iSlider + 1] / maxDistance * fullRect.dx;
            sliders[iSlider] = new UIControl(Rect(x - 1, 0, 3, ControlsFactory::BUTTON_HEIGHT));
            sliders[iSlider]->SetInputEnabled(false, false);
            sliders[iSlider]->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
            sliders[iSlider]->GetBackground()->SetColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
            
            AddControl(sliders[iSlider]);
        }
    }
    
    float32 x = GetRect().dx / 2.f;
    for(int32 iDist = 0; iDist < LodComponent::MAX_LOD_LAYERS; ++iDist)
    {
        RemoveControl(distanceText[iDist]);
        RemoveControl(distanceTextValues[iDist]);
        RemoveControl(trianglesText[iDist]);
        RemoveControl(trianglesTextValues[iDist]);
        
        if(iDist < count)
        {
            AddControl(distanceText[iDist]);
            distanceText[iDist]->SetText(Format(L"Distance_%d:", iDist));

            AddControl(distanceTextValues[iDist]);
            distanceTextValues[iDist]->SetText(Format(L"%.0f", distances[iDist]));
            
            
            float32 y = (float32)(count + iDist + 1) * (float32)ControlsFactory::BUTTON_HEIGHT;
            trianglesText[iDist]->SetPosition(Vector2(0, y));
            AddControl(trianglesText[iDist]);
            trianglesText[iDist]->SetText(Format(L"Triangles_%d:", iDist));
            
            trianglesTextValues[iDist]->SetPosition(Vector2(x, y));
            AddControl(trianglesTextValues[iDist]);
            trianglesTextValues[iDist]->SetText(Format(L"%d", triangles[iDist]));
        }
    }
    
    float32 distanceToCameraY = (count * 2.f + 1.f) * (float32)ControlsFactory::BUTTON_HEIGHT;
    distanceToCameraText->SetPosition(Vector2(0, distanceToCameraY));
    distanceToCameraValue->SetPosition(Vector2(x, distanceToCameraY));
}

void LodDistanceControl::Input(UIEvent *currentInput)
{
    if(-1 == mainTouch && UIEvent::PHASE_BEGAN == currentInput->phase)
    {
        mainTouch =  currentInput->tid;
    }

    
    if(mainTouch == currentInput->tid)
    {
        newPos = currentInput->point.x;
        Vector2 point = currentInput->point - GetPosition(true);

        switch (currentInput->phase) 
        {
            case UIEvent::PHASE_BEGAN:
            {
                leftEdge = Vector2(0.0f, 0.0f);
                rightEdge = Vector2(GetRect().dx, 0.0f);
                
                for(int32 iSlider = 0; iSlider < count - 1; ++iSlider)
                {
                    Rect intersectionRect = sliders[iSlider]->GetRect();
                    intersectionRect.x -= 1;
                    intersectionRect.dx += 2;
                    
                    if(intersectionRect.PointInside(point))
                    {
                        activeSlider = sliders[iSlider];
                        
                        if(iSlider)
                        {
                            leftEdge.x = sliders[iSlider - 1]->GetRect().x + sliders[iSlider - 1]->GetRect().dx;
                        }
                        if(iSlider < count - 2)
                        {
                            rightEdge.x = sliders[iSlider + 1]->GetRect().x;
                        }
                        
                        leftZone = zones[iSlider];
                        rightZone = zones[iSlider + 1];
                        
                        activeLodIndex = iSlider + 1;
                        
                        break;
                    }
                }
                
                oldPos = newPos;
                break;
            }
            case UIEvent::PHASE_DRAG:
            {
                UpdateSliderPos();

                break;
            }
            case UIEvent::PHASE_ENDED:
            {
                UpdateSliderPos();

                activeSlider = NULL;
                leftZone = rightZone = NULL;
                
                mainTouch = -1;
                activeLodIndex = -1;
                break;
            }
        }
    }
}

void LodDistanceControl::UpdateSliderPos()
{
    if(activeSlider)
    {
        float32 deltax = newPos - oldPos;
        oldPos = newPos;

        Vector2 newPosition = activeSlider->GetPosition() + Vector2(deltax, 0.0f);
        newPosition.x = Max(newPosition.x, leftEdge.x);
        newPosition.x = Min(newPosition.x, rightEdge.x - 3);

        Vector2 deltaPosition = activeSlider->GetPosition() - newPosition;
        activeSlider->SetPosition(newPosition);
        
        Rect leftRect = leftZone->GetRect();
        leftRect.dx -= deltaPosition.x;
        leftZone->SetRect(leftRect);

        Rect rightRect = rightZone->GetRect();
        rightRect.x -= deltaPosition.x;
        rightRect.dx += deltaPosition.x;
        rightZone->SetRect(rightRect);
        
        float32 newDistance = ((newPosition.x + 1) / GetRect().dx * maxDistance);
        if(delegate)
        {
            delegate->DistanceChanged(this, activeLodIndex, newDistance);
        }
        
        distances[activeLodIndex] = newDistance;
        distanceTextValues[activeLodIndex]->SetText(Format(L"%.0f", newDistance));
    }
}


void LodDistanceControl::TextFieldShouldReturn(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LodDistanceControl::TextFieldShouldCancel(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LodDistanceControl::TextFieldLostFocus(UITextField * textField)
{
    for(int32 iText = 0; iText < count; ++iText)
    {
        if(textField == distanceTextValues[iText])
        {
            float32 newDistance = (float32)atof(WStringToString(textField->GetText()).c_str());
            
            newDistance = Max(newDistance, 0.f);
            newDistance = Min(newDistance, (float32)LodComponent::MAX_LOD_DISTANCE);
            if(iText)
            {
                newDistance = Max(newDistance, distances[iText-1]);
            }
            if(iText < count - 1)
            {
                newDistance = Min(newDistance, distances[iText + 1]);
            }

            //TODO: udpate sliders
            float32 deltaX = ((newDistance - distances[iText]) / maxDistance * GetRect().dx);
            if(iText)
            {
                Vector2 sz = zones[iText - 1]->GetSize();
                sz.x += deltaX; 
                zones[iText-1]->SetSize(sz);
                
                Vector2 pos = sliders[iText - 1]->GetPosition();
                pos.x += deltaX;
                sliders[iText - 1]->SetPosition(pos);
            }
            
            Rect r = zones[iText]->GetRect();
            r.x += deltaX; 
            r.dx -= deltaX;
            zones[iText]->SetRect(r);

            textField->SetText(Format(L"%.0f", newDistance));
            distances[iText] = newDistance;
            if(delegate)
            {
                delegate->DistanceChanged(this, iText, newDistance);
            }
            
            break;
        }
    }
}

bool LodDistanceControl::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    if (replacementLength < 0) 
    {
        return true;
    }
    
    WideString newText = textField->GetAppliedChanges(replacementLocation, replacementLength, replacementString);
    bool allOk;
    int pointsCount = 0;
    for (int32 i = 0; i < (int32)newText.length(); i++) 
    {
        allOk = false;
        if (newText[i] == L'-' && i == 0)
        {
            allOk = true;
        }
        else if(newText[i] >= L'0' && newText[i] <= L'9')
        {
            allOk = true;
        }
        else if(newText[i] == L'.' && pointsCount == 0)
        {
            allOk = true;
            pointsCount++;
        }
        if (!allOk) 
        {
            return false;
        }
    }
    return true;
};


const float32 LodDistanceControl::GetControlHeightForLodCount(int32 lodCount)
{
    return (lodCount * 2.0f + 2.0f) * (float32)ControlsFactory::BUTTON_HEIGHT;
}

