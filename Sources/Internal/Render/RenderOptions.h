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


#ifndef __DAVAENGINE_RENDEROPTIONS_H__
#define __DAVAENGINE_RENDEROPTIONS_H__

#include "Base/BaseTypes.h"
#include "Base/Observable.h"
#include "Base/FastName.h"

namespace DAVA
{

class RenderOptions : public Observable
{
public:
	enum RenderOption
	{
        TEST_OPTION = 0,

		LANDSCAPE_DRAW,
		WATER_REFLECTION_REFRACTION_DRAW,
		OPAQUE_DRAW,
		TRANSPARENT_DRAW,
        SPRITE_DRAW,
        SHADOWVOLUME_DRAW,
        VEGETATION_DRAW,

		FOG_ENABLE,

		UPDATE_LODS, 
		UPDATE_LANDSCAPE_LODS, 
		UPDATE_ANIMATIONS, 
		PROCESS_CLIPPING,
		UPDATE_UI_CONTROL_SYSTEM,

        SPEEDTREE_ANIMATIONS,
        WAVE_DISTURBANCE_PROCESS,

        ALL_RENDER_FUNCTIONS_ENABLED,
        TEXTURE_LOAD_ENABLED,
        
        LAYER_OCCLUSION_STATS,

        ENABLE_STATIC_OCCLUSION,
        DEBUG_DRAW_STATIC_OCCLUSION,

        UPDATE_PARTICLE_EMMITERS,
        PARTICLES_DRAW,
        PARTICLES_PREPARE_BUFFERS,
        REPLACE_ALBEDO_MIPMAPS,
        REPLACE_LIGHTMAP_MIPMAPS,
#if defined(LOCALIZATION_DEBUG)
        DRAW_LOCALIZATION_WARINGS,
        DRAW_LOCALIZATION_ERRORS,
        DRAW_LINEBREAK_ERRORS,
#endif
        DRAW_NONDEF_GLYPH,
        HIGHLIGHT_HARD_CONTROLS,
		OPTIONS_COUNT
	};

	bool IsOptionEnabled(RenderOption option);
	void SetOption(RenderOption option, bool value);
    FastName GetOptionName(RenderOption option);
    RenderOptions();
private:	

	bool options[OPTIONS_COUNT];
    
};

};

#endif //__DAVAENGINE_RENDEROPTIONS_H__
