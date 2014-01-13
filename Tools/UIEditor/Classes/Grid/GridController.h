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

#ifndef __GRIDCONTROLLER__H__
#define __GRIDCONTROLLER__H__

#include "DAVAEngine.h"
using namespace DAVA;

// This class is responsible for handling Grid in UI Editor and calculating the controls positions over it.
class GridController : public Singleton<GridController>
{
public:
    // Construction/destruction.
    GridController();
    virtual ~GridController();

    // Set the grid X and Y spacing.
    void SetGridSpacing(const Vector2& spacing);
    
    // Set the current screen scale.
    void SetScale(float32 scale);

    // Recalculate the mouse position according to the grid and current zoom level.
    Vector2 RecalculateMousePos(const Vector2& mousePos);

protected:
    // Calcute discrete step from position.
    inline float32 CalculateDiscreteStep(float32 position, float32 step);

private:
    // Grid spacing.
    Vector2 gridSpacing;
    
    // Screen scale.
    float32 screenScale;
};

inline float32 GridController::CalculateDiscreteStep(float32 position, float32 step)
{
    float32 scaledValue = position/step;
    float32 integerPart = floor(scaledValue);

    // Check the fractional part.
    if (scaledValue - integerPart < 0.5f)
    {
        return integerPart * step;
    }
    
    return (integerPart + 1) * step;
}

#endif /* defined(__GRIDCONTROLLER__H__) */
