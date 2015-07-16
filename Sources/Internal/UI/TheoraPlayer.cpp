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


#include "UI/TheoraPlayer.h"
#include "FileSystem/YamlNode.h"
#include "Render/2D/Systems/RenderSystem2D.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include <theora/theoradec.h>

namespace DAVA
{
    
struct TheoraData
{
    th_info             thInfo;
    th_comment          thComment;
    th_setup_info       * thSetup;
    th_dec_ctx          * thCtx;
    ogg_sync_state      syncState;
    ogg_stream_state    state;
    ogg_page            page;
    ogg_packet          packet;
    th_ycbcr_buffer     yuvBuffer;
    ogg_int64_t         videoBufGranulePos;
};
    
    
TheoraPlayer::TheoraPlayer(const FilePath & _filePath)
    : currFrameTime(0)
    , frameTime(0)
    , videoTime(0)
    , file(0)
    , frameBuffer(0)
    , theora_p(0)
    , isVideoBufReady(false)
    , isPlaying(false)
    , isRepeat(false)
{
    theoraData = new TheoraData();
    theoraData->thSetup = 0;
    theoraData->thCtx = 0;
    theoraData->videoBufGranulePos = -1;
    filePath = _filePath;
    OpenFile(filePath);
}

TheoraPlayer::~TheoraPlayer()
{
    ReleaseData();
    SafeDelete(theoraData);
}

int32 TheoraPlayer::BufferData()
{
    char * buffer = ogg_sync_buffer(&theoraData->syncState, 4096);
    int32 bytes = file->Read(buffer, 4096);
    ogg_sync_wrote(&theoraData->syncState, bytes);
    return bytes;
}

void TheoraPlayer::ReleaseData()
{
    if(theoraData)
    {
        if(theoraData->thSetup)
            th_setup_free(theoraData->thSetup);
        theoraData->thSetup = 0;
        if(theoraData->thCtx)
            th_decode_free(theoraData->thCtx);
        theoraData->thCtx = 0;
        theoraData->videoBufGranulePos = -1;
        ogg_sync_clear(&theoraData->syncState);
    }
    theora_p = 0;
    isVideoBufReady = false;
    videoTime = 0;
    SafeRelease(file);
    if(frameBuffer)
    {
        delete [] frameBuffer;
        frameBuffer = 0;
    }
    isPlaying = false;
}

void TheoraPlayer::OpenFile(const FilePath &path)
{
    ReleaseData();
    
    if(path.IsEmpty())
        return;
    
    filePath = path;
    
    file = File::Create(path, File::OPEN | File::READ);
    ogg_sync_init(&theoraData->syncState);
    th_info_init(&theoraData->thInfo);
    th_comment_init(&theoraData->thComment);
    
    int32 stateflag = 0;
    while(!stateflag)
    {
        if(!BufferData())
            break;
        
        while(ogg_sync_pageout(&theoraData->syncState, &theoraData->page) > 0)
        {
            ogg_stream_state test;
            
            /* is this a mandated initial header? If not, stop parsing */
            if(!ogg_page_bos(&theoraData->page))
            {
                /* don't leak the page; get it into the appropriate stream */
                ogg_stream_pagein(&theoraData->state, &theoraData->page);
                stateflag = 1;
                break;
            }
            
            ogg_stream_init(&test, ogg_page_serialno(&theoraData->page));
            ogg_stream_pagein(&test, &theoraData->page);
            ogg_stream_packetout(&test, &theoraData->packet);
            
            /* identify the codec: try theora */
            if(!theora_p && th_decode_headerin(&theoraData->thInfo, &theoraData->thComment, &theoraData->thSetup, &theoraData->packet) >= 0)
            {
                /* it is theora */
                memcpy(&theoraData->state, &test, sizeof(test));
                theora_p = 1;
            }
            else
            {
                /* whatever it is, we don't care about it */
                ogg_stream_clear(&test);
            }
        }
        /* fall through to non-bos page parsing */
    }
    
    while(theora_p && theora_p < 3)
    {
        int ret;
        
        /* look for further theora headers */
        while(theora_p && (theora_p < 3) && (ret = ogg_stream_packetout(&theoraData->state, &theoraData->packet)))
        {
            if(ret < 0)
            {
                Logger::Error("TheoraPlayer: Error parsing Theora stream headers; corrupt stream?\n");
                return;
            }
            if(!th_decode_headerin(&theoraData->thInfo, &theoraData->thComment, &theoraData->thSetup, &theoraData->packet))
            {
                Logger::Error("TheoraPlayer: Error parsing Theora stream headers; corrupt stream?\n");
                return;
            }
            theora_p++;
        }
        
        /* The header pages/packets will arrive before anything else we
         care about, or the stream is not obeying spec */
        
        if(ogg_sync_pageout(&theoraData->syncState, &theoraData->page) > 0)
        {
            ogg_stream_pagein(&theoraData->state, &theoraData->page); /* demux into the appropriate stream */
        }
        else
        {
            /* someone needs more data */
            if(BufferData() == 0)
            {
                Logger::Error("TheoraPlayer: End of file while searching for codec headers.\n");
                return;
            }
        }
    }
    if(theora_p)
    {
        theoraData->thCtx = th_decode_alloc(&theoraData->thInfo, theoraData->thSetup);
        
        th_decode_ctl(theoraData->thCtx, TH_DECCTL_GET_PPLEVEL_MAX, &pp_level_max, sizeof(pp_level_max));
        pp_level=pp_level_max;
        th_decode_ctl(theoraData->thCtx, TH_DECCTL_SET_PPLEVEL, &pp_level, sizeof(pp_level));
        pp_inc=0;
    }
    else
    {
        /* tear down the partial theora setup */
        th_info_clear(&theoraData->thInfo);
        th_comment_clear(&theoraData->thComment);
    }
    
    if(theoraData->thSetup)
        th_setup_free(theoraData->thSetup);
    theoraData->thSetup = 0;

    frameBufferW = NextPowerOf2(theoraData->thInfo.pic_width);
    frameBufferH = NextPowerOf2(theoraData->thInfo.pic_height);
    
    frameBuffer = new unsigned char[frameBufferW * frameBufferH * 4];
    
    repeatFilePos = file->GetPos();
    
    frameTime = (float32)(theoraData->thInfo.fps_denominator)/(float32)(theoraData->thInfo.fps_numerator);
    
    isPlaying = true;
}

void TheoraPlayer::SetPlaying(bool _isPlaying)
{
    isPlaying = _isPlaying;
}

void TheoraPlayer::SetRepeat(bool _isRepeat)
{
    isRepeat = _isRepeat;
}

bool TheoraPlayer::IsRepeat()
{
    return isRepeat;
}

bool TheoraPlayer::IsPlaying()
{
    return isPlaying;
}

void TheoraPlayer::Update(float32 timeElapsed)
{
    if(!isPlaying)
        return;
        
    videoTime += timeElapsed;
    
    currFrameTime += timeElapsed;
    if(currFrameTime < frameTime)
    {
        return;
    }
    else
    {
        currFrameTime -= frameTime;
    }
    
    int ret;
    
    while(theora_p && !isVideoBufReady)
    {
        ret = ogg_stream_packetout(&theoraData->state, &theoraData->packet);
        if(ret > 0)
        {
            if(pp_inc)
            {
                pp_level += pp_inc;
                th_decode_ctl(theoraData->thCtx, TH_DECCTL_SET_PPLEVEL, &pp_level, sizeof(pp_level));
                pp_inc = 0;
            }
            if(theoraData->packet.granulepos >= 0)
                th_decode_ctl(theoraData->thCtx, TH_DECCTL_SET_GRANPOS, &theoraData->packet.granulepos, sizeof(theoraData->packet.granulepos));

            if(th_decode_packetin(theoraData->thCtx, &theoraData->packet, &theoraData->videoBufGranulePos) == 0)
            {
                if((videoBufTime = (float32)th_granule_time(theoraData->thCtx, theoraData->videoBufGranulePos)) >= videoTime)
                    isVideoBufReady = true;
                else
                    pp_inc = (pp_level > 0)? -1 : 0;
            }
        }
        else
        {
            isVideoBufReady = false;
            break;
        }
    }
    
    if(!isVideoBufReady)
    {
        BufferData();
        while(ogg_sync_pageout(&theoraData->syncState, &theoraData->page) > 0)
            ogg_stream_pagein(&theoraData->state, &theoraData->page);
    }
    
    if(isVideoBufReady)
    {
        isVideoBufReady = false;
        ret = th_decode_ycbcr_out(theoraData->thCtx, theoraData->yuvBuffer);
    
        for(int i = 0; i < frameBufferH; i++) //Y
        {
            int yShift = 0, uShift = 0, vShift = 0;
            const bool inBuffer = (i <= theoraData->yuvBuffer[0].height);
            if(inBuffer)
            {
                yShift = theoraData->yuvBuffer[0].stride * i;
                uShift = theoraData->yuvBuffer[1].stride * (i / 2);
                vShift = theoraData->yuvBuffer[2].stride * (i / 2);
            }
            
            for(int j = 0; j < frameBufferW; j++) //X
            {
                const int index = (i * frameBufferW + j) * 4;
                
                if(inBuffer && j <= theoraData->yuvBuffer[0].width)
                {
                    const unsigned char Y = *(theoraData->yuvBuffer[0].data + yShift + j);
                    const unsigned char U = *(theoraData->yuvBuffer[1].data + uShift + j / 2);
                    const unsigned char V = *(theoraData->yuvBuffer[2].data + vShift + j / 2);

                    frameBuffer[index]   = (uint8)Clamp(Y + 1.371f * (V - 128), 0.f, 255.f);
                    frameBuffer[index+1] = (uint8)Clamp(Y - 0.698f * (V - 128) - 0.336f * (U - 128), 0.f, 255.f);
                    frameBuffer[index+2] = (uint8)Clamp(Y + 1.732f * (U - 128), 0.f, 255.f);
                    frameBuffer[index+3] = 255;
                }
                else
                {
                    memset(&frameBuffer[index], 0, 4 * sizeof(unsigned char));
                }
            }
        }
    
        if(!ret)
        {
            Texture * tex = Texture::CreateFromData(FORMAT_RGBA8888, frameBuffer, frameBufferW, frameBufferH, false);
            Sprite * spr = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);
            spr->ConvertToVirtualSize();

            SafeRelease(tex);
            SetSprite(spr, 0);
            SafeRelease(spr);
        }
    }
    
    if(theora_p)
    {
        double tdiff = videoBufTime - videoTime;
        /*If we have lots of extra time, increase the post-processing level.*/
        if(tdiff > theoraData->thInfo.fps_denominator * 0.25f / theoraData->thInfo.fps_numerator)
        {
            pp_inc = (pp_level < pp_level_max) ? 1 : 0;
        }
        else if(tdiff < theoraData->thInfo.fps_denominator * 0.05 / theoraData->thInfo.fps_numerator)
        {
            pp_inc = (pp_level > 0)? -1 : 0;
        }
    }
    if(isRepeat && file->GetPos() == file->GetSize())
    {
        ReleaseData();
        OpenFile(filePath);
    }
}

void TheoraPlayer::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    UIControl::LoadFromYamlNode(node, loader);
    const YamlNode * fileNode = node->Get("file");
    
	if(fileNode)
        OpenFile(fileNode->AsString());
    
    const YamlNode * rectNode = node->Get("rect");
    
    if(rectNode)
    {
        Rect rect = rectNode->AsRect();
        if(rect.dx == -1)
            rect.dx = (float32)theoraData->thInfo.pic_width;
        if(rect.dy == -1)
            rect.dy = (float32)theoraData->thInfo.pic_height;
        
        SetRect(rect);
    }
}

void TheoraPlayer::Draw(const UIGeometricData &geometricData)
{
    Sprite* sprite = GetSprite();
    if(sprite)
    {
        Sprite::DrawState drawState;
        drawState.SetPosition(geometricData.position);
        RenderSystem2D::Instance()->Draw(sprite, &drawState, Color::White);
    }
}
    
}

#endif //#if !defined(__DAVAENGINE_ANDROID__)
