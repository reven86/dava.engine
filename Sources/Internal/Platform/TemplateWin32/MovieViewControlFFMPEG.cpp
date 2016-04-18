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
            if (yuvDecodedScaledFrame)
            {
                av_frame_free(&yuvDecodedScaledFrame);
            }
            if (decodedFrame)
            {
                av_frame_free(&decodedFrame);
            }
            if (videoCodecContext)
            {
                avcodec_close(videoCodecContext);
            }
            if (avFormatVideoContext)
            {
                avformat_close_input(&avFormatVideoContext);
            }
            if (img_convert_ctx)
            {
                sws_freeContext(img_convert_ctx);
            }

            SafeDeleteArray(rgbTextureBuffer);

            SafeRelease(pcmData);

            isFFMGEGInited = false;
        }

    }

    // Initialize the control.
    void MovieViewControl::Initialize(const Rect& rect)
    {
        Renderer::SetDesiredFPS(60);
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

    AV::AVFormatContext* MovieViewControl::CreateContext(const String& path)
    {
        AV::AVFormatContext* context = AV::avformat_alloc_context();
        bool isSuccess = true;

        if (AV::avformat_open_input(&context, path.c_str(), nullptr, nullptr) != 0)
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

        videoDecodingThread = Thread::Create(Message(this, &MovieViewControl::VideoDecodingThread));
        audioDecodingThread = Thread::Create(Message(this, &MovieViewControl::AudioDecodingThread));
        readingDataThread = Thread::Create(Message(this, &MovieViewControl::ReadingThread));

        videoDecodingThread->Start();
        audioDecodingThread->Start();

        readingDataThread->Start();
    }

    // Open the Movie.
    void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
    {
        isAudioVideoStreamsInited = false;

        avFormatVideoContext = CreateContext(filepath);
        if (nullptr == avFormatVideoContext)
        {
            Logger::Error("Can't Open video.");
        }

        avFormatAudioContext = CreateContext(filepath);
        if (nullptr == avFormatAudioContext)
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
        for (unsigned int i = 0; i < avFormatVideoContext->nb_streams; i++)
        {
            if (avFormatVideoContext->streams[i]->codec->codec_type == AV::AVMEDIA_TYPE_VIDEO)
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

        AV::AVRational avfps = avFormatVideoContext->streams[videoStreamIndex]->avg_frame_rate;
        videoFramerate = avfps.num / static_cast<float32>(avfps.den);

        videoCodecContext = avFormatVideoContext->streams[videoStreamIndex]->codec;
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

        AV::AVPixelFormat avPixelFormat = AV::AV_PIX_FMT_RGBA;

        //packet = static_cast<AV::AVPacket*>(AV::av_mallocz(sizeof(AV::AVPacket)));
        decodedFrame = AV::av_frame_alloc();
        yuvDecodedScaledFrame = AV::av_frame_alloc();
        out_buffer = (uint8_t*)AV::av_mallocz(AV::av_image_get_buffer_size(avPixelFormat, videoCodecContext->width, videoCodecContext->height, PixelFormatDescriptor::GetPixelFormatSizeInBits(textureFormat)));
        AV::avpicture_fill((AV::AVPicture*)yuvDecodedScaledFrame, out_buffer, avPixelFormat, videoCodecContext->width, videoCodecContext->height);

        img_convert_ctx = AV::sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, videoCodecContext->width, videoCodecContext->height, avPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

        textureWidth = NextPowerOf2(videoCodecContext->width);
        textureHeight = NextPowerOf2(videoCodecContext->height);
        textureBufferSize = textureWidth * textureHeight * PixelFormatDescriptor::GetPixelFormatSizeInBytes(textureFormat);
        frameHeight = videoCodecContext->height;
        frameWidth = videoCodecContext->width;

        // a trick to get converted data into one buffer with textureBufferSize because it could be larger than frame frame size.
        rgbTextureBuffer = new uint8[textureBufferSize];
        rgbTextureBufferHolder[0] = rgbTextureBuffer;
        // we fill codecContext->width x codecContext->height area, it is smaller than texture size. So fill all the texture by empty color once.
        // we suppose that next time we will fill same part of the texture.
        Memset(rgbTextureBuffer, emptyPixelColor, textureBufferSize);

        return true;
    }

    FMOD_RESULT F_CALLBACK pcmreadcallback(FMOD_SOUND* sound, void* data, unsigned int datalen)
    {
        // Read from your buffer here...

        void* soundData;
        reinterpret_cast<FMOD::Sound*>(sound)->getUserData(&soundData);

        MovieViewControl* movieControl = reinterpret_cast<MovieViewControl*>(soundData);

        // if (nullptr == movieControl || movieControl->pcmBuffers.size() <= 0)
        //   return FMOD_OK;

        if (nullptr == movieControl)
            return FMOD_OK;

        auto pcmData = movieControl->pcmData;

        if (nullptr == pcmData)
            return FMOD_OK;

        Memset(data, 0, datalen);

        uint32 pcmDataSize = pcmData->GetSize();

        movieControl->pcmMutex.Lock();
        pcmData->Seek(movieControl->readPos, File::SEEK_FROM_START);
        movieControl->readPos += pcmData->Read(data, static_cast<uint32>(datalen));
        movieControl->pcmMutex.Unlock();

        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK pcmsetposcallback(FMOD_SOUND* sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
    {
        // Seek to a location in your data, may not be required for what you want to do
        return FMOD_OK;
    }

    bool MovieViewControl::InitAudio()
    {
        for (unsigned int i = 0; i < avFormatVideoContext->nb_streams; i++)
        {
            if (avFormatVideoContext->streams[i]->codec->codec_type == AV::AVMEDIA_TYPE_AUDIO)
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
        audioCodecContext = avFormatVideoContext->streams[audioStreamIndex]->codec;

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

        outAudioBuffer = (uint8_t*)AV::av_mallocz(maxAudioFrameSize * 2);
        audioFrame = AV::av_frame_alloc();

        //FIX:Some Codec's Context Information is missing
        in_channel_layout = AV::av_get_default_channel_layout(audioCodecContext->channels);

        audioConvertContext = AV::swr_alloc_set_opts(audioConvertContext, out_channel_layout, out_sample_fmt, out_sample_rate, in_channel_layout, audioCodecContext->sample_fmt, audioCodecContext->sample_rate, 0, nullptr);
        AV::swr_init(audioConvertContext);

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
            exinfo.pcmreadcallback = &pcmreadcallback; /* User callback for reading. */
            exinfo.pcmsetposcallback = &pcmsetposcallback; /* User callback for seeking. */

            FMOD::Sound* sound;
            FMOD::System* system = SoundSystem::Instance()->GetFmodSystem();
            FMOD_RESULT result = system->createStream(nullptr, FMOD_OPENUSER, &exinfo, &sound);
            sound->setUserData(this);

            FMOD::Channel* channel;
            system->playSound(FMOD_CHANNEL_FREE, sound, true, &channel);
            channel->setLoopCount(-1);
            channel->setMode(FMOD_LOOP_NORMAL);
            channel->setPosition(0, FMOD_TIMEUNIT_MS); // this flushes the buffer to ensure the loop mode takes effect
            channel->setPaused(false);
        }

        SafeRelease(pcmData);
        pcmData = DynamicMemoryFile::Create(File::CREATE | File::READ | File::WRITE);

        return true;
    }

    void MovieViewControl::UpdateVideo(AV::AVPacket * packet, float32 timeElapsed)
    {
        int32 got_picture;
        int32 ret = AV::avcodec_decode_video2(videoCodecContext, decodedFrame, &got_picture, packet);

        if (ret < 0)
        {
            Logger::Error("Video Decode Error. %d", ret);
            return;
        }

        if (got_picture)
        {
            {
                // limit lower fps
                const float32 maxSkippedTime = 1.f;
                static float32 timeAfterLastRender = 0.f;
                timeAfterLastRender += timeElapsed;
                if (timeAfterLastRender >= 1 / videoFramerate && timeAfterLastRender < maxSkippedTime)
                {
                    return;
                }
                timeAfterLastRender = 0.f;
            }

            // rgbTextureBufferHolder is a pointer to pointer to uint8. Used to obtain data directly to our rgbTextureBuffer
            const uint32 scaledHeight = AV::sws_scale(img_convert_ctx, decodedFrame->data, decodedFrame->linesize, 0, frameHeight, rgbTextureBufferHolder, yuvDecodedScaledFrame->linesize);

            if (nullptr == videoTexture)
            {
                // rgbTextureBuffer is a rgbTextureBufferHolder[0]
                videoTexture = Texture::CreateFromData(textureFormat, rgbTextureBuffer, textureWidth, textureHeight, false);
                Sprite * videoSprite = Sprite::CreateFromTexture(videoTexture, 0, 0, static_cast<int32>(frameWidth), static_cast<int32>(frameHeight), static_cast<float32>(frameWidth), static_cast<float32>(frameHeight));
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
                videoTexture->TexImage(0, textureWidth, textureHeight, rgbTextureBuffer, textureBufferSize, Texture::INVALID_CUBEMAP_FACE);
            }
        }
    }

    void MovieViewControl::UpdateAudio(AV::AVPacket* packet, float32 timeElapsed)
    {
        DVASSERT(packet->stream_index == audioStreamIndex);
        int got_data;
        int ret = avcodec_decode_audio4(audioCodecContext, audioFrame, &got_data, packet);
        if (ret < 0)
        {
            printf("Error in decoding audio frame.\n");
            return;
        }
        if (got_data > 0)
        {
            AV::swr_convert(audioConvertContext, &outAudioBuffer, maxAudioFrameSize, (const uint8_t**)audioFrame->data, audioFrame->nb_samples);
            index++;
        }

        pcmMutex.Lock();
        pcmData->Seek(writePos, File::SEEK_FROM_START);
        if (outAudioBufferSize == pcmData->Write(outAudioBuffer, outAudioBufferSize))
        {
            writePos += outAudioBufferSize;
        }
        pcmMutex.Unlock();
    }

    void MovieViewControl::AudioDecodingThread(BaseObject* caller, void* callerData, void* userData)
    {
        Thread* thread = static_cast<Thread*>(caller);

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

                UpdateAudio(audioPacket, 0);
                AV::av_packet_unref(audioPacket);
            }
            else
            {
                Thread::Sleep(0);
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

                UpdateVideo(videoPacket, 0);
                AV::av_packet_unref(videoPacket);
            }
            else
            {
                Thread::Sleep(0);
            }

        } while (thread && !thread->IsCancelling());
    }

    void MovieViewControl::ReadingThread(BaseObject* caller, void* callerData, void* userData)
    {
        Thread* thread = static_cast<Thread*>(caller);

        int retRead = 0;
        do
        {
            AV::AVPacket* packet = static_cast<AV::AVPacket*>(AV::av_mallocz(sizeof(AV::AVPacket)));
            av_init_packet(packet);

            retRead = AV::av_read_frame(avFormatVideoContext, packet);
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
    }

}