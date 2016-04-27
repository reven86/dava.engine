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

#ifndef __DAVAENGINE_MOVIEVIEWCONTROL_H__
#define __DAVAENGINE_MOVIEVIEWCONTROL_H__

#include "UI/IMovieViewControl.h"
#include "UI/UIControl.h"
#include "Render\PixelFormatDescriptor.h"
#include "Sound/SoundSystem.h"

namespace AV
{
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
};
#endif
}

#include "Sound/FMODUtils.h"
#include "fmod.h"

#include "FileSystem/DynamicMemoryFile.h"

#include <queue>

namespace DAVA
{
struct DecodedFrameBuffer
{
    ~DecodedFrameBuffer()
    {
        SafeDeleteArray(data);
    }

    float64 frame_last_pts = 0.f;
    float64 pts = 0.f;
    float64 sleepAfterPresent = 0;
    PixelFormat textureFormat = FORMAT_INVALID;
    uint8* data;
    uint32 size = 0;
    uint32 width = 0;
    uint32 height = 0;
};

    class MovieViewControl : public IMovieViewControl, public UIControl
    {
    public:
        ~MovieViewControl() override;

        // IMovieViewControl Interface implementation

        // Initialize the control.
        void Initialize(const Rect& rect) override;

        // Position/visibility.
        void SetRect(const Rect& rect) override
        {
            UIControl::SetRect(rect);
        };
        void SetVisible(bool isVisible) override
        {
            UIControl::SetVisibilityFlag(isVisible);
        }

        // Open the Movie.
        void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override;

        // Start/stop the video playback.
        void Play() override;
        void Stop() override;

        // Pause/resume the playback.
        void Pause() override;
        void Resume() override;

        // Whether the movie is being played?
        bool IsPlaying() const override;


        // UIControl update and draw implementation
        void Update(float32 timeElapsed) override;

    private:
        AV::AVFormatContext* CreateContext(const FilePath& path);

        static FMOD_RESULT F_CALLBACK PcmReadDecodeCallback(FMOD_SOUND* sound, void* data, unsigned int datalen);

        float64 GetMasterClock();
        bool InitVideo();
        bool DecodeVideoPacket(AV::AVPacket* packet, float64& pts);
        void UpdateVideo();
        bool InitAudio();
        void DecodeAudio(AV::AVPacket* packet, float64 timeElapsed);

        void VideoDecodingThread(BaseObject* caller, void* callerData, void* userData);
        void VideoPresentationThread(BaseObject* caller, void* callerData, void* userData);
        void AudioDecodingThread(BaseObject* caller, void* callerData, void* userData);
        void ReadingThread(BaseObject* caller, void* callerData, void* userData);

        float64 synchronize_video(AV::AVFrame* src_frame, float64 pts);
        float64 GetPTSForFrame(AV::AVFrame* frame, AV::AVPacket* packet, uint32 stream);

        float64 videoPlayTime = 0.f;
        static bool isFFMGEGInited;
        bool isPlaying = false;
        bool isAudioVideoStreamsInited = false;

        FMOD_CREATESOUNDEXINFO exinfo;
        AV::AVFormatContext* movieContext = nullptr;

        Thread* audioDecodingThread = nullptr;
        Thread* videoDecodingThread = nullptr;
        Thread* videoPresentationThread = nullptr;
        Thread* readingDataThread = nullptr;

        float64 lastUpdateTime = 0.f;

        const uint8 emptyPixelColor = 255;

        Texture * videoTexture = nullptr;
        float64 videoFramerate = 0.f;
        float64 lastDecodedVideoPTS = 0.f;
        float64 frame_last_delay = 40e-3;
        float64 lastDecodedAudioPTS = -1.f;
        float64 video_clock = 0.f;

        uint32 textureWidth = 0;
        uint32 textureHeight = 0;
        uint32 frameHeight = 0;
        uint32 frameWidth = 0;
        const AV::AVPixelFormat avPixelFormat = AV::AV_PIX_FMT_RGBA;
        const PixelFormat textureFormat = PixelFormat::FORMAT_RGBA8888;
        uint32 textureBufferSize = 0;

        uint32_t len = 0;
        int32 index = 0;
        int64_t in_channel_layout = -1;
        struct SwrContext* videoConvertContext = nullptr;

        // AV::AVPacket* packet = nullptr;
        unsigned int videoStreamIndex = -1;
        AV::AVCodecContext* videoCodecContext = nullptr;
        AV::AVCodec* videoCodec = nullptr;
        AV::AVFrame * decodedFrame = nullptr;
        AV::AVFrame* rgbDecodedScaledFrame = nullptr;
        uint8 * out_buffer = nullptr;
        AV::SwsContext * img_convert_ctx = nullptr;

        const uint32 maxAudioFrameSize = 192000; // 1 second of 48khz 32bit audio
        unsigned int audioStreamIndex = -1;
        AV::AVCodecContext* audioCodecContext = nullptr;
        AV::AVCodec* audioCodec = nullptr;
        AV::AVFrame* audioFrame = nullptr;
        AV::SwrContext* audioConvertContext = nullptr;
        uint32 outAudioBufferSize = 0;
        FMOD::Channel* fmodChannel = nullptr;

        Deque<AV::AVPacket*> audioPackets;
        Mutex audioPacketsMutex;

        Mutex pcmMutex;
        DynamicMemoryFile* pcmData = nullptr;
        uint32 writePos = 0;
        uint32 readPos = 0;

        Deque<AV::AVPacket*> videoPackets;
        Mutex videoPacketsMutex;

        Deque<DecodedFrameBuffer*> decodedFrames;
        Mutex decodedFramesMutex;

        void EnqueueDecodedVideoBuffer(DecodedFrameBuffer* buf);
        DecodedFrameBuffer* DequeueDecodedVideoBuffer();

        void SotrPacketsByVideoAndAudio(AV::AVPacket* packet);

        struct DecodedPCMData
        {
            ~DecodedPCMData()
            {
                SafeDeleteArray(data);
            }

            float64 pts = 0.f;
            uint8* data;
            uint32 size = 0;
            uint32 written = 0;
        };

        DecodedPCMData* DecodeAudioInPlace();

        Deque<DecodedPCMData*> decodedAudio;
        Mutex decodedAudioMutex;

        DecodedPCMData* lastPcmData = nullptr;
        bool decodeAudioOnCallback = true;
        void FillBufferByPcmData(uint8* data, uint32 datalen, bool decodeInPlace);
        DecodedPCMData* DequePCMAudio();

        uint32 playTime = 0;
        float64 frameTimer = 0.f;
        void StartPlayingTimer();
        float64 GetPlayTime();
        void ShiftPlayTime(float64 delta);
        float64 GetAudioClock();

        uint32 audio_buf_size = 0;

        float64 audio_clock = 0.f;

        bool hasMoreData = false;

        float64 GetTime();
    };

    inline float64 MovieViewControl::GetTime()
    {
        return (AV::av_gettime() / 1000000.0);
    }
    inline void MovieViewControl::StartPlayingTimer()
    {
        frameTimer = GetTime();
    }

    inline float64 MovieViewControl::GetPlayTime()
    {
        return frameTimer - SystemTimer::Instance()->AbsoluteMS() / 1000.0;
    }

    inline void MovieViewControl::ShiftPlayTime(float64 delta)
    {
        frameTimer += delta;
    }

    // Pause/resume the playback.
    inline void MovieViewControl::Pause()
    {
        if (!isAudioVideoStreamsInited)
            return;

        isPlaying = false;
        fmodChannel->setPaused(!isPlaying);
    }

    inline void MovieViewControl::Resume()
    {
        if (!isAudioVideoStreamsInited)
            return;

        isPlaying = true;
        fmodChannel->setPaused(!isPlaying);
    }

    // Whether the movie is being played?
    inline bool MovieViewControl::IsPlaying() const
    {
        return isPlaying;
    }
}



#endif