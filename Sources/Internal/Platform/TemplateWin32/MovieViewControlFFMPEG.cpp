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
        SafeDeleteArray(decodedFrameBuffer);

        if (isFFMGEGInited)
        {
            if (pFrameYUV)
            {
                av_frame_free(&pFrameYUV);
            }
            if (pFrame)
            {
                av_frame_free(&pFrame);
            }
            if (pCodecCtx)
            {
                avcodec_close(pCodecCtx);
            }
            if (pFormatCtx)
            {
                avformat_close_input(&pFormatCtx);
            }
            if (img_convert_ctx)
            {
                sws_freeContext(img_convert_ctx);
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
            printf("Couldn't open input stream.\n");
            return;
        }
        if (AV::avformat_find_stream_info(pFormatCtx, nullptr)<0){
            printf("Couldn't find stream information.\n");
            return;
        }

        for (unsigned int i = 0; i<pFormatCtx->nb_streams; i++)
            if (pFormatCtx->streams[i]->codec->codec_type == AV::AVMEDIA_TYPE_VIDEO){
                videoindex = i;
                break;
            }
        if (videoindex == -1){
            printf("Didn't find a video stream.\n");
            return;
        }

        AV::AVRational avfps = pFormatCtx->streams[videoindex]->avg_frame_rate;
        framerate = avfps.num / static_cast<float32>(avfps.den);
        
        pCodecCtx = pFormatCtx->streams[videoindex]->codec;
        pCodec = AV::avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == nullptr){
            printf("Codec not found.\n");
            return;
        }
        if (AV::avcodec_open2(pCodecCtx, pCodec, nullptr)<0){
            printf("Could not open codec.\n");
            return;
        }

        AV::AVPixelFormat avPixelFormat = AV::AV_PIX_FMT_YUV420P;

        pFrame = AV::av_frame_alloc();
        pFrameYUV = AV::av_frame_alloc();
        out_buffer = (uint8_t *)AV::av_mallocz(AV::av_image_get_buffer_size(avPixelFormat, pCodecCtx->width, pCodecCtx->height, PixelFormatDescriptor::GetPixelFormatSizeInBits(textureFormat)));
        AV::avpicture_fill((AV::AVPicture *)pFrameYUV, out_buffer, avPixelFormat, pCodecCtx->width, pCodecCtx->height);
        packet = (AV::AVPacket *)AV::av_mallocz(sizeof(AV::AVPacket));

        img_convert_ctx = AV::sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, avPixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

        textureWidth = NextPowerOf2(pCodecCtx->width);
        textureHeight = NextPowerOf2(pCodecCtx->height);
        textureBufferSize = textureWidth * textureHeight * PixelFormatDescriptor::GetPixelFormatSizeInBytes(textureFormat);
        decodedFrameBuffer = new char8[textureBufferSize];
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


    void YUVToRGB(const AV::AVFrame* const yuvFrame, char8 * outRGBBuffer, const Vector2 & frameSize, const Vector2 & textureSize)
    {
        DVASSERT(textureSize.dx >= frameSize.dx && textureSize.dy >= frameSize.dy);

        const uint32 bytesPerPixel = PixelFormatDescriptor::GetPixelFormatSizeInBytes(PixelFormat::FORMAT_RGBA8888);
        
        for (uint32 i = 0; i < textureSize.dy; i++) //Y
        {
            uint32 yShift = 0;
            uint32 uShift = 0;
            uint32 vShift = 0;

            // convert only significant bytes.
            bool inBuffer = (i < frameSize.dy);
            if (inBuffer)
            {
                yShift = yuvFrame->linesize[0] * i;
                uShift = yuvFrame->linesize[1] * (i / 2);
                vShift = yuvFrame->linesize[2] * (i / 2);
            }

            for (uint32 j = 0; j < textureSize.dx; j++) //X
            {
                const int32 index = (i * frameSize.dx + j) * bytesPerPixel;

                if (inBuffer && j < frameSize.dx)
                {
                    const unsigned char Y = yuvFrame->data[0][yShift + j];
                    const unsigned char U = yuvFrame->data[1][uShift + j / 2];
                    const unsigned char V = yuvFrame->data[2][vShift + j / 2];
                    
                    // R G B A layout
                    outRGBBuffer[index] = static_cast<char8>(Clamp(Y + 1.371f * (V - 128), 0.f, 255.f));
                    outRGBBuffer[index + 1] = static_cast<char8>(Clamp(Y - 0.698f * (V - 128) - 0.336f * (U - 128), 0.f, 255.f));
                    outRGBBuffer[index + 2] = static_cast<char8>(Clamp(Y + 1.732f * (U - 128), 0.f, 255.f));                    
                    outRGBBuffer[index + 3] = static_cast<char8>(255);
                }
                else
                {
                    memset(&outRGBBuffer[index], 0, bytesPerPixel * sizeof(char8));
                }
            }
        }
    }

    // UIControl update and draw implementation
    void MovieViewControl::Update(float32 timeElapsed)
    {
        if (!isPlaying)
            return;

        do
        {
            if (AV::av_read_frame(pFormatCtx, packet) < 0)
                break;
        }while (packet->stream_index != videoindex);


        int32 got_picture;
        int32 ret = AV::avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);               
        if (ret < 0){
            Logger::Error("Video Decode Error.\n");
            return;
        }

        if (got_picture)
        {
            // limit fps        
            static float32 timeAfterLastRender = 0.f;
            timeAfterLastRender += timeElapsed;
            Logger::Error("passed: %f", timeElapsed);

            if (timeAfterLastRender < 1 / framerate)
            {
                return;
            }
            timeAfterLastRender = 0.f;

            AV::sws_scale(img_convert_ctx, reinterpret_cast<const uint8* const*>(pFrame->data), pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

            YUVToRGB(pFrameYUV, decodedFrameBuffer, Vector2(pCodecCtx->width, pCodecCtx->height), Vector2(textureWidth, textureHeight));

            if (nullptr == videoTexture)
            {
                videoTexture = Texture::CreateFromData(textureFormat, reinterpret_cast<uint8*>(decodedFrameBuffer), textureWidth, textureHeight, false);
                Sprite * videoSprite = Sprite::CreateFromTexture(videoTexture, 0, 0, pCodecCtx->width, pCodecCtx->height, pCodecCtx->width, pCodecCtx->height);
                auto back = GetBackground();
                if (back)
                {
                    back->SetSprite(videoSprite, 0);
                }
                SafeRelease(videoSprite);
            }
            else
            {
                videoTexture->TexImage(0, textureWidth, textureHeight, reinterpret_cast<void *>(decodedFrameBuffer), textureBufferSize, Texture::INVALID_CUBEMAP_FACE);
            }
        }
        AV::av_packet_unref(packet);
    }

}