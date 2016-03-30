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

#include "Render\PixelFormatDescriptor.h"
namespace DAVA
{
    // Initialize the control.
    void MovieViewControl::Initialize(const Rect& rect)
    {
        AV::av_register_all();
        AV::avformat_network_init();
        pFormatCtx = AV::avformat_alloc_context();

        if (AV::avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0){
            printf("Couldn't open input stream.\n");
            return;
        }
        if (AV::avformat_find_stream_info(pFormatCtx, NULL)<0){
            printf("Couldn't find stream information.\n");
            return;
        }
        videoindex = -1;
        for (i = 0; i<pFormatCtx->nb_streams; i++)
            if (pFormatCtx->streams[i]->codec->codec_type == AV::AVMEDIA_TYPE_VIDEO){
                videoindex = i;
                break;
            }
        if (videoindex == -1){
            printf("Didn't find a video stream.\n");
            return;
        }


        pCodecCtx = pFormatCtx->streams[videoindex]->codec;
        pCodec = AV::avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL){
            printf("Codec not found.\n");
            return;
        }
        if (AV::avcodec_open2(pCodecCtx, pCodec, NULL)<0){
            printf("Could not open codec.\n");
            return;
        }



        AV::AVPixelFormat avPixelFormat = AV::AV_PIX_FMT_YUV420P;

        pFrame = AV::av_frame_alloc();
        pFrameYUV = AV::av_frame_alloc();
        out_buffer = (uint8_t *)AV::av_malloc(AV::avpicture_get_size(avPixelFormat, pCodecCtx->width, pCodecCtx->height));
        AV::avpicture_fill((AV::AVPicture *)pFrameYUV, out_buffer, avPixelFormat, pCodecCtx->width, pCodecCtx->height);
        packet = (AV::AVPacket *)AV::av_malloc(sizeof(AV::AVPacket));
        //Output Info-----------------------------
        printf("--------------- File Information ----------------\n");
        AV::av_dump_format(pFormatCtx, 0, filepath, 0);
        printf("-------------------------------------------------\n");
        img_convert_ctx = AV::sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            pCodecCtx->width, pCodecCtx->height, avPixelFormat, SWS_BICUBIC, NULL, NULL, NULL);


        screen_w = pCodecCtx->width;
        screen_h = pCodecCtx->height;
    }

    // Position/visibility.
    void MovieViewControl::SetRect(const Rect& rect)
    {

    }

    void MovieViewControl::SetVisible(bool isVisible)
    {

    }

    // Open the Movie.
    void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
    {

    }

    // Start/stop the video playback.
    void MovieViewControl::Play()
    {

    }

    void MovieViewControl::Stop()
    {

    }

    // Pause/resume the playback.
    void MovieViewControl::Pause()
    {

    }

    void MovieViewControl::Resume()
    {

    }

    // Whether the movie is being played?
    bool MovieViewControl::IsPlaying() const
    {
        return isPlaying;
    }


    void YUVToRGB(AV::AVFrame* yuvFrame, char8 * outRGBBuffer, uint32 bufferSize, uint32 width, uint32 height, uint32 bytesPerPixel)
    {
        for (int i = 0; i < height; i++) //Y
        {
            uint8_t yShift = 0, uShift = 0, vShift = 0;
            bool inBuffer = (i <= height);
            if (inBuffer)
            {
                yShift = 0;
                uShift = 0;
                vShift = 0;
            }

            for (int j = 0; j < width; j++) //X
            {
                const int32 index = (i * width + j) * bytesPerPixel;

                if (inBuffer && j < width)
                {
                    size_t yPos = yShift + j;
                    const unsigned char Y = yuvFrame->data[0][yPos];
                    size_t uPos = uShift + j / 2;
                    const unsigned char U = yuvFrame->data[1][uPos];
                    size_t vPos = vShift + j / 2;
                    const unsigned char V = yuvFrame->data[2][vPos];

                    outRGBBuffer[index]     = static_cast<char8>(Clamp(Y + 1.371f * (V - 128), 0.f, 255.f));
                    outRGBBuffer[index + 1] = static_cast<char8>(Clamp(Y - 0.698f * (V - 128) - 0.336f * (U - 128), 0.f, 255.f));
                    outRGBBuffer[index + 2] = static_cast<char8>(Clamp(Y + 1.732f * (U - 128), 0.f, 255.f));
                }
                else
                {
                    memset(&outRGBBuffer[index], 0, bytesPerPixel * sizeof(unsigned char));
                }
            }
        }
    }

    // UIControl update and draw implementation
    void MovieViewControl::Update(float32 timeElapsed)
    {

        static float32 time = 0.f;

        time += timeElapsed;
        if (time < 0.40f)
            return;

        const uint32 imageWidth = 1024;
        const uint32 imageHeight = 768;
        const PixelFormat textureFormat = PixelFormat::FORMAT_RGB888;
        const uint32 pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(textureFormat);
        const uint32 bufferSize = pixelSize * imageWidth * imageHeight;
        char8* buffer = new char8[bufferSize];
        Memset(buffer, 0xff, bufferSize);


        Texture * videoTexture = nullptr;
        if (AV::av_read_frame(pFormatCtx, packet) >= 0){

            if (packet->stream_index == videoindex){
                ret = AV::avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
                if (ret < 0){
                    printf("Decode Error.\n");
                    return;
                }
                if (got_picture){
                    AV::sws_scale(img_convert_ctx, (const uint8* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                        pFrameYUV->data, pFrameYUV->linesize);

                    //int * rgba = convertYUV420_NV21toARGB8888((char*)pFrame->data, pCodecCtx->width, pCodecCtx->height);
                    y_size = pCodecCtx->width*pCodecCtx->height;

                    YUVToRGB(pFrameYUV, buffer, bufferSize, pCodecCtx->width, pCodecCtx->height, pixelSize);

                    /*
                    File * f = File::Create(FilePath("D:/Projects/Win10/1.yuv"), File::OPEN | File::WRITE | File::APPEND);
                    f->Write(pFrameYUV->data[0], y_size);    //Y 
                    f->Write(pFrameYUV->data[1], y_size/4);    //U 
                    f->Write(pFrameYUV->data[2], y_size/4);    //V 
                    f->Release();

                    f = File::Create(FilePath("D:/Projects/Win10/2.yuv"), File::OPEN | File::WRITE | File::APPEND);
                    f->Write(buffer, bufferSize);    //Y 
                    f->Release();
                    
                    */
                    videoTexture = Texture::CreateFromData(textureFormat, (uint8*)buffer, imageWidth, imageHeight, false);



                }
            }
            AV::av_free_packet(packet);
        }




        
        if (videoTexture)
        {
            Sprite * videoSprite = Sprite::CreateFromTexture(videoTexture, 0, 0, float32(imageWidth), float32(imageHeight));
            videoSprite->ConvertToVirtualSize();
            GetBackground()->SetSprite(videoSprite, 0);
            SafeRelease(videoSprite);
        }
        SafeRelease(videoTexture);

        SafeDeleteArray(buffer);
    }
}