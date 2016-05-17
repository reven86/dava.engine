#pragma once

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "UI/IMovieViewControl.h"
#include "UI/UIControl.h"
#include "Render/PixelFormatDescriptor.h"
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
class FfmpegPlayer : public IMovieViewControl, public StreamDelegate
{
public:
    ~FfmpegPlayer() override;

    // IMovieViewControl Interface implementation

    // Initialize the control.
    void Initialize(const Rect& rect) override;

    // Open the Movie.
    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override;

    // Position/visibility.
    void SetRect(const Rect& rect) override{};
    void SetVisible(bool isVisible) override{};

    // Start/stop the video playback.
    void Play() override;
    void Stop() override;

    // Pause/resume the playback.
    void Pause() override;
    void Resume() override;

    // Whether the movie is being played?
    bool IsPlaying() const override;

    void Update();

    struct DrawVideoFrameData
    {
        Vector<uint8> data;
        uint32 frameHeight;
        uint32 frameWidth;
        PixelFormat format;
    };

    // Data getter
    DrawVideoFrameData GetDrawData();

    PixelFormat GetPixelFormat() const;
    Vector2 GetResolution() const;

    enum PlayState : uint32
    {
        PLAYING = 0,
        PREFETCHING,
        PAUSED,
        STOPPED,
    };

    PlayState GetState() const;

    // StreamDelegate
    void PcmDataCallback(uint8* data, uint32 datalen) override;

private:
    struct DecodedFrameBuffer
    {
        DecodedFrameBuffer(uint32 dataSize, PixelFormat format, float64 pts_)
            : pts(pts_)
            , textureFormat(format)
            , data(dataSize)
        {
            // we fill codecContext->width x codecContext->height area, it is smaller than texture size. So fill all the texture by empty color once.
            // we suppose that next time we will fill same part of the texture.
            Memset(data.data(), emptyPixelColor, dataSize);
        }

        const uint8 emptyPixelColor = 255;

        float64 frame_last_pts = 0.f;
        float64 pts = 0.f;
        float64 sleepAfterPresent = 0;
        PixelFormat textureFormat = FORMAT_INVALID;
        Vector<uint8> data;
        uint32 width = 0;
        uint32 height = 0;
    };

    PlayState state = STOPPED;
    bool videoShown = false;
    bool audioListen = false;

    const float64 AV_SYNC_THRESHOLD = 0.01;
    const float64 AV_NOSYNC_THRESHOLD = 0.5;

    AV::AVFormatContext* CreateContext(const FilePath& path);

    float64 GetMasterClock() const;
    bool InitVideo();
    DecodedFrameBuffer* DecodeVideoPacket(AV::AVPacket* packet);
    void UpdateVideo(DecodedFrameBuffer* frameBuffer);
    bool InitAudio();
    void DecodeAudio(AV::AVPacket* packet, float64 timeElapsed);

    void ClearBuffers();
    void CloseMovie();

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

    bool isVideoSubsystemInited = false;
    bool isAudioSubsystemInited = false;

    FMOD_CREATESOUNDEXINFO exinfo;
    AV::AVFormatContext* movieContext = nullptr;

    Thread* audioDecodingThread = nullptr;
    Thread* videoDecodingThread = nullptr;
    Thread* readingDataThread = nullptr;

    void UpdateDrawData(DecodedFrameBuffer* buffer);
    Mutex lastFrameLocker;
    DrawVideoFrameData lastFrameData;
    float64 videoFramerate = 0.f;
    float64 frameLastPts = 0.f;
    float64 frameLastDelay = 40e-3;
    float64 videoClock = 0.f;

    uint32 frameHeight = 0;
    uint32 frameWidth = 0;
    const AV::AVPixelFormat avPixelFormat = AV::AV_PIX_FMT_RGBA;
    const PixelFormat pixelFormat = PixelFormat::FORMAT_RGBA8888;
    uint32 frameBufferSize = 0;

    unsigned int videoStreamIndex = -1;
    AV::AVCodecContext* videoCodecContext = nullptr;
    AV::AVFrame* rgbDecodedScaledFrame = nullptr;

    const uint32 maxAudioFrameSize = 192000; // 1 second of 48khz 32bit audio
    unsigned int audioStreamIndex = -1;
    AV::AVCodecContext* audioCodecContext = nullptr;

    AV::SwrContext* audioConvertContext = nullptr;
    uint32 outAudioBufferSize = 0;

    int outChannels = -1;
    SoundStream* soundStream = nullptr;
    StreamBuffer pcmBuffer;

    Deque<AV::AVPacket*> audioPackets;
    Mutex audioPacketsMutex;

    Deque<AV::AVPacket*> videoPackets;
    Mutex videoPacketsMutex;

    uint32 playTime = 0;
    float64 frameTimer = 0.f;
    uint32 audioBufSize = 0;
    std::atomic<float64> audioClock = 0.f;
    float64 GetAudioClock() const;
    float64 GetTime();

    bool mediaFileEOF = false;
};
}

#endif