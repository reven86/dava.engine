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

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "UI/IMovieViewControl.h"
#include "UI/UIControl.h"
#include "Render\PixelFormatDescriptor.h"
#include "Sound/SoundSystem.h"
#include "FileSystem/StreamBuffer.h"
#include "Sound/FMODUtils.h"
#include "fmod.h"
#include "Concurrency/ConditionVariable.h"

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

#include <atomic>

namespace DAVA
{
struct DecodedFrameBuffer
{
    DecodedFrameBuffer(uint32 dataSize, PixelFormat format, float64 pts_)
        : pts(pts_)
        , textureFormat(format)
    {
        data = new uint8[dataSize];

        // we fill codecContext->width x codecContext->height area, it is smaller than texture size. So fill all the texture by empty color once.
        // we suppose that next time we will fill same part of the texture.
        Memset(data, emptyPixelColor, dataSize);
    }
    ~DecodedFrameBuffer()
    {
        SafeDeleteArray(data);
    }

    const uint8 emptyPixelColor = 255;

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
    enum PlayState : uint32
    {
        PLAYING = 0,
        PREFETCHING,
        PAUSED,
        STOPPED,
    };

    PlayState state = STOPPED;

    const float64 AV_SYNC_THRESHOLD = 0.01;
    const float64 AV_NOSYNC_THRESHOLD = 0.5;

    AV::AVFormatContext* CreateContext(const FilePath& path);

    static FMOD_RESULT F_CALLBACK PcmReadDecodeCallback(FMOD_SOUND* sound, void* data, unsigned int datalen);

    float64 GetMasterClock();
    bool InitVideo();
    DecodedFrameBuffer* DecodeVideoPacket(AV::AVPacket* packet);
    void UpdateVideo(DecodedFrameBuffer* frameBuffer);
    bool InitAudio();
    void DecodeAudio(AV::AVPacket* packet, float64 timeElapsed);

    void VideoDecodingThread(BaseObject* caller, void* callerData, void* userData);
    void AudioDecodingThread(BaseObject* caller, void* callerData, void* userData);
    void ReadingThread(BaseObject* caller, void* callerData, void* userData);
    void SortPacketsByVideoAndAudio(AV::AVPacket* packet);

    FilePath moviePath;

    const uint32 maxAudioPacketsPrefetchedCount = 100;
    std::atomic<uint32> currentPrefetchedPacketsCount = 0;
    void PrefetchData(uint32 dataSize);
    ConditionVariable prefetchCV;

    float64 SyncVideoClock(AV::AVFrame* src_frame, float64 pts);
    float64 GetPTSForFrame(AV::AVFrame* frame, AV::AVPacket* packet, uint32 stream);

    bool isAudioVideoStreamsInited = false;

    FMOD_CREATESOUNDEXINFO exinfo;
    AV::AVFormatContext* movieContext = nullptr;

    Thread* audioDecodingThread = nullptr;
    Thread* videoDecodingThread = nullptr;
    Thread* readingDataThread = nullptr;

    Texture* videoTexture = nullptr;
    float64 videoFramerate = 0.f;
    float64 frameLastPts = 0.f;
    float64 frameLastDelay = 40e-3;
    float64 video_clock = 0.f;

    uint32 textureWidth = 0;
    uint32 textureHeight = 0;
    uint32 frameHeight = 0;
    uint32 frameWidth = 0;
    const AV::AVPixelFormat avPixelFormat = AV::AV_PIX_FMT_RGBA;
    const PixelFormat textureFormat = PixelFormat::FORMAT_RGBA8888;
    uint32 textureBufferSize = 0;

    unsigned int videoStreamIndex = -1;
    AV::AVCodecContext* videoCodecContext = nullptr;
    AV::AVFrame* rgbDecodedScaledFrame = nullptr;

    const uint32 maxAudioFrameSize = 192000; // 1 second of 48khz 32bit audio
    unsigned int audioStreamIndex = -1;
    AV::AVCodecContext* audioCodecContext = nullptr;

    AV::SwrContext* audioConvertContext = nullptr;
    uint32 outAudioBufferSize = 0;

    static bool isFFMGEGInited;
    int out_channels = -1;
    const int out_sample_rate = 44100;
    FMOD::Sound* sound = nullptr;
    FMOD::Channel* fmodChannel = nullptr;
    StreamBuffer pcmBuffer;
    void InitFmod();

    Deque<AV::AVPacket*> audioPackets;
    Mutex audioPacketsMutex;

    Deque<AV::AVPacket*> videoPackets;
    Mutex videoPacketsMutex;

    uint32 playTime = 0;
    float64 frameTimer = 0.f;
    float64 GetAudioClock();

    uint32 audio_buf_size = 0;
    std::atomic<float64> audio_clock = 0.f;

    bool eof = false;

    float64 GetTime();

    void FlushBuffers();
    void CloseMovie();
};
}

#endif

#endif