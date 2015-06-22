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


#ifndef __DAVAENGINE_THEORA_PLAYER_H__
#define __DAVAENGINE_THEORA_PLAYER_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "FileSystem/File.h"

#if !defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{

struct TheoraData;
    
class TheoraPlayer : public UIControl
{
public:
    
    /**
     \brief Constructor
     \param[in] filePath path to video file
	 */
    TheoraPlayer(const FilePath & filePath = FilePath());

    virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
    
	/**
	 \brief Calls on every frame to draw control.
     Can be overriden to implement custom draw functionality.
     Default realization is drawing UIControlBackground with requested parameters.
	 \param[in] geometricData Control geometric data.
	 */
	virtual void Draw(const UIGeometricData &geometricData);
    
    /**
     \brief open video file
     \param[in] filePath path to video file
	 */
    void OpenFile(const FilePath & filePath);
    
    /**
     \brief release theora data and close file
	 */
    void ReleaseData();
    
    /**
	 \brief Calls on every frame with frame delata time parameter. 
     Should be overriden to implement perframe functionality.
     Default realization is empty.
	 \param[in] timeElapsed Current frame time delta.
	 */
    virtual void Update(float32 timeElapsed);
    
    /**
	 \brief Set player playing state (play/pause)
	 \param[in] isPlaying use true to set state to playing, false - to pause
	 */
    void SetPlaying(bool isPlaying);
    
    /**
	 \brief return player playing state
     return player true if state is playing, false - if paused
	 */
    bool IsPlaying();
    
    /**
	 \brief Set player repeat state
	 \param[in] isPlaying true for repeat file, false - to play file onсe time
	 */
    void SetRepeat(bool isRepeat);
    
    /**
	 \brief return player repeat state
     return return player repeat state
	 */
    bool IsRepeat();
    
protected:
    ~TheoraPlayer();
    
private:
    int32 BufferData();
    
    float32             currFrameTime;
    float32             frameTime;
    
    FilePath              filePath;
    float32             videoTime;
    float32             videoBufTime;
    File                * file;
    unsigned char       * frameBuffer;
    int32               frameBufferW;
    int32               frameBufferH;
    TheoraData          * theoraData;
    int32               theora_p;
    bool                isVideoBufReady;
    bool                isPlaying;
    bool                isRepeat;
    uint32              repeatFilePos;
    int32               pp_level_max;
    int32               pp_level;
    int32               pp_inc;
};

}

#endif //#if !defined(__DAVAENGINE_ANDROID__)

#endif
