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

        if (isFFMGEGInited)
        {
            if (rgbDecodedScaledFrame)
            {
                av_frame_free(&rgbDecodedScaledFrame);
            }
            if (decodedFrame)
            {
                av_frame_free(&decodedFrame);
            }
            if (videoCodecContext)
            {
                avcodec_close(videoCodecContext);
            }
            if (movieContext)
            {
                avformat_close_input(&movieContext);
            }
            if (img_convert_ctx)
            {
                sws_freeContext(img_convert_ctx);
            }

            pcmMutex.Lock();
            SafeRelease(pcmData);
            pcmMutex.Unlock();

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

    // Position/visibility.
    void MovieViewControl::SetRect(const Rect& rect)
    {
        UIControl::SetRect(rect);
    }

    void MovieViewControl::SetVisible(bool isVisible)
    {
        UIControl::SetVisibilityFlag(isVisible);
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

        readingDataThread = Thread::Create(Message(this, &MovieViewControl::ReadingThread));
        readingDataThread->Start();
    }

    // Open the Movie.
    void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
    {
        isAudioVideoStreamsInited = false;

        movieContext = CreateContext(moviePath);
        if (nullptr == movieContext)
        {
            Logger::Error("Can't Open video.");
        }

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

        isAudioVideoStreamsInited = isVideoSubsystemInited && isAudioSubsystemInited;
    }

    bool MovieViewControl::InitVideo()
    {
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
            return false;
        }

        AV::AVRational avfps = movieContext->streams[videoStreamIndex]->avg_frame_rate;
        videoFramerate = avfps.num / static_cast<float64>(avfps.den);
        Renderer::SetDesiredFPS(30);

        videoCodecContext = movieContext->streams[videoStreamIndex]->codec;
        videoCodec = AV::avcodec_find_decoder(videoCodecContext->codec_id);
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

        decodedFrame = AV::av_frame_alloc();
        rgbDecodedScaledFrame = AV::av_frame_alloc();
        out_buffer = (uint8_t*)AV::av_mallocz(AV::av_image_get_buffer_size(avPixelFormat, videoCodecContext->width, videoCodecContext->height, PixelFormatDescriptor::GetPixelFormatSizeInBits(textureFormat)));
        AV::avpicture_fill((AV::AVPicture*)rgbDecodedScaledFrame, out_buffer, avPixelFormat, videoCodecContext->width, videoCodecContext->height);

        img_convert_ctx = AV::sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, videoCodecContext->width, videoCodecContext->height, avPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

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

        if (videoPresentationThread)
        {
            videoPresentationThread->Cancel();
            videoPresentationThread->Join();
            SafeRelease(videoPresentationThread);
        }
        videoPresentationThread = Thread::Create(Message(this, &MovieViewControl::VideoPresentationThread));
        videoPresentationThread->Start();

        return true;
    }

    void MovieViewControl::EnqueueDecodedVideoBuffer(MovieViewControl::DecodedFrameBuffer* buf)
    {
        uint32 framesInBuffer = 0;

        do
        {
            decodedFramesMutex.Lock();
            framesInBuffer = decodedFrames.size();
            decodedFramesMutex.Unlock();

            if (framesInBuffer > 1)
            {
                Thread::Sleep(1);
            }

        } while (framesInBuffer > 1);

        decodedFramesMutex.Lock();
        decodedFrames.push_back(buf);
        decodedFramesMutex.Unlock();
    }

    MovieViewControl::DecodedFrameBuffer* MovieViewControl::DequeueDecodedVideoBuffer()
    {
        decodedFramesMutex.Lock();
        uint32 size = decodedFrames.size();
        decodedFramesMutex.Unlock();

        if (size == 0)
        {
            return nullptr;
        }

        decodedFramesMutex.Lock();
        DecodedFrameBuffer* frameBuffer = decodedFrames.front();
        decodedFrames.pop_front();
        decodedFramesMutex.Unlock();

        return frameBuffer;
    }

    void MovieViewControl::SotrPacketsByVideoAndAudio(AV::AVPacket* packet)
    {
        //while (videoPackets.size() > 3)
        //            Thread::Sleep(1);
        //while (audioPackets.size() > 20)
        //  Thread::Sleep(1);

        if (packet->stream_index == videoStreamIndex)
        {
            LockGuard<Mutex> locker(videoPacketsMutex);
            videoPackets.push_back(packet);
        }
        else
        if (packet->stream_index == audioStreamIndex)
        {
            LockGuard<Mutex> locker(audioPacketsMutex);
            audioPackets.push_back(packet);
        }
        else
        {
            AV::av_packet_unref(packet);
        }
    }

    MovieViewControl::DecodedPCMData* MovieViewControl::DequePCMAudio()
    {
        decodedAudioMutex.Lock();
        uint32 size = decodedAudio.size();
        decodedAudioMutex.Unlock();

        if (size == 0)
        {
            return nullptr;
        }

        decodedAudioMutex.Lock();
        DecodedPCMData* data = decodedAudio.front();
        decodedAudio.pop_front();
        decodedAudioMutex.Unlock();

        return data;
    }

    float64 MovieViewControl::GetAudioClock()
    {
        float64 pts;
        int32 hw_buf_size, bytes_per_sec, n;

        pts = audio_clock; /* maintained in the audio thread */
        hw_buf_size = audioPackets.size();
        bytes_per_sec = 0;
        n = audioCodecContext->channels * 2;
        if (movieContext->streams[audioStreamIndex])
        {
            bytes_per_sec = audioCodecContext->sample_rate * n;
        }
        if (bytes_per_sec)
        {
            pts -= (double)hw_buf_size / bytes_per_sec;
        }
        return pts;
    }

    FMOD_RESULT F_CALLBACK MovieViewControl::pcmreadcallback(FMOD_SOUND* sound, void* data, unsigned int datalen)
    {
        // Read from your buffer here...
        void* soundData;
        reinterpret_cast<FMOD::Sound*>(sound)->getUserData(&soundData);

        MovieViewControl* movieControl = reinterpret_cast<MovieViewControl*>(soundData);

        if (nullptr == movieControl)
            return FMOD_ERR_MUSIC_UNINITIALIZED;

        Memset(data, 0, datalen);

        static DecodedPCMData* lastPcmData = nullptr;

        uint32 fmodDataWritten = 0;

        // Write the data remains from previous call
        if (nullptr != lastPcmData)
        {
            if (lastPcmData->size - lastPcmData->written > 0)
            {
                // |     fmodDataWritten  |     size    |
                // [.....written data.....][data to write]
                uint32 dataSizeToWrite = lastPcmData->size - lastPcmData->written;
                Memcpy(data, lastPcmData->data + lastPcmData->written, dataSizeToWrite);
                fmodDataWritten += dataSizeToWrite;
            }
            SafeDelete(lastPcmData);
        }

        bool fillRestPartOfData = false;

        bool isFirstPacketInCallback = true;
        do
        {
            // get another PCM Data Packet
            lastPcmData = movieControl->DequePCMAudio();
            // it means that it was the last decoded packet in the queue
            if (nullptr == lastPcmData)
                break;

            if (isFirstPacketInCallback)
            {
                movieControl->lastDecodedAudioPTS = lastPcmData->pts;
                isFirstPacketInCallback = false;
            }
            // empty space in the data buffer
            uint32 fmodDataEmptySpace = datalen - fmodDataWritten;
            // if data came couldn't be written into the FMOD buffer fully
            if (lastPcmData->size - lastPcmData->written >= fmodDataEmptySpace)
            {
                //               |     to write       |to move|
                //               [         new data size      ]
                // |             |   dataSizeToWrite  |
                // [written data |     empty space    ]
                Memcpy((uint8*)data + fmodDataWritten, lastPcmData->data + lastPcmData->written, fmodDataEmptySpace);
                lastPcmData->written += fmodDataEmptySpace;
                fmodDataWritten += fmodDataEmptySpace;

                return FMOD_OK;
            }
            else
            {
                // write all data rest
                uint32 dataSizeToWrite = lastPcmData->size - lastPcmData->written;
                Memcpy((uint8*)data + fmodDataWritten, lastPcmData->data, dataSizeToWrite);
                lastPcmData->written += dataSizeToWrite;
                fmodDataWritten += dataSizeToWrite;
                fillRestPartOfData = true;

                SafeDelete(lastPcmData);
            }
        } while (fillRestPartOfData);

        SafeDelete(lastPcmData);

        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK MovieViewControl::PcmReadDecodeCallback(FMOD_SOUND* sound, void* data, unsigned int datalen)
    {
        // Read from your buffer here...
        void* soundData;
        reinterpret_cast<FMOD::Sound*>(sound)->getUserData(&soundData);

        MovieViewControl* movieControl = reinterpret_cast<MovieViewControl*>(soundData);

        if (nullptr == movieControl)
            return FMOD_ERR_MUSIC_UNINITIALIZED;

        Memset(data, 0, datalen);

        static DecodedPCMData* lastPcmData = nullptr;

        uint32 fmodDataWritten = 0;

        // Write the data remains from previous call
        if (nullptr != lastPcmData)
        {
            if (lastPcmData->size - lastPcmData->written > 0)
            {
                // |     fmodDataWritten  |     size    |
                // [.....written data.....][data to write]
                uint32 dataSizeToWrite = lastPcmData->size - lastPcmData->written;
                Logger::Error("%d pcm data written", dataSizeToWrite);
                Memcpy((uint8*)data + fmodDataWritten, lastPcmData->data + lastPcmData->written, dataSizeToWrite);
                fmodDataWritten += dataSizeToWrite;
            }
            SafeDelete(lastPcmData);
        }

        bool fillRestPartOfData = false;

        do
        {
            // get another PCM Data Packet
            lastPcmData = movieControl->DecodeAudioInPlace();
            // it means that it was the last decoded packet in the queue
            if (!movieControl->hasMoreData || nullptr == lastPcmData && !fillRestPartOfData)
                break;

            if (nullptr == lastPcmData && fillRestPartOfData)
                continue;

            movieControl->lastDecodedAudioPTS = lastPcmData->pts;
            Logger::Error("lastDecodedAudioPTS = %f", movieControl->lastDecodedAudioPTS);
            // empty space in the data buffer
            uint32 fmodDataEmptySpace = datalen - fmodDataWritten;
            // if data came couldn't be written into the FMOD buffer fully
            if (lastPcmData->size - lastPcmData->written >= fmodDataEmptySpace)
            {
                //               |     to write       |to move|
                //               [         new data size      ]
                // |             |   dataSizeToWrite  |
                // [written data |     empty space    ]
                Logger::Error("%d of %d pcm data written", fmodDataEmptySpace, lastPcmData->size - lastPcmData->written);
                Memcpy((uint8*)data + fmodDataWritten, lastPcmData->data + lastPcmData->written, fmodDataEmptySpace);
                lastPcmData->written += fmodDataEmptySpace;
                fmodDataWritten += fmodDataEmptySpace;

                return FMOD_OK;
            }
            else
            {
                // write all data rest
                uint32 dataSizeToWrite = lastPcmData->size - lastPcmData->written;
                Logger::Error("%d of %d pcm data written", dataSizeToWrite, lastPcmData->size - lastPcmData->written);
                Memcpy((uint8*)data + fmodDataWritten, lastPcmData->data, dataSizeToWrite);
                lastPcmData->written += dataSizeToWrite;
                fmodDataWritten += dataSizeToWrite;
                fillRestPartOfData = fmodDataEmptySpace > 0;
                SafeDelete(lastPcmData);
            }
        } while (fillRestPartOfData);

        SafeDelete(lastPcmData);

        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK MovieViewControl::pcmsetposcallback(FMOD_SOUND* sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
    {
        // Seek to a location in your data, may not be required for what you want to do
        return FMOD_OK;
    }

    bool MovieViewControl::InitAudio()
    {
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
            return false;
        }

        // Get a pointer to the codec context for the audio stream
        audioCodecContext = movieContext->streams[audioStreamIndex]->codec;

        // Find the decoder for the audio stream
        audioCodec = avcodec_find_decoder(audioCodecContext->codec_id);
        if (audioCodec == nullptr)
        {
            printf("Codec not found.\n");
            return false;
        }

        // Open codec
        if (avcodec_open2(audioCodecContext, audioCodec, nullptr) < 0)
        {
            printf("Could not open codec.\n");
            return false;
        }

        //Out Audio Param
        uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
        //nb_samples: AAC-1024 MP3-1152
        int out_nb_samples = audioCodecContext->frame_size;
        AV::AVSampleFormat out_sample_fmt = AV::AV_SAMPLE_FMT_S16;
        int out_sample_rate = 44100;
        int out_channels = AV::av_get_channel_layout_nb_channels(out_channel_layout);
        //Out Buffer Size
        outAudioBufferSize = static_cast<uint32>(av_samples_get_buffer_size(nullptr, out_channels, out_nb_samples, out_sample_fmt, 1));

        audioFrame = AV::av_frame_alloc();

        //FIX:Some Codec's Context Information is missing
        in_channel_layout = AV::av_get_default_channel_layout(audioCodecContext->channels);

        audioConvertContext = AV::swr_alloc_set_opts(audioConvertContext, out_channel_layout, out_sample_fmt, out_sample_rate, in_channel_layout, audioCodecContext->sample_fmt, audioCodecContext->sample_rate, 0, nullptr);
        AV::swr_init(audioConvertContext);

        SafeRelease(pcmData);
        pcmData = DynamicMemoryFile::Create(File::CREATE | File::READ | File::WRITE);
        // Init FMOD
        {
            FMOD_CREATESOUNDEXINFO exinfo;
            memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

            exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO); /* required. */
            exinfo.decodebuffersize = out_sample_rate; /* Chunk size of stream update in samples.  This will be the amount of data passed to the user callback. */
            exinfo.length = out_sample_rate * out_channels * sizeof(signed short) * 5; /* Length of PCM data in bytes of whole song (for Sound::getLength) */
            exinfo.numchannels = out_channels; /* Number of channels in the sound. */
            exinfo.defaultfrequency = out_sample_rate; /* Default playback rate of sound. */
            exinfo.format = FMOD_SOUND_FORMAT_PCM16; /* Data format of sound. */
            //exinfo.pcmreadcallback = &MovieViewControl::pcmreadcallback; /* User callback for reading. */
            exinfo.pcmreadcallback = &MovieViewControl::PcmReadDecodeCallback; /* User callback for reading. */
            exinfo.pcmsetposcallback = &MovieViewControl::pcmsetposcallback; /* User callback for seeking. */

            FMOD::Sound* sound;
            FMOD::System* system = SoundSystem::Instance()->GetFmodSystem();
            FMOD_RESULT result = system->createStream(nullptr, FMOD_OPENUSER, &exinfo, &sound);
            sound->setUserData(this);

            if (nullptr != fmodChannel)
            {
                fmodChannel->setPaused(true);
            }
            system->playSound(FMOD_CHANNEL_FREE, sound, true, &fmodChannel);
            fmodChannel->setLoopCount(-1);
            fmodChannel->setMode(FMOD_LOOP_NORMAL);
            fmodChannel->setPosition(0, FMOD_TIMEUNIT_MS); // this flushes the buffer to ensure the loop mode takes effect
            fmodChannel->setPaused(false);
        }

        StartPlayingTimer();

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
        frame_delay = AV::av_q2d(movieContext->streams[videoStreamIndex]->time_base);
        /* if we are repeating a frame, adjust clock accordingly */
        frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
        video_clock += frame_delay;

        return pts;
    }

    bool MovieViewControl::DecodeVideoPacket(AV::AVPacket* packet, float64& pts)
    {
        int32 got_picture;
        int32 ret = AV::avcodec_decode_video2(videoCodecContext, decodedFrame, &got_picture, packet);

        if (ret < 0)
        {
            Logger::Error("Video Decode Error. %d", ret);
            return false;
        }

        if (got_picture)
        {
            DecodedFrameBuffer* frameBuffer = new DecodedFrameBuffer();
            frameBuffer->data = new uint8[textureBufferSize];
            // a trick to get converted data into one buffer with textureBufferSize because it could be larger than frame frame size.
            uint8* rgbTextureBufferHolder[1];
            rgbTextureBufferHolder[0] = frameBuffer->data;
            // we fill codecContext->width x codecContext->height area, it is smaller than texture size. So fill all the texture by empty color once.
            // we suppose that next time we will fill same part of the texture.
            Memset(frameBuffer->data, emptyPixelColor, textureBufferSize);

            // rgbTextureBufferHolder is a pointer to pointer to uint8. Used to obtain data directly to our rgbTextureBuffer
            const uint32 scaledHeight = AV::sws_scale(img_convert_ctx, decodedFrame->data, decodedFrame->linesize, 0, frameHeight, rgbTextureBufferHolder, rgbDecodedScaledFrame->linesize);
            frameBuffer->textureFormat = textureFormat;

            EnqueueDecodedVideoBuffer(frameBuffer);

            float64 effectivePTS = GetPTSForFrame(decodedFrame, packet, videoStreamIndex);
            effectivePTS = synchronize_video(decodedFrame, effectivePTS);
            lastDecodedVideoPTS = effectivePTS;
            frameBuffer->pts = effectivePTS;

            /* update the video clock */
            float64 frame_delay = static_cast<float64>(AV::av_q2d(movieContext->streams[videoStreamIndex]->time_base));
            /* if we are repeating a frame, adjust clock accordingly */
            frame_delay += decodedFrame->repeat_pict * (frame_delay * 0.5f);
            uint32 sleepTime = frame_delay * 1000;
            frameBuffer->sleepAfterPresent = frame_delay;

            pts = effectivePTS;

            return true;
        }
        return false;
    }

    void MovieViewControl::UpdateVideo(float64 elapsedTime)
    {
        DecodedFrameBuffer* frameBuffer = DequeueDecodedVideoBuffer();
        if (nullptr == frameBuffer)
        {
            Thread::Sleep(1);
            return;
        }

        pcmMutex.Lock();
        float64 audioPTS = lastDecodedAudioPTS;
        pcmMutex.Unlock();

        float64 actual_delay;
        float64 delay;
        float64 sync_threshold;
        float64 ref_clock;
        float64 diff;

        delay = frameBuffer->pts - lastDecodedVideoPTS; /* the pts from last time */
        Logger::Error("Delay %f", delay);
        if (delay <= 0 || delay >= 1.0)
        {
            /* if incorrect delay, use previous one */
            delay = frame_last_delay;
        }
        /* save for next time */
        frame_last_delay = delay;
        lastDecodedVideoPTS = frameBuffer->pts;

        /* update delay to sync to audio */
        ref_clock = GetAudioClock();
        Logger::Error("RefClock %f", ref_clock);
        diff = frameBuffer->pts - ref_clock;

        /* Skip or repeat the frame. Take delay into account
        FFPlay still doesn't "know if this is the best guess." */
        sync_threshold = (delay > 0.01f) ? delay : 0.01;
        if (fabs(diff) < 10)
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
        ShiftPlayTime(-delay);

        /* computer the REAL delay */
        actual_delay = GetPlayTime();
        if (actual_delay < 0.010)
        {
            /* Really it should skip the picture instead */
            actual_delay = 0.010;
        }

        uint32 sleepTime = static_cast<uint32>(actual_delay * 1000 + 0.5);

        Logger::Error("sleep time %d", playTime, sleepTime);

        if (nullptr == videoTexture)
        {
            // rgbTextureBuffer is a rgbTextureBufferHolder[0]
            videoTexture = Texture::CreateFromData(frameBuffer->textureFormat, frameBuffer->data, textureWidth, textureHeight, false);
            Sprite* videoSprite = Sprite::CreateFromTexture(videoTexture, 0, 0, static_cast<int32>(frameWidth), static_cast<int32>(frameHeight), static_cast<float64>(frameWidth), static_cast<float64>(frameHeight));
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

        Thread::Sleep(sleepTime);
        StartPlayingTimer();
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

        float64 effectivePTS = static_cast<float64>(pts * AV::av_q2d(movieContext->streams[stream]->time_base));
        return effectivePTS;
    }

    MovieViewControl::DecodedPCMData* MovieViewControl::DecodeAudioInPlace()
    {
        audioPacketsMutex.Lock();
        auto size = audioPackets.size();
        audioPacketsMutex.Unlock();

        if (size == 0)
            return nullptr;

        audioPacketsMutex.Lock();
        AV::AVPacket* packet = audioPackets.front();
        audioPackets.pop_front();
        audioPacketsMutex.Unlock();

        SCOPE_EXIT
        {
            AV::av_packet_unref(packet);
        };

        DVASSERT(packet->stream_index == audioStreamIndex);
        int got_data;
        int ret = avcodec_decode_audio4(audioCodecContext, audioFrame, &got_data, packet);
        if (ret < 0)
        {
            printf("Error in decoding audio frame.\n");
            return nullptr;
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
            return nullptr;
        }

        DecodedPCMData* data = new DecodedPCMData();
        data->data = outAudioBuffer;
        data->size = outAudioBufferSize;
        data->pts = GetPTSForFrame(audioFrame, packet, audioStreamIndex);

        return data;
    }

    void MovieViewControl::DecodeAudio(AV::AVPacket* packet, float64 timeElapsed)
    {
        DVASSERT(packet->stream_index == audioStreamIndex);
        int got_data;
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

        DecodedPCMData* data = new DecodedPCMData();
        data->data = outAudioBuffer;
        data->size = outAudioBufferSize;
        //data->pts = GetPTSForFrame(audioFrame, packet, audioStreamIndex);

        if (packet->dts != AV_NOPTS_VALUE)
        {
            data->pts = packet->pts;
        }
        else
        {
            data->pts = 0;
        }
        if (data->pts != AV_NOPTS_VALUE)
        {
            audio_clock = AV::av_q2d(movieContext->streams[audioStreamIndex]->time_base) * data->pts;
        }

        decodedAudioMutex.Lock();
        decodedAudio.push_back(data);
        decodedAudioMutex.Unlock();
    }

    void MovieViewControl::AudioDecodingThread(BaseObject* caller, void* callerData, void* userData)
    {
        Thread* thread = static_cast<Thread*>(caller);

        //return;
        AV::AVPacket* audioPacket = nullptr;
        do
        {
            audioPacketsMutex.Lock();
            auto size = audioPackets.size();
            audioPacketsMutex.Unlock();

            if (size > 0)
            {
                audioPacketsMutex.Lock();
                audioPacket = audioPackets.front();
                audioPackets.pop_front();
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
            videoPacketsMutex.Lock();
            auto size = videoPackets.size();
            videoPacketsMutex.Unlock();

            if (size > 0)
            {
                videoPacketsMutex.Lock();
                videoPacket = videoPackets.front();
                videoPackets.pop_front();
                videoPacketsMutex.Unlock();

                float64 pts;
                DecodeVideoPacket(videoPacket, pts);
                AV::av_packet_unref(videoPacket);
            }
            else
            {
                Thread::Sleep(0);
            }

        } while (thread && !thread->IsCancelling());
    }

    void MovieViewControl::VideoPresentationThread(BaseObject* caller, void* callerData, void* userData)
    {
        Thread* thread = static_cast<Thread*>(caller);
        float64 curPlayTime = 0.f;
        float64 delta = 0.f;

        while (lastDecodedAudioPTS < 0)
        {
            Thread::Sleep(1);
        }

        do
        {
            //Thread::Sleep(100);

            curPlayTime = GetPlayTime();

            UpdateVideo(delta);

            delta = GetPlayTime() - curPlayTime;

        } while (thread && !thread->IsCancelling());
    }

    void MovieViewControl::ReadingThread(BaseObject* caller, void* callerData, void* userData)
    {
        Thread* thread = static_cast<Thread*>(caller);

        int retRead = 0;
        do
        {
            if (!isPlaying)
            {
                Thread::Sleep(0);
                continue;
            }

            AV::AVPacket* packet = static_cast<AV::AVPacket*>(AV::av_mallocz(sizeof(AV::AVPacket)));
            av_init_packet(packet);
            retRead = AV::av_read_frame(movieContext, packet);

            if (retRead < 0)
            {
                hasMoreData = false;
            }
            else
            {
                hasMoreData = true;
            }

            SotrPacketsByVideoAndAudio(packet);
        } while (retRead >= 0 && thread && !thread->IsCancelling());
    }

    // UIControl update and draw implementation
    void MovieViewControl::Update(float32 timeElapsed)
    {
        if (!isAudioVideoStreamsInited)
            return;

        if (!isPlaying)
            return;

        lastUpdateTime += timeElapsed;
        // UpdateVideo(timeElapsed);
    }

    void MovieViewControl::Stop()
    {
        if (!isAudioVideoStreamsInited)
            return;

        isPlaying = false;
        fmodChannel->setPaused(isPlaying);
        readingDataThread->Cancel();
        videoDecodingThread->Cancel();
        videoPresentationThread->Cancel();
        audioDecodingThread->Cancel();
        SafeRelease(readingDataThread);
        SafeRelease(videoDecodingThread);
        SafeRelease(videoPresentationThread);
        SafeRelease(audioDecodingThread);
    }

}