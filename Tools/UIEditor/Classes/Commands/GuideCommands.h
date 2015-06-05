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


#ifndef __GUIDECOMMANDS__H__
#define __GUIDECOMMANDS__H__

#include "BaseCommand.h"
#include "HierarchyTreeController.h"
#include "Guides/GuideData.h"

namespace DAVA {

// Base Guide command.
class BaseGuideCommand : public BaseCommand
{
public:
    BaseGuideCommand(const HierarchyTreeScreenNode* screenNode);
    
protected:
    void AddRememberedGuide(const GuideData& guideData);
    void RemoveRememberedGuide(const GuideData& guideData);
};
    
// Add the new guide.
class AddNewGuideCommand : public BaseGuideCommand
{
public:
	AddNewGuideCommand(const HierarchyTreeScreenNode* screenNode);

	virtual void Execute();
	virtual void Rollback();
    
	virtual bool IsUndoRedoSupported() {return true;};
    
protected:
    void AcceptNewGuide();

private:
    bool isFirstAdd;
    GuideData guideData;
};

// Move the guide by mouse.
class MoveGuideByMouseCommand : public BaseGuideCommand
{
public:
    MoveGuideByMouseCommand(const HierarchyTreeScreenNode* screenNode);
    
    virtual void Execute();
    virtual void Rollback();

   	virtual bool IsUndoRedoSupported() {return true;};
    
protected:
    void UpdateGuidePosition(const GuideData& guideData, const Vector2& newPos);
    
private:
    bool isFirstMovePerformed;
    Vector2 startGuidePos;
    GuideData movedGuideData;
};

// Move guide by setting its position.
class MoveGuideCommand : public BaseGuideCommand
{
public:
    MoveGuideCommand(const HierarchyTreeScreenNode* screenNode, const Vector2& delta);

    virtual void Execute();
    virtual void Rollback();
    
   	virtual bool IsUndoRedoSupported() {return true;};
    
private:
    bool isFirstMovePerformed;
    Vector2 moveDelta;
    List<Vector2> guidesPositions;
};

// Delete the single guide.
class DeleteSingleGuideCommand : public BaseGuideCommand
{
public:
    DeleteSingleGuideCommand(const HierarchyTreeScreenNode* screenNode, const GuideData& guideData);

    virtual void Execute();
    virtual void Rollback();

    virtual bool IsUndoRedoSupported() {return true;};

private:
    bool isFirstDeletePerformed;
    GuideData guideDataToDelete;
};

// Delete the guides.
class DeleteGuidesCommand : public BaseGuideCommand
{
public:
    DeleteGuidesCommand(const HierarchyTreeScreenNode* screenNode);

    virtual void Execute();
    virtual void Rollback();

    virtual bool IsUndoRedoSupported() {return true;};

protected:
    void AddRememberedGuides();
    void RemoveRememberedGuides();

private:
    bool isFirstDeletePerformed;
    List<GuideData> deletedGuidesData;
};

};

#endif /* defined(__GUIDECOMMANDS__H__) */
