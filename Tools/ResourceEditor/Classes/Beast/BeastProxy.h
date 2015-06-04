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


#ifndef __BEAST_PROXY__
#define __BEAST_PROXY__

#include "DAVAEngine.h"

class BeastManager;
struct LightmapAtlasingData;
class BeastProxy : public DAVA::Singleton<BeastProxy>
{
public:
    enum eBeastMode
    {
        MODE_LIGHTMAPS = 0,
        MODE_SPHERICAL_HARMONICS,
        MODE_PREVIEW
    };

	virtual BeastManager * CreateManager();
	virtual void SafeDeleteManager(BeastManager ** manager) {};
	virtual void Update(BeastManager * manager) {};
	virtual bool IsJobDone(BeastManager * manager) {return false;}

	virtual int GetCurTaskProcess(BeastManager * manager) const { return 0; };
	virtual DAVA::String GetCurTaskName(BeastManager * manager) const { return ""; };

	virtual void Run(BeastManager * manager, DAVA::Scene * scene) {};
	virtual void SetLightmapsDirectory(BeastManager * manager, const DAVA::FilePath & path) {};
	virtual void SetMode(BeastManager * manager, eBeastMode mode) {};

	virtual void UpdateAtlas(BeastManager * manager, DAVA::Vector<LightmapAtlasingData> * atlasData) {};
	virtual void Cancel(BeastManager * beastManager) {};
};

#endif //__BEAST_PROXY__
