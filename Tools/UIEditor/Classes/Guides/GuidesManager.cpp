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

#include "GuidesManager.h"

namespace DAVA {

// Minimum distance between rect and guide to apply stick.
#define GUIDE_STICK_TRESHOLD 10.0f
    
// Minimum distance between the point and the guide to decide "guide is under point".
#define GUIDE_POSITION_TRESHOLD 3.0f

GuidesManager::GuidesManager() :
    newGuide(NULL),
    moveGuide(NULL),
    stickMode(NotSticked),
    guidesEnabled(true),
    guidesLocked(false)
{
}

GuidesManager::~GuidesManager()
{
    Cleanup();
}

void GuidesManager::Cleanup()
{
    SafeDelete(newGuide);
    moveGuide = NULL;

    for (List<GuideData*>::iterator iter = activeGuides.begin(); iter !=  activeGuides.end(); iter ++)
    {
        SafeDelete(*iter);
    }
    
    activeGuides.clear();
}
    
void GuidesManager::StartNewGuide(GuideData::eGuideType guideType)
{
    if (newGuide)
    {
        // New guide creation is already in progress.
        return;
    }

    // Deselect all the existing guides - we are adding new one which should be selected.
    for (List<GuideData*>::iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        (*iter)->SetSelected(false);
    }

    newGuide = new GuideData(guideType, Vector2(), true);
}
    
void GuidesManager::MoveNewGuide(const Vector2& pos)
{
    if (!newGuide)
    {
        return;
    }
    
    newGuide->SetPosition(pos);
}

bool GuidesManager::CanAcceptNewGuide() const
{
    // Don't allow to add guides with the same coords.
    if (!newGuide || IsGuideExist(newGuide))
    {
        return false;
    }

    return true;
}

const GuideData* GuidesManager::AcceptNewGuide()
{
    if (!CanAcceptNewGuide())
    {
        CancelNewGuide();
        return NULL;
    }

    activeGuides.push_back(newGuide);

    GuideData* acceptedGuide = newGuide;
    newGuide = NULL; // don't release memory here - the guide is now in list.
    return acceptedGuide;
}

void GuidesManager::CancelNewGuide()
{
    SafeDelete(newGuide);
}
    
const List<GuideData*> GuidesManager::GetGuides(bool includeNewGuide) const
{
    List<GuideData*> allGuidesList = activeGuides;
    if (includeNewGuide && newGuide)
    {
        allGuidesList.push_back(newGuide);
    }
    
    return allGuidesList;
}

void GuidesManager::AddGuide(const GuideData& guideData)
{
    GuideData* newGuideData = new GuideData(guideData);
    activeGuides.push_back(newGuideData);
}

bool GuidesManager::RemoveGuide(const GuideData& guideData)
{
    for (List<GuideData*>::iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        GuideData* curGuide = *iter;
        if (*curGuide == guideData)
        {
            activeGuides.erase(iter);
            SafeDelete(curGuide);
            return true;
        }
    }
    
    return false;
}

bool GuidesManager::UpdateGuidePosition(const GuideData& guideData, const Vector2& newPos)
{
    for (List<GuideData*>::iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        GuideData* curGuide = *iter;
        if (*curGuide == guideData)
        {
            curGuide->SetPosition(newPos);
            return true;
        }
    }

    return false;
}

bool GuidesManager::IsGuideExist(GuideData* guideData) const
{
    if (!guideData)
    {
        DVASSERT(false);
        return false;
    }

    for (List<GuideData*>::const_iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        GuideData* curGuide = *iter;
        if (!curGuide || curGuide->GetType() != guideData->GetType())
        {
            continue;
        }

        switch (curGuide->GetType())
        {
            case GuideData::Horizontal:
            {
                if (curGuide->GetPosition().y == guideData->GetPosition().y)
                {
                    return true;
                }
                
                break;
            }
                
            case GuideData::Vertical:
            {
                if (curGuide->GetPosition().x == guideData->GetPosition().x)
                {
                    return true;
                }
                
                break;
            }
                
            case GuideData::Both:
            {
                if (curGuide->GetPosition() == guideData->GetPosition())
                {
                    return true;
                }
                
                break;
            }
                
            default:
            {
                break;
            }
        }
    }
    
    return false;
}

bool GuidesManager::StartMoveGuide(const Vector2& pos)
{
    GuideData* selectedGuideData = NULL;
    if (AreGuidesLocked())
    {
        // If guides are locked and no selected guide exist - discard the move. Selected guides are
        // reset while guides are locked, so selected guide will exist only if it was added as new one.
        // In this case allow guide move till the selection will be reset.
        for (List<GuideData*>::iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
        {
            GuideData* curGuideData = *iter;
            if (curGuideData->IsSelected() && IsGuideOnPosition(curGuideData, pos))
            {
                selectedGuideData = curGuideData;
                break;
            }
        }
        
        if (!selectedGuideData)
        {
            // Click happened outside of selected guide, so reset the current selection. Next move
            // will be available only after Locked flag will be reset.
            ResetSelection();
        }
    }
    else
    {
        // Lookup for the guide which is about to be moved and also update the selection flags.
        for (List<GuideData*>::iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
        {
            GuideData* curGuideData = *iter;
            curGuideData->SetSelected(false);
            if (IsGuideOnPosition(curGuideData, pos))
            {
                selectedGuideData = curGuideData;
                break;
            }
        }
    }

    if (selectedGuideData)
    {
        selectedGuideData->SetSelected(true);
        moveGuide = selectedGuideData;
        moveGuideStartPos = moveGuide->GetPosition();
    }

    return (selectedGuideData != NULL);
}

Vector2 GuidesManager::GetMoveGuideStartPos() const
{
    return moveGuideStartPos;
}

void GuidesManager::MoveGuide(const Vector2& pos)
{
    if (!moveGuide)
    {
        return;
    }
    
    Vector2 moveDelta = pos - moveGuideStartPos;
    moveGuide->SetPosition(moveGuideStartPos + moveDelta);
}

const GuideData* GuidesManager::AcceptMoveGuide()
{
    GuideData* resultData = moveGuide;
    moveGuide = NULL;
    
    return resultData;
}

bool GuidesManager::IsGuideOnPosition(GuideData* guide, const Vector2& pos) const
{
    if (!guide)
    {
        return false;
    }

    switch (guide->GetType())
    {
        case GuideData::Horizontal:
        {
            return fabs(guide->GetPosition().y - pos.y) < GUIDE_POSITION_TRESHOLD;
        }
        
        case GuideData::Vertical:
        {
            return fabs(guide->GetPosition().x - pos.x) < GUIDE_POSITION_TRESHOLD;
        }
            
        case GuideData::Both:
        {
            Vector2 distance = guide->GetPosition() - pos;
            return distance.Length() < GUIDE_POSITION_TRESHOLD;
        }
            
        default:
        {
            return false;
        }
    }
    
    // Should not be here though.
    return false;
}

bool GuidesManager::AreGuidesSelected() const
{
    for (List<GuideData*>::const_iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        if ((*iter)->IsSelected())
        {
            return true;
        }
    }
    
    return false;
}

void GuidesManager::ResetSelection()
{
    for (List<GuideData*>::iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        GuideData* curGuideData = *iter;
        curGuideData->SetSelected(false);
    }
}

List<GuideData> GuidesManager::DeleteSelectedGuides()
{
    List<GuideData*> selectedGuides;
    for (List<GuideData*>::iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        if ((*iter)->IsSelected())
        {
            selectedGuides.push_back(*iter);
        }
    }

    List<GuideData> deletedGuides;
    for (List<GuideData*>::iterator iter = selectedGuides.begin(); iter != selectedGuides.end(); iter ++)
    {
        deletedGuides.push_back(*(*iter));
        activeGuides.remove(*iter);
    }

    return deletedGuides;
}

bool GuidesManager::Load(const FilePath& fileName)
{
	YamlParser * parser = YamlParser::Create(fileName);
	if (!parser)
	{
		Logger::Error("GuidesManager::Load: Failed to open yaml file: %s", fileName.GetAbsolutePathname().c_str());
		return false;
	}
    
	YamlNode * rootNode = parser->GetRootNode();
    if (!rootNode)
    {
        Logger::Warning("GuidesManager::Load: YAML file: %s is empty", fileName.GetAbsolutePathname().c_str());
        SafeRelease(parser);
        return false;
    }

	Cleanup();
    const YamlNode* guidesNode = rootNode->Get("guides");
    if (!guidesNode)
    {
        // No Guides needed in this document - this is OK.
        SafeRelease(parser);
        return true;
    }
    
    for (MultiMap<String, YamlNode*>::const_iterator iter = guidesNode->AsMap().begin(); iter != guidesNode->AsMap().end(); iter ++)
    {
        YamlNode* node = iter->second;
        const YamlNode* typeNode = node->Get("type");
        const YamlNode* positionNode = node->Get("position");
        
        if (typeNode && positionNode)
        {
            GuideData* guideData = new GuideData((GuideData::eGuideType)typeNode->AsInt32(), positionNode->AsVector2());
            activeGuides.push_back(guideData);
        }
    }

    SafeRelease(parser);
    return true;
}

bool GuidesManager::Save(const FilePath& fileName, uint32 fileAttr)
{
    if (activeGuides.empty())
    {
        // Nothing to save - treat as OK.
        return true;
    }

    YamlParser * parser = YamlParser::Create();
    if (!parser)
    {
        Logger::Error("GuidesManager::Save: error while creating YAML parser!");
        return false;
    }

    YamlNode* rootNode = new YamlNode(YamlNode::TYPE_MAP);
    YamlNode* guidesNode = new YamlNode(YamlNode::TYPE_MAP);
    rootNode->AddNodeToMap("guides", guidesNode);

    int guideIndex = 0;
    for (List<GuideData*>::iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        GuideData* activeGuide = *iter;
        YamlNode* guideNode = new YamlNode(YamlNode::TYPE_MAP);
        guideNode->Add("type", activeGuide->GetType());
        guideNode->Add("position", activeGuide->GetPosition());

        guidesNode->AddNodeToMap(Format("guide%i", guideIndex), guideNode);
        guideIndex ++;
    }

    bool saveResult = parser->SaveToYamlFile(fileName, rootNode, true, fileAttr);
    SafeRelease(parser);

    return saveResult;
}

int32 GuidesManager::CalculateStickToGuides(const List<Rect>& controlsRectList, Vector2& offset) const
{
    Vector2 minDistance(FLT_MAX, FLT_MAX);

    // Lookup for the minimum distance from any of the controls rect to the any of the guides.
    for(List<GuideData*>::const_iterator iter = activeGuides.begin(); iter != activeGuides.end(); iter ++)
    {
        GuideData* activeGuide = *iter;
        for (List<Rect>::const_iterator rectIter = controlsRectList.begin(); rectIter != controlsRectList.end(); rectIter ++)
        {
            const Rect& rect = *rectIter;
            Vector2 distance = CalculateDistanceToGuide(activeGuide, rect);
            if (fabs(distance.x) < fabs(minDistance.x))
            {
                minDistance.x = distance.x;
            }
            if (fabs(distance.y) < fabs(minDistance.y))
            {
                minDistance.y = distance.y;
            }
        }
    }

    offset.SetZero();
    int result = NotSticked;
    if (fabs(minDistance.x) < GUIDE_STICK_TRESHOLD)
    {
        offset.x = minDistance.x;
        result |= StickedToX;
    }
    if (fabs(minDistance.y) < GUIDE_STICK_TRESHOLD)
    {
        offset.y = minDistance.y;
        result |= StickedToY;
    }

    return result;
}

int32 GuidesManager::GetGuideStickTreshold() const
{
    return GUIDE_STICK_TRESHOLD;
}

void GuidesManager::SetStickMode(int32 mode)
{
    stickMode = mode;
}

Vector2 GuidesManager::CalculateDistanceToGuide(GuideData* guide, const Rect& rect) const
{
    float32 minSidesX = FLT_MAX;
    float32 minSidesY = FLT_MAX;

    if (!guide || stickMode == StickDisabled)
    {
        return Vector2(minSidesX, minSidesY);
    }

    if (stickMode & StickToSides)
    {
        switch (guide->GetType())
        {
            case GuideData::Horizontal:
            {
                float32 distanceToTop = rect.y - guide->GetPosition().y;
                float32 distanceToBottom = rect.y + rect.dy - guide->GetPosition().y;
                minSidesY = fabs(distanceToTop) < fabs(distanceToBottom) ? distanceToTop : distanceToBottom;
                break;
            }

            case GuideData::Vertical:
            {
                float32 distanceToLeft = rect.x - guide->GetPosition().x;
                float32 distanceToRight = rect.x + rect.dx - guide->GetPosition().x;
                minSidesX = fabs(distanceToLeft) < fabs(distanceToRight) ? distanceToLeft : distanceToRight;
                break;
            }

            case GuideData::Both:
            {
                float32 distanceToLeft = rect.x - guide->GetPosition().x;
                float32 distanceToRight = rect.x + rect.dx - guide->GetPosition().x;
                float32 distanceToTop = rect.y - guide->GetPosition().y;
                float32 distanceToBottom = rect.y + rect.dy - guide->GetPosition().y;

                minSidesX = fabs(distanceToLeft) < fabs(distanceToRight) ? distanceToLeft : distanceToRight;
                minSidesY = fabs(distanceToTop) < fabs(distanceToBottom) ? distanceToTop : distanceToBottom;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    float32 minCentersX = FLT_MAX;
    float32 minCentersY = FLT_MAX;

    if (stickMode & StickToCenters)
    {
        Vector2 distanceToCenter = rect.GetCenter() - guide->GetPosition();
        switch ( guide->GetType())
        {
            case GuideData::Horizontal:
            {
                minCentersY = distanceToCenter.y;
                break;
            }
                
            case GuideData::Vertical:
            {
                minCentersX = distanceToCenter.x;
                break;
            }

            case GuideData::Both:
            {
                minCentersX = distanceToCenter.x;
                minCentersY = distanceToCenter.y;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    Vector2 resultVector;
    resultVector.x = fabs(minSidesX) < fabs(minCentersX) ? minSidesX : minCentersX;
    resultVector.y = fabs(minSidesY) < fabs(minCentersY) ? minSidesY : minCentersY;
 
    return resultVector;
}

bool GuidesManager::AreGuidesEnabled() const
{
    return guidesEnabled;
}
    
void GuidesManager::SetGuidesEnabled(bool value)
{
    guidesEnabled = value;
}

bool GuidesManager::AreGuidesLocked() const
{
    return guidesLocked;
}

void GuidesManager::LockGuides(bool value)
{
    guidesLocked = value;
    if (value)
    {
        ResetSelection();
    }
}

};
