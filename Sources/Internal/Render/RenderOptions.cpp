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



#include "Render/RenderOptions.h"

namespace DAVA
{

FastName optionsNames[RenderOptions::OPTIONS_COUNT] = 
{
    FastName("Test Option"),

    FastName("Draw Landscape"),
    FastName("Draw Water Refl/Refr"),
    FastName("Draw Opaque Layer"),
    FastName("Draw Transparent Layer"),
    FastName("Draw Sprites"),
    FastName("Draw Shadow Volumes"),
    FastName("Draw Vegetation"),

    FastName("Enable Fog"),

    FastName("Update LODs"),
    FastName("Update Landscape LODs"), 
    FastName("Update Animations"), 
    FastName("Process Clipping"),
    FastName("Update UI System"),

    FastName("SpeedTree Animations"),
    FastName("Waves System Process"),

    FastName("All Render Enabled"),
    FastName("Texture Loading"),

    FastName("Occlusion Stats"),

    FastName("Static Occlusion"),
    FastName("Debug Draw Occlusion"),

    FastName("Update Particle Emitters"),
    FastName("Draw Particles"),
    FastName("Particle Prepare Buffers"),
#if defined(LOCALIZATION_DEBUG)
    FastName("Localization Warings"),
    FastName("Localization Errors"),
    FastName("Line Break Errors"),
#endif
    FastName("Highlight Hard Controls")
};

RenderOptions::RenderOptions()
{
	for(int32 i = 0; i < OPTIONS_COUNT; ++i)
	{
		options[i] = true;
	}		

    options[DEBUG_DRAW_STATIC_OCCLUSION] = false;
    options[LAYER_OCCLUSION_STATS] = false;
    options[REPLACE_MIPMAPS] = false;
#if defined(LOCALIZATION_DEBUG)
    options[DRAW_LOCALIZATION_ERRORS] = false;
    options[DRAW_LOCALIZATION_WARINGS] = false;
    options[DRAW_LINEBREAK_ERRORS] = false;
#endif
    options[HIGHLIGHT_HARD_CONTROLS] = false;
}

bool RenderOptions::IsOptionEnabled(RenderOption option)
{
	return options[option];
}

void RenderOptions::SetOption(RenderOption option, bool value)
{
	options[option] = value;
	NotifyObservers();
}

FastName RenderOptions::GetOptionName(RenderOption option)
{
    return optionsNames[option];
}

};
