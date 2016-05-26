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
    TheoraPlayer(const FilePath& filePath = FilePath());
    /**
    \brief Constructor
    \param[in] rect - the rect of UIControl
    */
    TheoraPlayer(const Rect& rect)
        : TheoraPlayer(FilePath())
    {
        SetRect(rect);
    }

    /**
	 \brief Calls on every frame to draw control.
     Can be overriden to implement custom draw functionality.
     Default realization is drawing UIControlBackground with requested parameters.
	 \param[in] geometricData Control geometric data.
	 */
    virtual void Draw(const UIGeometricData& geometricData);

    /**
     \brief open video file
     \param[in] filePath path to video file
	 */
    void OpenFile(const FilePath& filePath);

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
    void Play()
    {
        SetPlaying(true);
    }
    void Pause()
    {
        SetPlaying(false);
    }
    void Resume()
    {
        SetPlaying(true);
    }
    void Stop()
    {
        SetPlaying(false);
    }

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

    float32 currFrameTime;
    float32 frameTime;

    FilePath filePath;
    float32 videoTime;
    float32 videoBufTime;
    File* file;
    unsigned char* frameBuffer;
    int32 frameBufferW;
    int32 frameBufferH;
    TheoraData* theoraData;
    int32 theora_p;
    bool isVideoBufReady;
    bool isPlaying;
    bool isRepeat;
    uint32 repeatFilePos;
    int32 pp_level_max;
    int32 pp_level;
    int32 pp_inc;
};
}

#endif //#if !defined(__DAVAENGINE_ANDROID__)

#endif
