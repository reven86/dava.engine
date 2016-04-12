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
            if (codecContext)
            {
                avcodec_close(codecContext);
            }
            if (pFormatCtx)
            {
                avformat_close_input(&pFormatCtx);
            }
            if (img_convert_ctx)
            {
                sws_freeContext(img_convert_ctx);
            }

            if (packet)
            {
                av_packet_unref(packet);
            }

            SafeDeleteArray(rgbTextureBuffer);

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

    // Open the Movie.
    void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
    {
        pFormatCtx = AV::avformat_alloc_context();

        if (AV::avformat_open_input(&pFormatCtx, filepath, nullptr, nullptr) != 0){
            Logger::Error("Couldn't open input stream.\n");
            return;
        }
        if (AV::avformat_find_stream_info(pFormatCtx, nullptr)<0){
            Logger::Error("Couldn't find stream information.\n");
            return;
        }

        for (unsigned int i = 0; i<pFormatCtx->nb_streams; i++)
            if (pFormatCtx->streams[i]->codec->codec_type == AV::AVMEDIA_TYPE_VIDEO){
                videoIndex = i;
                break;
            }
        if (videoIndex == -1)
        {
            Logger::Error("Didn't find a video stream.\n");
            return;
        }

        AV::AVRational avfps = pFormatCtx->streams[videoIndex]->avg_frame_rate;
        videoFramerate = avfps.num / static_cast<float32>(avfps.den);

        codecContext = pFormatCtx->streams[videoIndex]->codec;
        pCodec = AV::avcodec_find_decoder(codecContext->codec_id);
        if (pCodec == nullptr){
            Logger::Error("Codec not found.\n");
            return;
        }
        if (AV::avcodec_open2(codecContext, pCodec, nullptr)<0){
            Logger::Error("Could not open codec.\n");
            return;
        }

        AV::AVPixelFormat avPixelFormat = AV::AV_PIX_FMT_RGBA;

        decodedFrame = AV::av_frame_alloc();
        yuvDecodedScaledFrame = AV::av_frame_alloc();
        out_buffer = (uint8_t *)AV::av_mallocz(AV::av_image_get_buffer_size(avPixelFormat, codecContext->width, codecContext->height, PixelFormatDescriptor::GetPixelFormatSizeInBits(textureFormat)));
        AV::avpicture_fill((AV::AVPicture *)yuvDecodedScaledFrame, out_buffer, avPixelFormat, codecContext->width, codecContext->height);
        packet = (AV::AVPacket *)AV::av_mallocz(sizeof(AV::AVPacket));

        img_convert_ctx = AV::sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt, codecContext->width, codecContext->height, avPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

        textureWidth = NextPowerOf2(codecContext->width);
        textureHeight = NextPowerOf2(codecContext->height);
        textureBufferSize = textureWidth * textureHeight * PixelFormatDescriptor::GetPixelFormatSizeInBytes(textureFormat);
        frameHeight = codecContext->height;
        frameWidth = codecContext->width;

        // a trick to get converted data into one buffer with textureBufferSize because it could be larger than frame frame size.
        rgbTextureBuffer = new uint8[textureBufferSize];
        rgbTextureBufferHolder[0] = rgbTextureBuffer;
        // we fill codecContext->width x codecContext->height area, it is smaller than texture size. So fill all the texture by empty color once.
        // we suppose that next time we will fill same part of the texture.
        Memset(rgbTextureBuffer, emptyPixelColor, textureBufferSize);

        Renderer::SetDesiredFPS(60);
    }

    // Start/stop the video playback.
    void MovieViewControl::Play()
    {
        isPlaying = true;
    }

    void MovieViewControl::Stop()
    {
        isPlaying = false;
    }

    // Pause/resume the playback.
    void MovieViewControl::Pause()
    {
        isPlaying = false;
    }

    void MovieViewControl::Resume()
    {
        isPlaying = true;
    }

    // Whether the movie is being played?
    bool MovieViewControl::IsPlaying() const
    {
        return isPlaying;
    }


    void MovieViewControl::UpdateVideo(AV::AVPacket * packet, float32 timeElapsed)
    {
        int32 got_picture;
        int32 ret = AV::avcodec_decode_video2(codecContext, decodedFrame, &got_picture, packet);
        SCOPE_EXIT
        {
            AV::av_packet_unref(packet);
        };

        if (ret < 0){
            Logger::Error("Video Decode Error.\n");
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

    // UIControl update and draw implementation
    void MovieViewControl::Update(float32 timeElapsed)
    {
        if (!isPlaying)
            return;

        {
            // limit higher fps        
            static float32 timeAfterLastRender = 0.f;
            timeAfterLastRender += timeElapsed;
            if (timeAfterLastRender < 1 / videoFramerate)
            {
                return;
            }
            timeAfterLastRender = 0.f;
        }

        do
        {
            int retRead = -1;
            retRead = AV::av_read_frame(pFormatCtx, packet);
            if (retRead < 0)
            {
                //  Logger::FrameworkDebug("EOF or error");
                return;
            }
        } while (packet->stream_index != videoIndex);

        if (packet->stream_index == videoIndex)
            ;
        UpdateVideo(packet, timeElapsed);
        //        if (packet->stream_index == audioIndex)
        //      UpdateAudio(packet, timeElapsed);
    }

}