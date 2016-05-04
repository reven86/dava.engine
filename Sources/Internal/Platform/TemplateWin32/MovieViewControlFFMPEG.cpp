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

#include "MovieViewControlFFMPEG.h"
#include "Sound/SoundSystem.h"

namespace DAVA
{
    bool MovieViewControl::isFFMGEGInited = false;

    MovieViewControl::~MovieViewControl()
    {
        Stop();

        if (readingDataThread)
        {
            readingDataThread->Cancel();
            readingDataThread->Join();
            SafeRelease(readingDataThread);
        }

        if (videoDecodingThread)
        {
            videoDecodingThread->Cancel();
            videoDecodingThread->Join();
            SafeRelease(videoDecodingThread);
        }
        if (audioDecodingThread)
        {
            audioDecodingThread->Cancel();
            audioDecodingThread->Join();
            SafeRelease(audioDecodingThread);
        }

        if (isFFMGEGInited)
        {
            if (rgbDecodedScaledFrame)
            {
                av_frame_free(&rgbDecodedScaledFrame);
            }

            if (videoCodecContext)
            {
                avcodec_close(videoCodecContext);
            }
            if (movieContext)
            {
                avformat_close_input(&movieContext);
            }

            isFFMGEGInited = false;
        }

    }

    // Initialize the control.
    void MovieViewControl::Initialize(const Rect& rect)
    {
        SetRect(rect);
        if (!isFFMGEGInited)
        {
            AV::av_register_all();
            isFFMGEGInited = true;
        }
        
        GetBackground()->SetBgDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    }

    AV::AVFormatContext* MovieViewControl::CreateContext(const FilePath& path)
    {
        AV::AVFormatContext* context = AV::avformat_alloc_context();
        bool isSuccess = true;

        if (AV::avformat_open_input(&context, path.GetAbsolutePathname().c_str(), nullptr, nullptr) != 0)
        {
            Logger::Error("Couldn't open input stream.\n");
            isSuccess = false;
        }

        if (AV::avformat_find_stream_info(context, nullptr) < 0)
        {
            Logger::Error("Couldn't find stream information.\n");
            isSuccess = false;
        }

        if (!isSuccess)
        {
            avformat_close_input(&context);
        }
        return context;
    }

    // Start/stop the video playback.
    void MovieViewControl::Play()
    {
        if (!isAudioVideoStreamsInited || isPlaying)
            return;

        isPlaying = true;

        if (fmodChannel)
            fmodChannel->setPaused(false);
        frameTimer = GetTime();
    }

    // Open the Movie.
    void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
    {
        movieContext = CreateContext(moviePath);
        if (nullptr == movieContext)
        {
            Logger::Error("Can't Open video.");
        }

        for (unsigned int i = 0; i < movieContext->nb_streams; i++)
        {
            if (movieContext->streams[i]->codec->codec_type == AV::AVMEDIA_TYPE_AUDIO)
            {
                audioStreamIndex = i;
                break;
            }
        }
        if (audioStreamIndex == -1)
        {
            Logger::Error("Didn't find an audio stream.\n");
            return; // false;
        }

        for (unsigned int i = 0; i < movieContext->nb_streams; i++)
        {
            if (movieContext->streams[i]->codec->codec_type == AV::AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex = i;
                break;
            }
        }
        if (videoStreamIndex == -1)
        {
            Logger::Error("Didn't find a video stream.\n");
            return; // false;
        }

        isAudioVideoStreamsInited = false;

        bool isVideoSubsystemInited = InitVideo();
        if (!isVideoSubsystemInited)
        {
            Logger::Error("Can't init video decoder.");
        }

        bool isAudioSubsystemInited = InitAudio();
        if (!isAudioSubsystemInited)
        {
            Logger::Error("Can't init audio decoder.");
        }

        // read some packets to fill audio buffers before init fmod
        PrefetchData(maxAudioPacketsPrefetchedCount);
        while (currentPrefetchedPacketsCount > 0)
        {
            Thread::Sleep(0);
        }

        readingDataThread = Thread::Create(Message(this, &MovieViewControl::ReadingThread));
        readingDataThread->Start();

        InitFmod();

        isAudioVideoStreamsInited = true;
    }

    bool MovieViewControl::InitVideo()
    {
        AV::AVRational avfps = movieContext->streams[videoStreamIndex]->avg_frame_rate;
        videoFramerate = avfps.num / static_cast<float64>(avfps.den);
        Renderer::SetDesiredFPS(60);

        videoCodecContext = movieContext->streams[videoStreamIndex]->codec;
        AV::AVCodec* videoCodec = AV::avcodec_find_decoder(videoCodecContext->codec_id);
        if (videoCodec == nullptr)
        {
            Logger::Error("Codec not found.\n");
            return false;
        }
        if (AV::avcodec_open2(videoCodecContext, videoCodec, nullptr) < 0)
        {
            Logger::Error("Could not open codec.\n");
            return false;
        }

        textureWidth = NextPowerOf2(videoCodecContext->width);
        textureHeight = NextPowerOf2(videoCodecContext->height);
        textureBufferSize = textureWidth * textureHeight * PixelFormatDescriptor::GetPixelFormatSizeInBytes(textureFormat);
        frameHeight = videoCodecContext->height;
        frameWidth = videoCodecContext->width;

        if (videoDecodingThread)
        {
            videoDecodingThread->Cancel();
            videoDecodingThread->Join();
            SafeRelease(videoDecodingThread);
        }
        videoDecodingThread = Thread::Create(Message(this, &MovieViewControl::VideoDecodingThread));
        videoDecodingThread->Start();

        return true;
    }

    void MovieViewControl::SortPacketsByVideoAndAudio(AV::AVPacket* packet)
    {
        DVASSERT(packet != nullptr);
        if (packet->stream_index == videoStreamIndex)
        {
            LockGuard<Mutex> locker(videoPacketsMutex);
            videoPackets.push_back(packet);
        }
        else
        if (packet->stream_index == audioStreamIndex)
        {
            /*DecodeAudio(packet, 0);*/
            LockGuard<Mutex> locker(audioPacketsMutex);
            currentPrefetchedPacketsCount++;
            audioPackets.push_back(packet);
        }
        else
        {
            AV::av_packet_unref(packet);
        }
    }

    float64 MovieViewControl::GetAudioClock()
    {
        audioPacketsMutex.Lock();
        float64 pts = audio_clock; /* maintained in the audio thread */
        audioPacketsMutex.Unlock();
        uint32 hw_buf_size = audio_buf_size;
        uint32 bytes_per_sec = 0;
        uint32 n = audioCodecContext->channels * 2;
        if (movieContext->streams[audioStreamIndex])
        {
            bytes_per_sec = audioCodecContext->sample_rate * n;
        }
        if (bytes_per_sec)
        {
            pts -= static_cast<float64>(hw_buf_size) / bytes_per_sec;
        }
        return pts;
    }

    void MovieViewControl::FlushBuffers()
    {
        {
            LockGuard<Mutex> audioLock(audioPacketsMutex);
            for (auto packet : audioPackets)
            {
                AV::av_packet_unref(packet);
            }
            audioPackets.clear();
        }
        {
            LockGuard<Mutex> videoLock(videoPacketsMutex);
            for (auto packet : videoPackets)
            {
                AV::av_packet_unref(packet);
            }
            videoPackets.clear();
        }
        pcmBuffer.Flush();
    }

    FMOD_RESULT F_CALLBACK MovieViewControl::PcmReadDecodeCallback(FMOD_SOUND* sound, void* data, unsigned int datalen)
    {
        Memset(data, 0, datalen);

        // Read from your buffer here...
        void* soundData;
        reinterpret_cast<FMOD::Sound*>(sound)->getUserData(&soundData);

        MovieViewControl* movieControl = reinterpret_cast<MovieViewControl*>(soundData);
        if (nullptr != movieControl)
        {
            movieControl->pcmBuffer.Read(static_cast<uint8*>(data), datalen);
        }

        return FMOD_OK;
    }


    bool MovieViewControl::InitAudio()
    {

        // Get a pointer to the codec context for the audio stream
        audioCodecContext = movieContext->streams[audioStreamIndex]->codec;

        // Find the decoder for the audio stream
        AV::AVCodec* audioCodec = avcodec_find_decoder(audioCodecContext->codec_id);
        if (audioCodec == nullptr)
        {
            printf("Codec not found.\n");
            //return false;
        }

        // Open codec
        if (avcodec_open2(audioCodecContext, audioCodec, nullptr) < 0)
        {
            printf("Could not open codec.\n");
            // return false;
        }

        //Out Audio Param
        uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
        //nb_samples: AAC-1024 MP3-1152
        int out_nb_samples = audioCodecContext->frame_size;
        AV::AVSampleFormat out_sample_fmt = AV::AV_SAMPLE_FMT_S16;

        audio_buf_size = out_sample_rate;
        out_channels = AV::av_get_channel_layout_nb_channels(out_channel_layout);
        //Out Buffer Size
        outAudioBufferSize = static_cast<uint32>(av_samples_get_buffer_size(nullptr, out_channels, out_nb_samples, out_sample_fmt, 1));

        //FIX:Some Codec's Context Information is missing
        int64_t in_channel_layout = AV::av_get_default_channel_layout(audioCodecContext->channels);

        audioConvertContext = AV::swr_alloc_set_opts(audioConvertContext, out_channel_layout, out_sample_fmt, out_sample_rate, in_channel_layout, audioCodecContext->sample_fmt, audioCodecContext->sample_rate, 0, nullptr);
        AV::swr_init(audioConvertContext);


        if (audioDecodingThread)
        {
            audioDecodingThread->Cancel();
            audioDecodingThread->Join();
            SafeRelease(audioDecodingThread);
        }

        audioDecodingThread = Thread::Create(Message(this, &MovieViewControl::AudioDecodingThread));
        audioDecodingThread->Start();

        return true;
    }

    float64 MovieViewControl::GetMasterClock()
    {
        return GetAudioClock();
    }

    float64 MovieViewControl::synchronize_video(AV::AVFrame* src_frame, float64 pts)
    {
        float64 frame_delay;

        if (pts != 0)
        {
            /* if we have pts, set video clock to it */
            video_clock = pts;
        }
        else
        {
            /* if we aren't given a pts, set it to the clock */
            pts = video_clock;
        }

        /* update the video clock */
        frame_delay = AV::av_q2d(videoCodecContext->time_base);
        /* if we are repeating a frame, adjust clock accordingly */
        frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
        video_clock += frame_delay;

        return pts;
    }

    DecodedFrameBuffer* MovieViewControl::DecodeVideoPacket(AV::AVPacket* packet)
    {
        int32 got_picture;
        AV::AVFrame* decodedFrame = AV::av_frame_alloc();
        int32 ret = AV::avcodec_decode_video2(videoCodecContext, decodedFrame, &got_picture, packet);

        DecodedFrameBuffer* frameBuffer = nullptr;
        if (ret >= 0 && got_picture)
        {
            frameBuffer = new DecodedFrameBuffer();
            frameBuffer->data = new uint8[textureBufferSize];
            // a trick to get converted data into one buffer with textureBufferSize because it could be larger than frame frame size.
            uint8* rgbTextureBufferHolder[1];
            rgbTextureBufferHolder[0] = frameBuffer->data;
            // we fill codecContext->width x codecContext->height area, it is smaller than texture size. So fill all the texture by empty color once.
            // we suppose that next time we will fill same part of the texture.
            Memset(frameBuffer->data, emptyPixelColor, textureBufferSize);

            // rgbTextureBufferHolder is a pointer to pointer to uint8. Used to obtain data directly to our rgbTextureBuffer
            AV::SwsContext* img_convert_ctx = AV::sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, videoCodecContext->width, videoCodecContext->height, avPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

            rgbDecodedScaledFrame = AV::av_frame_alloc();
            uint8* out_buffer = nullptr;
            out_buffer = (uint8_t*)AV::av_mallocz(AV::av_image_get_buffer_size(avPixelFormat, videoCodecContext->width, videoCodecContext->height, PixelFormatDescriptor::GetPixelFormatSizeInBits(textureFormat)));
            AV::avpicture_fill((AV::AVPicture*)rgbDecodedScaledFrame, out_buffer, avPixelFormat, videoCodecContext->width, videoCodecContext->height);
            AV::av_free(out_buffer);

            const uint32 scaledHeight = AV::sws_scale(img_convert_ctx, decodedFrame->data, decodedFrame->linesize, 0, frameHeight, rgbTextureBufferHolder, rgbDecodedScaledFrame->linesize);

            sws_freeContext(img_convert_ctx);

            frameBuffer->textureFormat = textureFormat;

            float64 effectivePTS = GetPTSForFrame(decodedFrame, packet, videoStreamIndex);
            effectivePTS = synchronize_video(decodedFrame, effectivePTS);
            frame_last_pts = effectivePTS;
            frameBuffer->pts = effectivePTS;
        }

        av_frame_free(&decodedFrame);

        return frameBuffer;
    }

    void MovieViewControl::UpdateVideo(DecodedFrameBuffer* frameBuffer)
    {
        if (nullptr == frameBuffer)
        {
            Thread::Sleep(1);
            return;
        }

        float64 delay = frameBuffer->pts - frame_last_pts; /* the pts from last time */
        if (delay <= 0.0 || delay >= 1.0)
        {
            /* if incorrect delay, use previous one */
            delay = frame_last_delay;
        }
        /* save for next time */
        frame_last_delay = delay;
        frame_last_pts = frameBuffer->pts;

        /* update delay to sync to audio */
        float64 ref_clock = GetMasterClock();

        float64 diff = frameBuffer->pts - ref_clock;
        Logger::Error("RefClock %f pts %f diff %f", ref_clock, frameBuffer->pts, diff);

        /* Skip or repeat the frame. Take delay into account
        FFPlay still doesn't "know if this is the best guess." */

        const float64 AV_SYNC_THRESHOLD = 0.01;
        const float64 AV_NOSYNC_THRESHOLD = 0.5;

        float64 sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;
        if (fabs(diff) < AV_NOSYNC_THRESHOLD)
        {
            if (diff <= -sync_threshold)
            {
                delay = 0;
            }
            else if (diff >= sync_threshold)
            {
                delay = 2 * delay;
            }
        }
        frameTimer += delay;

        /* computer the REAL delay */
        float64 actual_delay = frameTimer - GetTime();
        if (actual_delay < 0.010)
        {
            /* Really it should skip the picture instead */
            actual_delay = 0.001;
        }

        float64 timeBeforePresent = GetTime();

        if (nullptr == videoTexture)
        {
            // rgbTextureBuffer is a rgbTextureBufferHolder[0]
            videoTexture = Texture::CreateFromData(frameBuffer->textureFormat, frameBuffer->data, textureWidth, textureHeight, false);
            Sprite* videoSprite = Sprite::CreateFromTexture(videoTexture, 0, 0, static_cast<int32>(frameWidth), static_cast<int32>(frameHeight), static_cast<float32>(frameWidth), static_cast<float32>(frameHeight));
            auto back = GetBackground();
            if (back)
            {
                back->SetSprite(videoSprite, 0);
            }
            SafeRelease(videoSprite);
        }
        else
        {
            // rgbTextureBuffer is a rgbTextureBufferHolder[0]
            videoTexture->TexImage(0, textureWidth, textureHeight, frameBuffer->data, textureBufferSize, Texture::INVALID_CUBEMAP_FACE);
        }
        SafeDelete(frameBuffer);

        uint32 sleepLessFor = static_cast<uint32>(GetTime() - timeBeforePresent);

        uint32 sleepTime = static_cast<uint32>(actual_delay * 1000 + 0.5) - sleepLessFor;
        Logger::Error("Frame SLEEP = %d", sleepTime);
        Thread::Sleep(sleepTime);
    }

    float64 MovieViewControl::GetPTSForFrame(AV::AVFrame* frame, AV::AVPacket* packet, uint32 stream)
    {
        int64 pts;
        if (packet->dts != AV_NOPTS_VALUE)
        {
            pts = AV::av_frame_get_best_effort_timestamp(frame);
        }
        else
        {
            pts = 0;
        }

        float64 effectivePTS = static_cast<float64>(pts) * AV::av_q2d(videoCodecContext->time_base);
        return effectivePTS;
    }

    void MovieViewControl::InitFmod()
    {
        FMOD_CREATESOUNDEXINFO exinfo;
        memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO); /* required. */
        exinfo.decodebuffersize = out_sample_rate; /* Chunk size of stream update in samples.  This will be the amount of data passed to the user callback. */
        exinfo.length = out_sample_rate * out_channels * sizeof(signed short) * 5; /* Length of PCM data in bytes of whole song (for Sound::getLength) */
        exinfo.numchannels = out_channels; /* Number of channels in the sound. */
        exinfo.defaultfrequency = out_sample_rate; /* Default playback rate of sound. */
        exinfo.format = FMOD_SOUND_FORMAT_PCM16; /* Data format of sound. */
        exinfo.pcmreadcallback = &MovieViewControl::PcmReadDecodeCallback; /* User callback for reading. */

        FMOD::Sound* sound = nullptr;
        FMOD::System* system = SoundSystem::Instance()->GetFmodSystem();
        FMOD_RESULT result = system->createStream(nullptr, FMOD_OPENUSER, &exinfo, &sound);
        sound->setUserData(this);

        system->playSound(FMOD_CHANNEL_FREE, sound, !isPlaying, &fmodChannel);
        fmodChannel->setLoopCount(-1);
        fmodChannel->setMode(FMOD_LOOP_NORMAL | FMOD_NONBLOCKING);
        fmodChannel->setPosition(0, FMOD_TIMEUNIT_MS); // this flushes the buffer to ensure the loop mode takes effect
    }

    void MovieViewControl::DecodeAudio(AV::AVPacket* packet, float64 timeElapsed)
    {
        DVASSERT(packet->stream_index == audioStreamIndex);
        int got_data;
        AV::AVFrame* audioFrame = AV::av_frame_alloc();
        int ret = avcodec_decode_audio4(audioCodecContext, audioFrame, &got_data, packet);
        if (ret < 0)
        {
            printf("Error in decoding audio frame.\n");
            return;
        }

        uint8* outAudioBuffer = nullptr;
        if (got_data > 0)
        {
            outAudioBuffer = outAudioBuffer = new uint8[maxAudioFrameSize * 2];
            AV::swr_convert(audioConvertContext, &outAudioBuffer, maxAudioFrameSize, (const uint8_t**)audioFrame->data, audioFrame->nb_samples);
        }
        else
        {
            Logger::Error("Convert audio NO DATA");
            return;
        }

        int64_t pts;
        if (packet->dts != AV_NOPTS_VALUE)
        {
            pts = packet->pts;
        }
        else
        {
            pts = 0;
        }
        if (pts != AV_NOPTS_VALUE)
        {
            audio_clock = AV::av_q2d(audioCodecContext->time_base) * pts;
        }

        pcmBuffer.Write(outAudioBuffer, outAudioBufferSize);
    }

    void MovieViewControl::AudioDecodingThread(BaseObject* caller, void* callerData, void* userData)
    {
        Thread* thread = static_cast<Thread*>(caller);

        //return;
        AV::AVPacket* audioPacket = nullptr;
        do
        {
            if (!isPlaying)
            {
                Thread::Sleep(0);
            }
            audioPacketsMutex.Lock();
            auto size = audioPackets.size();
            audioPacketsMutex.Unlock();

            if (size > 0)
            {
                audioPacketsMutex.Lock();
                audioPacket = audioPackets.front();
                audioPackets.pop_front();
                currentPrefetchedPacketsCount--;
                audioPacketsMutex.Unlock();

                DecodeAudio(audioPacket, 0);

                AV::av_packet_unref(audioPacket);
            }
            else
            {
                Thread::Sleep(1);
            }

        } while (thread && !thread->IsCancelling());
    }

    void MovieViewControl::VideoDecodingThread(BaseObject* caller, void* callerData, void* userData)
    {
        Thread* thread = static_cast<Thread*>(caller);

        AV::AVPacket* videoPacket = nullptr;
        do
        {
            if (!isPlaying)
            {
                Thread::Sleep(0);
                continue;
            }

            videoPacketsMutex.Lock();
            auto size = videoPackets.size();
            videoPacketsMutex.Unlock();

            if (size > 0)
            {
                videoPacketsMutex.Lock();
                videoPacket = videoPackets.front();
                videoPackets.pop_front();
                videoPacketsMutex.Unlock();

                auto frameBuffer = DecodeVideoPacket(videoPacket);
                AV::av_packet_unref(videoPacket);

                UpdateVideo(frameBuffer);
            }
            else
            {
                Thread::Sleep(0);
            }

        } while (thread && !thread->IsCancelling());
    }

    void MovieViewControl::PrefetchData(uint32 dataSize)
    {
        int retRead = 0;
        do
        {
            AV::AVPacket* packet = static_cast<AV::AVPacket*>(AV::av_mallocz(sizeof(AV::AVPacket)));
            av_init_packet(packet);
            retRead = AV::av_read_frame(movieContext, packet);

            eof = retRead < 0;

            if (eof)
            {
                AV::av_packet_unref(packet);
            }
            else
            {
                SortPacketsByVideoAndAudio(packet);
            }
        } while (retRead >= 0 && currentPrefetchedPacketsCount < dataSize);
    }

    void MovieViewControl::ReadingThread(BaseObject* caller, void* callerData, void* userData)
    {
        Thread* thread = static_cast<Thread*>(caller);

        do
        {
            if (!isPlaying)
            {
                Thread::Sleep(0);
                continue;
            }

            if (currentPrefetchedPacketsCount >= maxAudioPacketsPrefetchedCount - 30)
            {
                Thread::Sleep(0);
                continue;
            }

            PrefetchData(maxAudioPacketsPrefetchedCount - currentPrefetchedPacketsCount);

        } while (thread && !thread->IsCancelling());
    }

    // UIControl update and draw implementation
    void MovieViewControl::Update(float32 timeElapsed)
    {
        if (!isAudioVideoStreamsInited)
            return;
    }

    void MovieViewControl::Stop()
    {
        if (!isAudioVideoStreamsInited)
            return;

        Pause();

        fmodChannel->setPosition(0, FMOD_TIMEUNIT_MS);
        AV::av_seek_frame(movieContext, audioStreamIndex, 0, AVSEEK_FLAG_FRAME);
        AV::av_seek_frame(movieContext, videoStreamIndex, 0, AVSEEK_FLAG_FRAME);

        FlushBuffers();

        audio_clock = 0;
        frame_last_pts = 0.f;
        frame_last_delay = 40e-3;
        video_clock = 0.f;
        eof = false;
    }

}