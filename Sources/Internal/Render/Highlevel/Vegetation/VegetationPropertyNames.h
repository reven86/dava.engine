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


#ifndef __DAVAENGINE_VEGETATIONPROPERTYNAMES_H__
#define __DAVAENGINE_VEGETATIONPROPERTYNAMES_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"

namespace DAVA
{

/**
 \brief Different uniform and shader flag names
 */
class VegetationPropertyNames
{
public:

    static const FastName UNIFORM_TILEPOS;
    static const FastName UNIFORM_WORLD_SIZE;
    static const FastName UNIFORM_HEIGHTMAP_SCALE;
    static const FastName UNIFORM_SWITCH_LOD_SCALE;
    static const FastName UNIFORM_PERTURBATION_FORCE;
    static const FastName UNIFORM_PERTURBATION_POINT;
    static const FastName UNIFORM_PERTURBATION_FORCE_DISTANCE;
    
    static const FastName FLAG_LOD_COLOR;
    
    static const FastName VEGETATION_QUALITY_NAME_HIGH;
    static const FastName VEGETATION_QUALITY_NAME_LOW;
    static const FastName VEGETATION_QUALITY_GROUP_NAME;
    
    static const FastName UNIFORM_SAMPLER_VEGETATIONMAP;
    
    static const FastName UNIFORM_VEGWAVEOFFSET_X;
    static const FastName UNIFORM_VEGWAVEOFFSET_Y;
};

};

#endif /* defined(__DAVAENGINE_VEGETATIONPROPERTYNAMES_H__) */
