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

#ifndef __QUICKED_TRANSFORM_SYSTEM_H__
#define __QUICKED_TRANSFORM_SYSTEM_H__

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "UI/UIControl.h"

class TransformSystem final : public BaseEditorSystem
{
public:
    explicit TransformSystem(EditorSystemsManager* parent);
    ~TransformSystem() = default;

    bool OnInput(DAVA::UIEvent* currentInput) override;

private:
    using PropertyDelta = std::pair<AbstractProperty* /*property*/, DAVA::VariantType /*delta*/>;

    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnActiveAreaChanged(const HUDAreaInfo& areaInfo);

    bool ProcessKey(const DAVA::int32 key);

    bool ProcessDrag(DAVA::Vector2 point);
    void ResizeControl(DAVA::Vector2 delta, bool withPivot, bool rateably);
    void AdjustResize(DAVA::Array<int, DAVA::Vector2::AXIS_COUNT> directions, DAVA::Vector2& delta);
    void MovePivot(DAVA::Vector2 delta);
    DAVA::Vector2 AdjustPivot(DAVA::Vector2& delta);
    void Rotate(DAVA::Vector2 pos);
    void MoveAllSelectedControls(DAVA::Vector2 delta);
    DAVA::Vector2 AdjustMove(DAVA::Vector2 delta);
    void AdjustProperty(ControlNode* node, const DAVA::Vector<PropertyDelta>& propertiesDelta);

    void CorrectNodesToMove();

    HUDAreaInfo::eArea activeArea = HUDAreaInfo::NO_AREA;
    ControlNode* activeControlNode = nullptr;
    DAVA::Vector2 prevPos;
    DAVA::Vector2 extraDelta;
    SelectedControls selectedControlNodes;
    DAVA::List<std::tuple<ControlNode* /*node*/, AbstractProperty* /*positionProperty*/, const DAVA::UIGeometricData* /*parent gd*/>> nodesToMove;
    const DAVA::Vector2 minimumSize = DAVA::Vector2(16.0f, 16.0f);
    size_t currentHash = 0;

    DAVA::UIGeometricData parentGeometricData;
    DAVA::UIGeometricData controlGeometricData;
    AbstractProperty* sizeProperty = nullptr;
    AbstractProperty* positionProperty = nullptr;
    AbstractProperty* angleProperty = nullptr;
    AbstractProperty* pivotProperty = nullptr;
};

#endif // __QUICKED_TRANSFORM_SYSTEM_H__
