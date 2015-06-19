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


#include "Render/Highlevel/Vegetation/VegetationPropertyNames.h"

namespace DAVA
{
const FastName VegetationPropertyNames::UNIFORM_TILEPOS("tilePos");
const FastName VegetationPropertyNames::UNIFORM_WORLD_SIZE("worldSize");
const FastName VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE("heightmapScale");
const FastName VegetationPropertyNames::UNIFORM_SWITCH_LOD_SCALE("lodSwitchScale");
const FastName VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE("perturbationForce");
const FastName VegetationPropertyNames::UNIFORM_PERTURBATION_POINT("perturbationPoint");
const FastName VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE_DISTANCE("perturbationForceDistance");
    
const FastName VegetationPropertyNames::FLAG_LOD_COLOR("LOD_COLOR");
    
const FastName VegetationPropertyNames::VEGETATION_QUALITY_NAME_HIGH("HIGH");
const FastName VegetationPropertyNames::VEGETATION_QUALITY_NAME_LOW("LOW");
const FastName VegetationPropertyNames::VEGETATION_QUALITY_GROUP_NAME("Vegetation");
    
const FastName VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP("vegetationmap");

const FastName VegetationPropertyNames::UNIFORM_VEGWAVEOFFSET_X("vegWaveOffsetx");
const FastName VegetationPropertyNames::UNIFORM_VEGWAVEOFFSET_Y("vegWaveOffsety");
};