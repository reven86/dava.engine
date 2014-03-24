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


#include "Render/LibPVRHelper.h"
#include "Render/Texture.h"
#include "Render/RenderManager.h"
#include "Render/OGLHelpers.h"
#include "FileSystem/Logger.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"
#include "Utils/CRC32.h"

#include "Render/Image.h"
#include "Render/ImageLoader.h"


#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)
#include "libpvr/PVRTError.h"
#include "libpvr/PVRTDecompress.h"
#include "libpvr/PVRTMap.h"
#include "libpvr/PVRTextureHeader.h"
#include "libpvr/PVRTexture.h"
#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)

#define METADATA_CRC_SIZE		16			//size for meta data with CRC32
#define METADATA_CRC_TAG		0x5f435243  // equivalent of 'C''R''C''_'


namespace DAVA 
{
	
const uint32 PVRTEX3_METADATAIDENT	= 0x03525650;
    
uint32 LibPVRHelper::GetBitsPerPixel(uint64 pixelFormat)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    if((pixelFormat & PVRTEX_PFHIGHMASK) != 0)
    {
        uint8 *pixelFormatChar = (uint8 *)&pixelFormat;
        return (pixelFormatChar[4] + pixelFormatChar[5] + pixelFormatChar[6] + pixelFormatChar[7]);
    }
    else
    {
        switch (pixelFormat)
        {
            case ePVRTPF_BW1bpp:
                return 1;
            case ePVRTPF_PVRTCI_2bpp_RGB:
            case ePVRTPF_PVRTCI_2bpp_RGBA:
            case ePVRTPF_PVRTCII_2bpp:
                return 2;
            case ePVRTPF_PVRTCI_4bpp_RGB:
            case ePVRTPF_PVRTCI_4bpp_RGBA:
            case ePVRTPF_PVRTCII_4bpp:
            case ePVRTPF_ETC1:
            case ePVRTPF_EAC_R11:
            case ePVRTPF_ETC2_RGB:
            case ePVRTPF_ETC2_RGB_A1:
            case ePVRTPF_DXT1:
            case ePVRTPF_BC4:
                return 4;
            case ePVRTPF_DXT2:
            case ePVRTPF_DXT3:
            case ePVRTPF_DXT4:
            case ePVRTPF_DXT5:
            case ePVRTPF_BC5:
            case ePVRTPF_EAC_RG11:
            case ePVRTPF_ETC2_RGBA:
                return 8;
            case ePVRTPF_YUY2:
            case ePVRTPF_UYVY:
            case ePVRTPF_RGBG8888:
            case ePVRTPF_GRGB8888:
                return 16;
            case ePVRTPF_SharedExponentR9G9B9E5:
                return 32;
            case ePVRTPF_NumCompressedPFs:
                return 0;
        }
    }
    return 0;
#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    return PVRTGetBitsPerPixel(pixelFormat);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
}

    
void LibPVRHelper::GetFormatMinDims(uint64 pixelFormat, uint32 &minX, uint32 &minY, uint32 &minZ)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    switch(pixelFormat)
    {
        case ePVRTPF_DXT1:
        case ePVRTPF_DXT2:
        case ePVRTPF_DXT3:
        case ePVRTPF_DXT4:
        case ePVRTPF_DXT5:
        case ePVRTPF_BC4:
        case ePVRTPF_BC5:
        case ePVRTPF_ETC1:
        case ePVRTPF_ETC2_RGB:
        case ePVRTPF_ETC2_RGBA:
        case ePVRTPF_ETC2_RGB_A1:
        case ePVRTPF_EAC_R11:
        case ePVRTPF_EAC_RG11:
            minX = 4;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCI_4bpp_RGB:
        case ePVRTPF_PVRTCI_4bpp_RGBA:
            minX = 8;
            minY = 8;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCI_2bpp_RGB:
        case ePVRTPF_PVRTCI_2bpp_RGBA:
            minX = 16;
            minY = 8;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCII_4bpp:
            minX = 4;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_PVRTCII_2bpp:
            minX = 8;
            minY = 4;
            minZ = 1;
            break;
        case ePVRTPF_UYVY:
        case ePVRTPF_YUY2:
        case ePVRTPF_RGBG8888:
        case ePVRTPF_GRGB8888:
            minX = 2;
            minY = 1;
            minZ = 1;
            break;
        case ePVRTPF_BW1bpp:
            minX = 8;
            minY = 1;
            minZ = 1;
            break;
        default: //Non-compressed formats all return 1.
            minX = 1;
            minY = 1;
            minZ = 1;
            break;
    }
#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    PVRTGetFormatMinDims(pixelFormat, minX, minY, minZ);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
}

    
uint32 LibPVRHelper::GetTextureDataSize(PVRHeaderV3 textureHeader, int32 mipLevel, bool allSurfaces, bool allFaces)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    //The smallest divisible sizes for a pixel format
    uint32 uiSmallestWidth = 1;
    uint32 uiSmallestHeight = 1;
    uint32 uiSmallestDepth = 1;
    
    uint64 PixelFormatPartHigh = textureHeader.u64PixelFormat & PVRTEX_PFHIGHMASK;
    
    //If the pixel format is compressed, get the pixel format's minimum dimensions.
    if (PixelFormatPartHigh==0)
    {
        GetFormatMinDims(textureHeader.u64PixelFormat, uiSmallestWidth, uiSmallestHeight, uiSmallestDepth);
    }
    
    //Needs to be 64-bit integer to support 16kx16k and higher sizes.
    uint64 uiDataSize = 0;
    if (mipLevel == -1)
    {
        for (uint8 uiCurrentMIP = 0; uiCurrentMIP<textureHeader.u32MIPMapCount; ++uiCurrentMIP)
        {
            //Get the dimensions of the current MIP Map level.
            uint32 uiWidth = PVRT_MAX(1,textureHeader.u32Width>>uiCurrentMIP);
            uint32 uiHeight = PVRT_MAX(1,textureHeader.u32Height>>uiCurrentMIP);
            uint32 uiDepth = PVRT_MAX(1,textureHeader.u32Depth>>uiCurrentMIP);
            
            //If pixel format is compressed, the dimensions need to be padded.
            if (PixelFormatPartHigh==0)
            {
                uiWidth=uiWidth+( (-1*uiWidth)%uiSmallestWidth);
                uiHeight=uiHeight+( (-1*uiHeight)%uiSmallestHeight);
                uiDepth=uiDepth+( (-1*uiDepth)%uiSmallestDepth);
            }
            
            //Add the current MIP Map's data size to the total.
            uiDataSize+=(uint64)GetBitsPerPixel(textureHeader.u64PixelFormat)*(uint64)uiWidth*(uint64)uiHeight*(uint64)uiDepth;
        }
    }
    else
    {
        //Get the dimensions of the specified MIP Map level.
        uint32 uiWidth = PVRT_MAX(1,textureHeader.u32Width>>mipLevel);
        uint32 uiHeight = PVRT_MAX(1,textureHeader.u32Height>>mipLevel);
        uint32 uiDepth = PVRT_MAX(1,textureHeader.u32Depth>>mipLevel);
        
        //If pixel format is compressed, the dimensions need to be padded.
        if (PixelFormatPartHigh==0)
        {
            uiWidth=uiWidth+( (-1*uiWidth)%uiSmallestWidth);
            uiHeight=uiHeight+( (-1*uiHeight)%uiSmallestHeight);
            uiDepth=uiDepth+( (-1*uiDepth)%uiSmallestDepth);
        }
        
        //Work out the specified MIP Map's data size
        uiDataSize=GetBitsPerPixel(textureHeader.u64PixelFormat)*uiWidth*uiHeight*uiDepth;
    }
    
    //The number of faces/surfaces to register the size of.
    uint32 numfaces = ((allFaces)?(textureHeader.u32NumFaces):(1));
    uint32 numsurfs = ((allSurfaces)?(textureHeader.u32NumSurfaces):(1));
    
    //Multiply the data size by number of faces and surfaces specified, and return.
    return (uint32)(uiDataSize/8)*numsurfs*numfaces;

#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    PVRTextureHeaderV3 *header = (PVRTextureHeaderV3 *)&textureHeader;
    return PVRTGetTextureDataSize(*header, mipLevel, allSurfaces, allFaces);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
}
 
void LibPVRHelper::MapLegacyTextureEnumToNewFormat(PVRTPixelType OldFormat, uint64& newType, EPVRTColourSpace& newCSpace, EPVRTVariableType& newChanType, bool& isPreMult)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    //Default value.
    isPreMult=false;
    
    switch (OldFormat)
    {
        case MGLPT_ARGB_4444:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',4,4,4,4);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_ARGB_1555:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',1,5,5,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_RGB_565:
        {
            newType=PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_RGB_555:
        {
            newType=PVRTGENPIXELID4('x','r','g','b',1,5,5,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_RGB_888:
        {
            newType=PVRTGENPIXELID3('r','g','b',8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case MGLPT_ARGB_8888:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case MGLPT_ARGB_8332:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,3,3,2);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_I_8:
        {
            newType=PVRTGENPIXELID1('i',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_AI_88:
        {
            newType=PVRTGENPIXELID2('a','i',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case MGLPT_1_BPP:
        {
            newType=ePVRTPF_BW1bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_VY1UY0:
        {
            newType=ePVRTPF_YUY2;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_Y1VY0U:
        {
            newType=ePVRTPF_UYVY;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_PVRTC2:
        {
            newType=ePVRTPF_PVRTCI_2bpp_RGBA;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case MGLPT_PVRTC4:
        {
            newType=ePVRTPF_PVRTCI_4bpp_RGBA;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_RGBA_4444:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',4,4,4,4);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case OGL_RGBA_5551:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',5,5,5,1);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case OGL_RGBA_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_RGB_565:
        {
            newType=PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case OGL_RGB_555:
        {
            newType=PVRTGENPIXELID4('r','g','b','x',5,5,5,1);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case OGL_RGB_888:
        {
            newType=PVRTGENPIXELID3('r','g','b',8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_I_8:
        {
            newType=PVRTGENPIXELID1('l',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_AI_88:
        {
            newType=PVRTGENPIXELID2('l','a',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_PVRTC2:
        {
            newType=ePVRTPF_PVRTCI_2bpp_RGBA;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_PVRTC4:
        {
            newType=ePVRTPF_PVRTCI_4bpp_RGBA;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_BGRA_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_A_8:
        {
            newType=PVRTGENPIXELID1('a',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_PVRTCII4:
        {
            newType=ePVRTPF_PVRTCII_4bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case OGL_PVRTCII2:
        {
            newType=ePVRTPF_PVRTCII_2bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
//#ifdef _WIN32
//        case D3D_DXT1:
//        {
//            newType=ePVRTPF_DXT1;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            break;
//        }
//            
//        case D3D_DXT2:
//        {
//            newType=ePVRTPF_DXT2;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            isPreMult=true;
//            break;
//        }
//            
//        case D3D_DXT3:
//        {
//            newType=ePVRTPF_DXT3;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            break;
//        }
//            
//        case D3D_DXT4:
//        {
//            newType=ePVRTPF_DXT4;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            isPreMult=true;
//            break;
//        }
//            
//        case D3D_DXT5:
//        {
//            newType=ePVRTPF_DXT5;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedByteNorm;
//            break;
//        }
//            
//#endif
        case D3D_RGB_332:
        {
            newType=PVRTGENPIXELID3('r','g','b',3,3,2);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_AL_44:
        {
            newType=PVRTGENPIXELID2('a','l',4,4);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_LVU_655:
        {
            newType=PVRTGENPIXELID3('l','g','r',6,5,5);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_XLVU_8888:
        {
            newType=PVRTGENPIXELID4('x','l','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_QWVU_8888:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_ABGR_2101010:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',2,10,10,10);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_ARGB_2101010:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',2,10,10,10);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_AWVU_2101010:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',2,10,10,10);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_GR_1616:
        {
            newType=PVRTGENPIXELID2('g','r',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_VU_1616:
        {
            newType=PVRTGENPIXELID2('g','r',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_ABGR_16161616:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_R16F:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_GR_1616F:
        {
            newType=PVRTGENPIXELID2('g','r',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_ABGR_16161616F:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_R32F:
        {
            newType=PVRTGENPIXELID1('r',32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_GR_3232F:
        {
            newType=PVRTGENPIXELID2('g','r',32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case D3D_ABGR_32323232F:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',32,32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case ETC_RGB_4BPP:
        {
            newType=ePVRTPF_ETC1;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case D3D_A8:
        {
            newType=PVRTGENPIXELID1('a',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_V8U8:
        {
            newType=PVRTGENPIXELID2('g','r',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedIntegerNorm;
            break;
        }
            
        case D3D_L16:
        {
            newType=PVRTGENPIXELID1('l',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_L8:
        {
            newType=PVRTGENPIXELID1('l',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_AL_88:
        {
            newType=PVRTGENPIXELID2('a','l',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case D3D_UYVY:
        {
            newType=ePVRTPF_UYVY;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case D3D_YUY2:
        {
            newType=ePVRTPF_YUY2;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R32G32B32A32_FLOAT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R32G32B32A32_UINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R32G32B32A32_SINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',32,32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedInteger;
            break;
        }
            
        case DX10_R32G32B32_FLOAT:
        {
            newType=PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R32G32B32_UINT:
        {
            newType=PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R32G32B32_SINT:
        {
            newType=PVRTGENPIXELID3('r','g','b',32,32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedInteger;
            break;
        }
            
        case DX10_R16G16B16A16_FLOAT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R16G16B16A16_UNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case DX10_R16G16B16A16_UINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShort;
            break;
        }
            
        case DX10_R16G16B16A16_SNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShortNorm;
            break;
        }
            
        case DX10_R16G16B16A16_SINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',16,16,16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShort;
            break;
        }
            
        case DX10_R32G32_FLOAT:
        {
            newType=PVRTGENPIXELID2('r','g',32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R32G32_UINT:
        {
            newType=PVRTGENPIXELID2('r','g',32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R32G32_SINT:
        {
            newType=PVRTGENPIXELID2('r','g',32,32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedInteger;
            break;
        }
            
        case DX10_R10G10B10A2_UNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',10,10,10,2);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
            break;
        }
            
        case DX10_R10G10B10A2_UINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',10,10,10,2);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R11G11B10_FLOAT:
        {
            newType=PVRTGENPIXELID3('r','g','b',11,11,10);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R8G8B8A8_UNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R8G8B8A8_UNORM_SRGB:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R8G8B8A8_UINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByte;
            break;
        }
            
        case DX10_R8G8B8A8_SNORM:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByteNorm;
            break;
        }
            
        case DX10_R8G8B8A8_SINT:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByte;
            break;
        }
            
        case DX10_R16G16_FLOAT:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R16G16_UNORM:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case DX10_R16G16_UINT:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShort;
            break;
        }
            
        case DX10_R16G16_SNORM:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShortNorm;
            break;
        }
            
        case DX10_R16G16_SINT:
        {
            newType=PVRTGENPIXELID2('r','g',16,16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShort;
            break;
        }
            
        case DX10_R32_FLOAT:
        {
            newType=PVRTGENPIXELID1('r',32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R32_UINT:
        {
            newType=PVRTGENPIXELID1('r',32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedInteger;
            break;
        }
            
        case DX10_R32_SINT:
        {
            newType=PVRTGENPIXELID1('r',32);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedInteger;
            break;
        }
            
        case DX10_R8G8_UNORM:
        {
            newType=PVRTGENPIXELID2('r','g',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R8G8_UINT:
        {
            newType=PVRTGENPIXELID2('r','g',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByte;
            break;
        }
            
        case DX10_R8G8_SNORM:
        {
            newType=PVRTGENPIXELID2('r','g',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByteNorm;
            break;
        }
            
        case DX10_R8G8_SINT:
        {
            newType=PVRTGENPIXELID2('r','g',8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByte;
            break;
        }
            
        case DX10_R16_FLOAT:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R16_UNORM:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case DX10_R16_UINT:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedShort;
            break;
        }
            
        case DX10_R16_SNORM:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShortNorm;
            break;
        }
            
        case DX10_R16_SINT:
        {
            newType=PVRTGENPIXELID1('r',16);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedShort;
            break;
        }
            
        case DX10_R8_UNORM:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R8_UINT:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByte;
            break;
        }
            
        case DX10_R8_SNORM:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByteNorm;
            break;
        }
            
        case DX10_R8_SINT:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedByte;
            break;
        }
            
        case DX10_A8_UNORM:
        {
            newType=PVRTGENPIXELID1('r',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R1_UNORM:
        {
            newType=ePVRTPF_BW1bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_R9G9B9E5_SHAREDEXP:
        {
            newType=ePVRTPF_SharedExponentR9G9B9E5;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeSignedFloat;
            break;
        }
            
        case DX10_R8G8_B8G8_UNORM:
        {
            newType=ePVRTPF_RGBG8888;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case DX10_G8R8_G8B8_UNORM:
        {
            newType=ePVRTPF_GRGB8888;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
//#ifdef _WIN32
//        case DX10_BC1_UNORM:
//        {
//            newType=ePVRTPF_DXT1;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC1_UNORM_SRGB:
//        {
//            newType=ePVRTPF_DXT1;
//            newCSpace=ePVRTCSpacesRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC2_UNORM:
//        {
//            newType=ePVRTPF_DXT3;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC2_UNORM_SRGB:
//        {
//            newType=ePVRTPF_DXT3;
//            newCSpace=ePVRTCSpacesRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC3_UNORM:
//        {
//            newType=ePVRTPF_DXT5;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC3_UNORM_SRGB:
//        {
//            newType=ePVRTPF_DXT5;
//            newCSpace=ePVRTCSpacesRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC4_UNORM:
//        {
//            newType=ePVRTPF_BC4;
//            newCSpace=ePVRTCSpacesRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC4_SNORM:
//        {
//            newType=ePVRTPF_BC4;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeSignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC5_UNORM:
//        {
//            newType=ePVRTPF_BC5;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeUnsignedIntegerNorm;
//            break;
//        }
//            
//        case DX10_BC5_SNORM:
//        {
//            newType=ePVRTPF_BC5;
//            newCSpace=ePVRTCSpacelRGB;
//            newChanType=ePVRTVarTypeSignedIntegerNorm;
//            break;
//        }
//            
//#endif
        case ePT_VG_sRGBX_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','x',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sRGBA_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sRGBA_8888_PRE:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sRGB_565:
        {
            newType=PVRTGENPIXELID3('r','g','b',5,6,5);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sRGBA_5551:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',5,5,5,1);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sRGBA_4444:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',4,4,4,4);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sL_8:
        {
            newType=PVRTGENPIXELID1('l',8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lRGBX_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','x',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lRGBA_8888:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lRGBA_8888_PRE:
        {
            newType=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_lL_8:
        {
            newType=PVRTGENPIXELID1('l',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_A_8:
        {
            newType=PVRTGENPIXELID1('a',8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_BW_1:
        {
            newType=ePVRTPF_BW1bpp;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sXRGB_8888:
        {
            newType=PVRTGENPIXELID4('x','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sARGB_8888:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sARGB_8888_PRE:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sARGB_1555:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',1,5,5,5);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sARGB_4444:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',4,4,4,4);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_lXRGB_8888:
        {
            newType=PVRTGENPIXELID4('x','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lARGB_8888:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lARGB_8888_PRE:
        {
            newType=PVRTGENPIXELID4('a','r','g','b',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sBGRX_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','x',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sBGRA_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sBGRA_8888_PRE:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sBGR_565:
        {
            newType=PVRTGENPIXELID3('b','g','r',5,6,5);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sBGRA_5551:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',5,5,5,1);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sBGRA_4444:
        {
            newType=PVRTGENPIXELID4('b','g','r','x',4,4,4,4);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_lBGRX_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','x',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lBGRA_8888:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lBGRA_8888_PRE:
        {
            newType=PVRTGENPIXELID4('b','g','r','a',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sXBGR_8888:
        {
            newType=PVRTGENPIXELID4('x','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sABGR_8888:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_sABGR_8888_PRE:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
            
        case ePT_VG_sABGR_1555:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',1,5,5,5);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_sABGR_4444:
        {
            newType=PVRTGENPIXELID4('x','b','g','r',4,4,4,4);
            newCSpace=ePVRTCSpacesRGB;
            newChanType=ePVRTVarTypeUnsignedShortNorm;
            break;
        }
            
        case ePT_VG_lXBGR_8888:
        {
            newType=PVRTGENPIXELID4('x','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lABGR_8888:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            break;
        }
            
        case ePT_VG_lABGR_8888_PRE:
        {
            newType=PVRTGENPIXELID4('a','b','g','r',8,8,8,8);
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeUnsignedByteNorm;
            isPreMult=true;
            break;
        }
        default:
        {
            newType=ePVRTPF_NumCompressedPFs;
            newCSpace=ePVRTCSpacelRGB;
            newChanType=ePVRTVarTypeNumVarTypes;
            break;
        }
    }
    
#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    PVRTMapLegacyTextureEnumToNewFormat(OldFormat, newType, newCSpace, newChanType, isPreMult);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
}

void LibPVRHelper::ConvertOldTextureHeaderToV3(const PVRHeaderV2* LegacyHeader, PVRHeaderV3& NewHeader)
{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    //Setup variables
    bool isPreMult;
    uint64 ptNew;
    EPVRTColourSpace cSpaceNew;
    EPVRTVariableType chanTypeNew;
    
    //Map the old enum to the new format.
    MapLegacyTextureEnumToNewFormat((PVRTPixelType)(LegacyHeader->dwpfFlags&0xff),ptNew,cSpaceNew,chanTypeNew,isPreMult);
    
    //Check if this is a cube map.
    bool isCubeMap = (LegacyHeader->dwpfFlags&PVRTEX_CUBEMAP)!=0;
    
    //Setup the new header.
    NewHeader.u64PixelFormat=ptNew;
    NewHeader.u32ChannelType=chanTypeNew;
    NewHeader.u32ColourSpace=cSpaceNew;
    NewHeader.u32Depth=1;
    NewHeader.u32Flags=isPreMult?PVRTEX3_PREMULTIPLIED:0;
    NewHeader.u32Height=LegacyHeader->dwHeight;
    NewHeader.u32MetaDataSize=0;
    NewHeader.u32MIPMapCount=(LegacyHeader->dwpfFlags&PVRTEX_MIPMAP?LegacyHeader->dwMipMapCount+1:1); //Legacy headers have a MIP Map count of 0 if there is only the top level. New Headers have a count of 1.
    NewHeader.u32NumFaces=(isCubeMap?6:1);
    
    //Only compute the number of surfaces if it's a V2 header, else default to 1 surface.
    if (LegacyHeader->dwHeaderSize==sizeof(PVRHeaderV2))
        NewHeader.u32NumSurfaces=(LegacyHeader->dwNumSurfs/(isCubeMap?6:1));
    else
        NewHeader.u32NumSurfaces=1;
    
    NewHeader.u32Version=PVRTEX3_IDENT;
    NewHeader.u32Width=LegacyHeader->dwWidth;
    
//    //Clear any currently stored MetaData, or it will be inaccurate.
//    if (pMetaData)
//    {
//        pMetaData->Clear();
//    }
//    
//    //Check if this is a normal map.
//    if (LegacyHeader->dwpfFlags&PVRTEX_BUMPMAP && pMetaData)
//    {
//        //Get a reference to the correct block.
//        MetaDataBlock& mbBumpData=(*pMetaData)[PVRTEX_CURR_IDENT][ePVRTMetaDataBumpData];
//        
//        //Set up the block.
//        mbBumpData.DevFOURCC=PVRTEX_CURR_IDENT;
//        mbBumpData.u32Key=ePVRTMetaDataBumpData;
//        mbBumpData.u32DataSize=8;
//        mbBumpData.Data=new PVRTuint8[8];
//        
//        //Setup the data for the block.
//        float bumpScale = 1.0f;
//        const char* bumpOrder = "xyz";
//        
//        //Copy the bumpScale into the data.
//        memcpy(mbBumpData.Data,&bumpScale,4);
//        
//        //Clear the string
//        memset(mbBumpData.Data+4,0,4);
//        
//        //Copy the bumpOrder into the data.
//        memcpy(mbBumpData.Data+4, bumpOrder,3);
//        
//        //Increment the meta data size.
//        NewHeader.u32MetaDataSize+=(12+mbBumpData.u32DataSize);
//    }
//    
//    //Check if for vertical flip orientation.
//    if (LegacyHeader->dwpfFlags&PVRTEX_VERTICAL_FLIP && pMetaData)
//    {
//        //Get the correct meta data block
//        MetaDataBlock& mbTexOrientation=(*pMetaData)[PVRTEX_CURR_IDENT][ePVRTMetaDataTextureOrientation];
//        
//        //Set the block up.
//        mbTexOrientation.u32DataSize=3;
//        mbTexOrientation.Data=new PVRTuint8[3];
//        mbTexOrientation.DevFOURCC=PVRTEX_CURR_IDENT;
//        mbTexOrientation.u32Key=ePVRTMetaDataTextureOrientation;
//        
//        //Initialise the block to default orientation.
//        memset(mbTexOrientation.Data,0,3);
//        
//        //Set the block oriented upwards.
//        mbTexOrientation.Data[ePVRTAxisY]=ePVRTOrientUp;
//        
//        //Increment the meta data size.
//        NewHeader.u32MetaDataSize+=(12+mbTexOrientation.u32DataSize);
//    }

#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    PVRTConvertOldTextureHeaderToV3((const PVR_Texture_Header *)LegacyHeader, (PVRTextureHeaderV3&)NewHeader, NULL);
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
}

    

const PixelFormat LibPVRHelper::GetCompressedFormat(const uint64 pixelFormat)
{
    switch (pixelFormat)
    {
        case ePVRTPF_PVRTCI_2bpp_RGB:
        {
            return FORMAT_PVR2;
        }
        case ePVRTPF_PVRTCI_2bpp_RGBA:
        {
            return FORMAT_PVR2;
            break;
        }
        case ePVRTPF_PVRTCI_4bpp_RGB:
        {
            return FORMAT_PVR4;
        }
        case ePVRTPF_PVRTCI_4bpp_RGBA:
        {
            return FORMAT_PVR4;
        }
        case ePVRTPF_ETC1:
        {
            return FORMAT_ETC1;
        }
            
        default:
            break;
    }
    
    return FORMAT_INVALID;
}
    
const PixelFormat LibPVRHelper::GetFloatTypeFormat(const uint64 pixelFormat)
{
    switch (pixelFormat)
    {
        case PVRTGENPIXELID4('r','g','b','a',16,16,16,16):
        {
            return FORMAT_RGBA16161616;
        }
        case PVRTGENPIXELID3('r','g','b',16,16,16):
        {
            return FORMAT_INVALID;
        }
        case PVRTGENPIXELID2('l','a',16,16):
        {
            return FORMAT_INVALID;
        }
        case PVRTGENPIXELID1('l',16):
        {
            return FORMAT_INVALID;
        }
        case PVRTGENPIXELID1('a',16):
        {
            return FORMAT_A16;
        }
        case PVRTGENPIXELID4('r','g','b','a',32,32,32,32):
        {
            return FORMAT_RGBA32323232;
        }
        case PVRTGENPIXELID3('r','g','b',32,32,32):
        {
            return FORMAT_INVALID;
        }
        case PVRTGENPIXELID2('l','a',32,32):
        {
            return FORMAT_INVALID;
        }
        case PVRTGENPIXELID1('l',32):
        {
            return FORMAT_INVALID;
        }
        case PVRTGENPIXELID1('a',32):
        {
            return FORMAT_INVALID;
        }
    }
    
    return FORMAT_INVALID;
}

const PixelFormat LibPVRHelper::GetUnsignedByteFormat(const uint64 pixelFormat)
{
    switch (pixelFormat)
    {
        case PVRTGENPIXELID4('r','g','b','a',8,8,8,8):
        {
            return FORMAT_RGBA8888;
        }
        case PVRTGENPIXELID3('r','g','b',8,8,8):
        {
            return FORMAT_RGB888;
        }
        case PVRTGENPIXELID2('l','a',8,8):
        {
            return FORMAT_INVALID;
        }
        case PVRTGENPIXELID1('l',8):
        {
            return FORMAT_A8;
        }
        case PVRTGENPIXELID1('a',8):
        {
            return FORMAT_A8;
        }
        case PVRTGENPIXELID4('b','g','r','a',8,8,8,8):
        {
            return FORMAT_INVALID;
        }
    }

    return FORMAT_INVALID;
}
    
const PixelFormat LibPVRHelper::GetUnsignedShortFormat(const uint64 pixelFormat)
{
    switch (pixelFormat)
    {
        case PVRTGENPIXELID4('r','g','b','a',4,4,4,4):
        {
            return FORMAT_RGBA4444;
        }
        case PVRTGENPIXELID4('r','g','b','a',5,5,5,1):
        {
            return FORMAT_RGBA5551;
        }
        case PVRTGENPIXELID3('r','g','b',5,6,5):
        {
            return FORMAT_RGB565;
        }
    }
    
    return FORMAT_INVALID;
}
    
const PixelFormat LibPVRHelper::GetTextureFormat(const PVRHeaderV3& textureHeader)
{
    uint64 pixelFormat = textureHeader.u64PixelFormat;
    EPVRTVariableType ChannelType = (EPVRTVariableType)textureHeader.u32ChannelType;
    
    //Get the last 32 bits of the pixel format.
    uint64 PixelFormatPartHigh = pixelFormat&PVRTEX_PFHIGHMASK;
    
    //Check for a compressed format (The first 8 bytes will be 0, so the whole thing will be equal to the last 32 bits).
    PixelFormatDescriptor formatDescriptor;
    if (PixelFormatPartHigh==0)
    {
        //Format and type == 0 for compressed textures.
        return GetCompressedFormat(pixelFormat);
    }
    else
    {
        switch (ChannelType)
        {
            case ePVRTVarTypeFloat:
            {
                return GetFloatTypeFormat(pixelFormat);
            }
            case ePVRTVarTypeUnsignedByteNorm:
            {
                return GetUnsignedByteFormat(pixelFormat);
            }
            case ePVRTVarTypeUnsignedShortNorm:
            {
                return GetUnsignedShortFormat(pixelFormat);
            }
            default:
                break;
        }
    }
    
    return FORMAT_INVALID;
}
	
bool LibPVRHelper::ReadMipMapLevel(const char* pvrData, const int32 pvrDataSize, const Vector<Image*>& images, uint32 mipMapLevel)
{
    //Texture setup
    PVRHeaderV3 compressedHeader;
    uint8* pTextureData=NULL;
    
    //Check if it's an old header format
    if((*(uint32*)pvrData)!=PVRTEX3_IDENT)
    {
        ConvertOldTextureHeaderToV3((PVRHeaderV2 *)pvrData, compressedHeader);
        
        //Get the texture data.
        pTextureData = (uint8*)pvrData + *(uint32*)pvrData;
    }
    else
    {
        //Get the header from the main pointer.
        compressedHeader =* (PVRHeaderV3*)pvrData;
        
        //Get the texture data.
        pTextureData = (uint8*)pvrData+PVRTEX3_HEADERSIZE + compressedHeader.u32MetaDataSize;
    }
    
    //Get the OGLES format values.
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();
    PixelFormatDescriptor formatDescriptor = Texture::GetPixelFormatDescriptor(GetTextureFormat(compressedHeader));
    if(!IsFormatSupported(formatDescriptor))
    {
        Logger::Error("[LibPVRHelper::ReadMipMapLevel] Unsupported format");
        return false;
    }
	
	uint32 cubemapLayout = LibPVRHelper::GetCubemapLayout(&compressedHeader, pvrData, pvrDataSize);
	
	bool result = true;
	for(uint32 faceIndex = 0; faceIndex < compressedHeader.u32NumFaces; ++faceIndex)
    {
		Image* image = images[mipMapLevel * compressedHeader.u32NumFaces + faceIndex];
		
		image->width = PVRT_MAX(1, compressedHeader.u32Width >> mipMapLevel);
		image->height = PVRT_MAX(1, compressedHeader.u32Height >> mipMapLevel);
		image->format = formatDescriptor.formatID;
		image->mipmapLevel = mipMapLevel;
		
		if(cubemapLayout != 0)
		{
			image->cubeFaceID = (cubemapLayout & (0x0000000F << (faceIndex * 4))) >> (faceIndex * 4);
		}
	
		//Check for compressed formats
		if((FORMAT_PVR4 == formatDescriptor.formatID) || (FORMAT_PVR2 == formatDescriptor.formatID))
		{
			//Check for PVRTCI support.
			if(deviceCaps.isPVRTCSupported)
			{
				bool copied = CopyToImage(image, mipMapLevel, faceIndex, compressedHeader, pTextureData);
				if(!copied)
				{
					result = false;
					break;
				}
			}
			else
			{
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
				DVASSERT(false && "Must be hardware supported");
				return false;
#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
				//Create a near-identical texture header for the decompressed header.
				PVRHeaderV3 decompressedHeader = CreateDecompressedHeader(compressedHeader);
				if(!AllocateImageData(image, mipMapLevel, decompressedHeader))
				{
					result = false;
					break;
				}
            
				//Setup temporary variables.
				uint8* pTempDecompData = image->data;
				uint8* pTempCompData = (uint8*)pTextureData + GetMipMapLayerOffset(mipMapLevel, faceIndex, compressedHeader);
			
				//Get the face offset. Varies per MIP level.
//				uint32 decompressedFaceOffset = GetTextureDataSize(decompressedHeader, mipMapLevel, false, false);
//				uint32 compressedFaceOffset = GetTextureDataSize(compressedHeader, mipMapLevel, false, false);
				//for (uint32 uiFace=0;uiFace<compressedHeader.u32NumFaces;++uiFace)
				//{
				PVRTDecompressPVRTC(pTempCompData, (FORMAT_PVR2 == formatDescriptor.formatID) ? 1 : 0, image->width, image->height, pTempDecompData);
				//Move forward through the pointers.
//				pTempDecompData+=decompressedFaceOffset;
//				pTempCompData+=compressedFaceOffset;
				//}
				image->format = FORMAT_RGBA8888;
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
			}
		}
#if !defined(__DAVAENGINE_IPHONE__)
		else if (FORMAT_ETC1 == formatDescriptor.formatID)
		{
			if(deviceCaps.isETCSupported)
			{
				bool copied = CopyToImage(image, mipMapLevel, faceIndex, compressedHeader, pTextureData);
				if(!copied)
				{
					result = false;
					break;
				}
			}
			else
			{
				//Create a near-identical texture header for the decompressed header.
#if defined (__DAVAENGINE_ANDROID__)
				DVASSERT(!"Must be hardware supported");
				return false;
#else
				PVRHeaderV3 decompressedHeader = CreateDecompressedHeader(compressedHeader);
				if(!AllocateImageData(image, mipMapLevel, decompressedHeader))
				{
					result = false;
					break;
				}
			
				//Setup temporary variables.
				uint8* pTempDecompData = (uint8*)image->data;
				uint8* pTempCompData = (uint8*)pTextureData + GetMipMapLayerOffset(mipMapLevel, faceIndex, compressedHeader);
			
				//Get the face offset. Varies per MIP level.
//				uint32 decompressedFaceOffset = GetTextureDataSize(decompressedHeader, mipMapLevel, false, false);
//				uint32 compressedFaceOffset = GetTextureDataSize(compressedHeader, mipMapLevel, false, false);
				//for (uint32 uiFace=0;uiFace<compressedHeader.u32NumFaces;++uiFace)
				//{
				PVRTDecompressETC(pTempCompData, image->width, image->height, pTempDecompData, 0);
                
				//Move forward through the pointers.
//				pTempDecompData += decompressedFaceOffset;
//				pTempCompData += compressedFaceOffset;
				//}
				image->format = FORMAT_RGBA8888;
#endif //defined (__DAVAENGINE_ANDROID__)
			}
		}
#endif //#if !defined(__DAVAENGINE_IPHONE__)
		else
		{
			bool copied = CopyToImage(image, mipMapLevel, faceIndex, compressedHeader, pTextureData);
			if(!copied)
			{
				result = false;
				break;
			}
		}
	}

	return result;
}
    
/*bool LibPVRHelper::ReadMipMapLevel(const char* pvrData, const int32 pvrDataSize, Image *image, uint32 mipMapLevel)
{
    bool bIsLegacyPVR=false;
    
    //Texture setup
    PVRHeaderV3 compressedHeader;
    uint8* pTextureData=NULL;
    
    //Check if it's an old header format
    if((*(uint32*)pvrData)!=PVRTEX3_IDENT)
    {
        ConvertOldTextureHeaderToV3((PVRHeaderV2 *)pvrData, compressedHeader);
        
        //Get the texture data.
        pTextureData = (uint8*)pvrData + *(uint32*)pvrData;
        
        bIsLegacyPVR=true;
    }
    else
    {
        //Get the header from the main pointer.
        compressedHeader =* (PVRHeaderV3*)pvrData;
        
        //Get the texture data.
        pTextureData = (uint8*)pvrData+PVRTEX3_HEADERSIZE + compressedHeader.u32MetaDataSize;
    }
    
    //Get the OGLES format values.
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();
    PixelFormatDescriptor formatDescriptor = Texture::GetPixelFormatDescriptor(GetTextureFormat(compressedHeader));
    if(!IsFormatSupported(formatDescriptor))
    {
        Logger::Error("[LibPVRHelper::ReadMipMapLevel] Unsupported format");
        return false;
    }
    
    image->width = PVRT_MAX(1, compressedHeader.u32Width >> mipMapLevel);
    image->height = PVRT_MAX(1, compressedHeader.u32Height >> mipMapLevel);
    image->format = formatDescriptor.formatID;

    //Check for compressed formats
    if((FORMAT_PVR4 == formatDescriptor.formatID) || (FORMAT_PVR2 == formatDescriptor.formatID))
    {
        //Check for PVRTCI support.
        if(deviceCaps.isPVRTCSupported)
        {
            bool copied = CopyToImage(image, mipMapLevel, 0, compressedHeader, pTextureData);
            if(!copied)
            {
                return false;
            }
        }
        else
        {
#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
			DVASSERT(false && "Must be hardware supported");
			return false;
#else //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
            //Create a near-identical texture header for the decompressed header.
            PVRHeaderV3 decompressedHeader = CreateDecompressedHeader(compressedHeader);
            if(!AllocateImageData(image, mipMapLevel, decompressedHeader))
            {
                return false;
            }
            
            //Setup temporary variables.
            uint8* pTempDecompData = image->data;
            uint8* pTempCompData = (uint8*)pTextureData + GetMipMapLayerOffset(mipMapLevel, compressedHeader);

            //Get the face offset. Varies per MIP level.
            uint32 decompressedFaceOffset = GetTextureDataSize(decompressedHeader, mipMapLevel, false, false);
            uint32 compressedFaceOffset = GetTextureDataSize(compressedHeader, mipMapLevel, false, false);
            for (uint32 uiFace=0;uiFace<compressedHeader.u32NumFaces;++uiFace)
            {
                PVRTDecompressPVRTC(pTempCompData, (FORMAT_PVR2 == formatDescriptor.formatID) ? 1 : 0, image->width, image->height, pTempDecompData);
                //Move forward through the pointers.
                pTempDecompData+=decompressedFaceOffset;
                pTempCompData+=compressedFaceOffset;
            }
            image->format = FORMAT_RGBA8888;
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)			
        }
    }
#if !defined(__DAVAENGINE_IPHONE__)
    else if (FORMAT_ETC1 == formatDescriptor.formatID)
    {
        if(deviceCaps.isETCSupported)
        {
            bool copied = CopyToImage(image, mipMapLevel, 0, compressedHeader, pTextureData);
            if(!copied)
            {
                return false;
            }
        }
        else
        {
            //Create a near-identical texture header for the decompressed header.
#if defined (__DAVAENGINE_ANDROID__)
			DVASSERT(!"Must be hardware supported");
			return false;
#else
            PVRHeaderV3 decompressedHeader = CreateDecompressedHeader(compressedHeader);
            if(!AllocateImageData(image, mipMapLevel, decompressedHeader))
            {
                return false;
            }

            //Setup temporary variables.
            uint8* pTempDecompData = (uint8*)image->data;
            uint8* pTempCompData = (uint8*)pTextureData + GetMipMapLayerOffset(mipMapLevel, compressedHeader);

            //Get the face offset. Varies per MIP level.
            uint32 decompressedFaceOffset = GetTextureDataSize(decompressedHeader, mipMapLevel, false, false);
            uint32 compressedFaceOffset = GetTextureDataSize(compressedHeader, mipMapLevel, false, false);
            for (uint32 uiFace=0;uiFace<compressedHeader.u32NumFaces;++uiFace)
            {
                PVRTDecompressETC(pTempCompData, image->width, image->height, pTempDecompData, 0);
                
                //Move forward through the pointers.
                pTempDecompData += decompressedFaceOffset;
                pTempCompData += compressedFaceOffset;
            }
            image->format = FORMAT_RGBA8888;
#endif //defined (__DAVAENGINE_ANDROID__)
        }
    }
#endif //#if !defined(__DAVAENGINE_IPHONE__)
    else
    {
        bool copied = CopyToImage(image, mipMapLevel, 0, compressedHeader, pTextureData);
        if(!copied)
        {
            return false;
        }
    }
    return true;
}
*/

bool LibPVRHelper::CopyToImage(Image *image, uint32 mipMapLevel, uint32 faceIndex, const PVRHeaderV3 &header, const uint8 *pvrData)
{
    if(AllocateImageData(image, mipMapLevel, header))
    {
        //Setup temporary variables.
        uint8* data = (uint8*)pvrData + GetMipMapLayerOffset(mipMapLevel, faceIndex, header);
        Memcpy(image->data, data, image->dataSize * sizeof(uint8));

        return true;
    }
    
    return false;
}
    
bool LibPVRHelper::AllocateImageData(DAVA::Image *image, uint32 mipMapLevel, const DAVA::PVRHeaderV3 &header)
{
    image->dataSize = GetTextureDataSize(header, mipMapLevel, false, (header.u32NumFaces == 1));
    image->data = new uint8[image->dataSize];
    if(!image->data)
    {
        Logger::Error("[LibPVRHelper::AllocateImageData] Unable to allocate memory to compressed texture.\n");
        return false;
    }
    
    return true;
}
    
int32 LibPVRHelper::GetMipMapLayerOffset(uint32 mipMapLevel, uint32 faceIndex, const PVRHeaderV3 &header)
{
    int32 offset = 0;
    for (uint32 uiMIPMap=0;uiMIPMap<mipMapLevel;++uiMIPMap)
    {
        offset += GetTextureDataSize(header, uiMIPMap, false, true);
    }
    return offset + faceIndex * GetTextureDataSize(header, mipMapLevel, false, false);
}


PVRHeaderV3 LibPVRHelper::CreateDecompressedHeader(const PVRHeaderV3 &compressedHeader)
{
    PVRHeaderV3 decompressedHeader = compressedHeader;
    decompressedHeader.u32ChannelType=ePVRTVarTypeUnsignedByteNorm;
    decompressedHeader.u32ColourSpace=ePVRTCSpacelRGB;
    decompressedHeader.u64PixelFormat=PVRTGENPIXELID4('r','g','b','a',8,8,8,8);

    return decompressedHeader;
}
   
    
bool LibPVRHelper::PreparePVRData(const char* pvrData, const int32 pvrDataSize)
{
    PVRHeaderV3 textureHeader;
    //Header size.
    uint32 u32HeaderSize=0;
    
    //Boolean whether to byte swap the texture data or not.
    bool bSwapDataEndianness=false;
    
    //The channel type for endian swapping.
    EPVRTVariableType u32CurrentChannelType=ePVRTVarTypeUnsignedByte;
    
    //Check the first word of the file and see if it's equal to the current identifier (or reverse identifier)
    if(*(uint32*)pvrData!=PVRTEX_CURR_IDENT && *(uint32*)pvrData!=PVRTEX_CURR_IDENT_REV)
    {
        //Swap the header bytes if necessary.
        if(!PVRTIsLittleEndian())
        {
            bSwapDataEndianness=true;
            uint32 u32HeaderSize=PVRTByteSwap32(*(uint32*)pvrData);
            
            for (uint32 i=0; i<u32HeaderSize; ++i)
            {
                PVRTByteSwap( (uint8*)( ( (uint32*)pvrData )+i),sizeof(uint32) );
            }
        }
        
        //Get a pointer to the header.
        PVRHeaderV2* sLegacyTextureHeader=(PVRHeaderV2*)pvrData;
        
        //Set the header size.
        u32HeaderSize=sLegacyTextureHeader->dwHeaderSize;
        
        //We only really need the channel type.
        uint64 tempFormat;
        EPVRTColourSpace tempColourSpace;
        bool tempIsPreMult;
        
        //Map the enum to get the channel type.
        MapLegacyTextureEnumToNewFormat( (PVRTPixelType)( sLegacyTextureHeader->dwpfFlags&0xff),tempFormat,tempColourSpace, u32CurrentChannelType, tempIsPreMult);
    }
    else
    {
        // If the header file has a reverse identifier, then we need to swap endianness
        if(*(uint32*)pvrData==PVRTEX_CURR_IDENT_REV)
        {
            bSwapDataEndianness=true;
            PVRHeaderV3* pTextureHeader=(PVRHeaderV3*)pvrData;
            
            pTextureHeader->u32ChannelType=PVRTByteSwap32(pTextureHeader->u32ChannelType);
            pTextureHeader->u32ColourSpace=PVRTByteSwap32(pTextureHeader->u32ColourSpace);
            pTextureHeader->u32Depth=PVRTByteSwap32(pTextureHeader->u32Depth);
            pTextureHeader->u32Flags=PVRTByteSwap32(pTextureHeader->u32Flags);
            pTextureHeader->u32Height=PVRTByteSwap32(pTextureHeader->u32Height);
            pTextureHeader->u32MetaDataSize=PVRTByteSwap32(pTextureHeader->u32MetaDataSize);
            pTextureHeader->u32MIPMapCount=PVRTByteSwap32(pTextureHeader->u32MIPMapCount);
            pTextureHeader->u32NumFaces=PVRTByteSwap32(pTextureHeader->u32NumFaces);
            pTextureHeader->u32NumSurfaces=PVRTByteSwap32(pTextureHeader->u32NumSurfaces);
            pTextureHeader->u32Version=PVRTByteSwap32(pTextureHeader->u32Version);
            pTextureHeader->u32Width=PVRTByteSwap32(pTextureHeader->u32Width);
            PVRTByteSwap((uint8*)&pTextureHeader->u64PixelFormat,sizeof(uint64));
            
            //Channel type.
            u32CurrentChannelType=(EPVRTVariableType)pTextureHeader->u32ChannelType;
        }
        
        //Setup the texture header
        textureHeader = *(PVRHeaderV3*)pvrData;
        //Header size.
        u32HeaderSize=PVRTEX3_HEADERSIZE+textureHeader.u32MetaDataSize;
    }

    // Convert the data if needed
    if(bSwapDataEndianness)
    {
        //Get the size of the variables types.
        uint32 ui32VariableSize=0;
        switch(u32CurrentChannelType)
        {
            case ePVRTVarTypeFloat:
            case ePVRTVarTypeUnsignedInteger:
            case ePVRTVarTypeUnsignedIntegerNorm:
            case ePVRTVarTypeSignedInteger:
            case ePVRTVarTypeSignedIntegerNorm:
            {
                ui32VariableSize=4;
                break;
            }
            case ePVRTVarTypeUnsignedShort:
            case ePVRTVarTypeUnsignedShortNorm:
            case ePVRTVarTypeSignedShort:
            case ePVRTVarTypeSignedShortNorm:
            {
                ui32VariableSize=2;
                break;
            }
            case ePVRTVarTypeUnsignedByte:
            case ePVRTVarTypeUnsignedByteNorm:
            case ePVRTVarTypeSignedByte:
            case ePVRTVarTypeSignedByteNorm:
            {
                ui32VariableSize=1;
                break;
            }
            default:
                break;
        }
        
        //If the size of the variable type is greater than 1, then we need to byte swap.
        if (ui32VariableSize>1)
        {
            //Get the texture data.
            uint8* pu8OrigData = ( (uint8*)pvrData + u32HeaderSize);

            //Get the size of the texture data.
            uint32 ui32TextureDataSize = GetTextureDataSize(textureHeader);

            //Loop through and byte swap all the data. It's swapped in place so no need to do anything special.
            for(uint32 i = 0; i < ui32TextureDataSize; i+=ui32VariableSize)
            {
                PVRTByteSwap(pu8OrigData+i,ui32VariableSize);
            }
        }
    }
    
    return true;
}

PixelFormat LibPVRHelper::GetPixelFormat(const FilePath &filePathname)
{
    PVRHeaderV3 header = GetHeader(filePathname);
    return GetTextureFormat(header);
}
    
uint32 LibPVRHelper::GetDataSize(const FilePath &filePathname)
{
    PVRHeaderV3 header = GetHeader(filePathname);
    return GetTextureDataSize(header);
}
	
bool LibPVRHelper::AddCRCIntoMetaData(const FilePath &filePathname)
{
	String fileNameStr = filePathname.GetAbsolutePathname();

	PVRHeaderV3 header = GetHeader(filePathname);
	if(header.u32Version != PVRTEX3_METADATAIDENT)
	{
		Logger::Error("[LibPVRHelper::AddCRCIntoMetaData]  %s has wrong version", fileNameStr.c_str());
		return false;
	}
	
	uint32 presentCRC = 0;
	if(GetCRCFromMetaData(filePathname, &presentCRC))
	{
		Logger::Error("[LibPVRHelper::AddCRCIntoMetaData] CRC is already added into file %s", fileNameStr.c_str());
		return false;
	}
	
	File *fileRead = File::Create(filePathname, File::READ | File::OPEN);
	if(!fileRead)
	{
		Logger::Error("[LibPVRHelper::AddCRCIntoMetaData] cannot open file %s", fileNameStr.c_str());
		return false;
	}

    uint32 originalFileSize = fileRead->GetSize();
    //create single buffer for incresed data amount
    uint32 newFileSize = originalFileSize + METADATA_CRC_SIZE;
    char *fileBuffer = new char[newFileSize];
	if(!fileBuffer)
	{
		Logger::Error("[LibPVRHelper::AddCRCIntoMetaData]: cannot allocate buffer for file data");
		SafeRelease(fileRead);
		return false;
	}
    
    uint32 readSize = fileRead->Read(fileBuffer, originalFileSize);
    if(readSize != originalFileSize)
    {
        Logger::Error("[LibPVRHelper::AddCRCIntoMetaData]: cannot read from file %s", fileNameStr.c_str());
        SafeRelease(fileRead);
        SafeDeleteArray(fileBuffer);
        return false;
    }
    SafeRelease(fileRead);
    //calculate for only existed part of buffer(fileSize)
    uint32 newCRC = CRC32::ForBuffer(fileBuffer, originalFileSize);
    uint32 textureDataSize = originalFileSize - sizeof(header) - header.u32MetaDataSize;
    char* currentTextureDataPointer = fileBuffer + sizeof(header) + header.u32MetaDataSize;
    //shift texture data to get space for new crc metadata
    memmove((currentTextureDataPointer + METADATA_CRC_SIZE), currentTextureDataPointer, textureDataSize);
    
    uint32 metadata[4] = {METADATA_CRC_TAG, 0, 0, newCRC};
    metadata[2] = sizeof(metadata);
    memcpy(fileBuffer + sizeof(header) + header.u32MetaDataSize, &metadata, metadata[2]);
    
    uint32* metaDataSizePointer = (uint32*)(fileBuffer + sizeof(header) - sizeof(header.u32MetaDataSize))	;
    *metaDataSizePointer = header.u32MetaDataSize + METADATA_CRC_SIZE;
    
    FilePath filePathnameTmp(filePathname.GetAbsolutePathname() + "_");
	File *fileWrite = File::Create(filePathnameTmp, File::WRITE | File::CREATE);
    if(!fileWrite)
    {
        SafeDeleteArray(fileBuffer);
        Logger::Error("[LibPVRHelper::AddCRCIntoMetaData] cannot open file %s", filePathnameTmp.GetAbsolutePathname().c_str());
        return false;
    }
    
    bool writeSucces = fileWrite->Write(fileBuffer, newFileSize) == newFileSize;
    SafeDeleteArray(fileBuffer);
    SafeRelease(fileWrite);
    if(writeSucces)
	{
		FileSystem::Instance()->DeleteFile(filePathname);
		FileSystem::Instance()->MoveFile(filePathnameTmp, filePathname, true);
		return true;
	}
    else
    {
        Logger::Error("[LibPVRHelper::AddCRCIntoMetaData]: cannot write to file %s",
                      filePathnameTmp.GetAbsolutePathname().c_str());
        FileSystem::Instance()->DeleteFile(filePathnameTmp);
        return false;
    }
}

bool LibPVRHelper::GetCRCFromMetaData(const FilePath &filePathname, uint32* outputCRC)
{
	bool retValue = false;
	String fileNameStr = filePathname.GetAbsolutePathname();

	PVRHeaderV3 header = GetHeader(filePathname);
	if(header.u32Version != PVRTEX3_METADATAIDENT)
	{
		Logger::Error("[LibPVRHelper::GetCRCFromFile]  %s has wrong version", fileNameStr.c_str());
		return retValue;
	}
	
	File *fileRead = File::Create(filePathname, File::READ | File::OPEN);
	if(!fileRead)
	{
		Logger::Error("[LibPVRHelper::GetCRCFromFile] cannot open file %s", fileNameStr.c_str());
		return retValue;
	}

	if(header.u32MetaDataSize != 0)
	{
		uint32 crc = 0, readSize = 0;
		fileRead->Seek(sizeof(header), File::SEEK_FROM_START);
		while (crc == 0 && readSize != header.u32MetaDataSize)
		{
			readSize += ReadNextMetadata(fileRead, &crc);
		}
		
		if(crc != 0)
		{
			*outputCRC = crc;
			retValue = true;
		}
	}

	SafeRelease(fileRead);
	return retValue;
}

uint32 LibPVRHelper::ReadNextMetadata(DAVA::File* file, uint32* outputCRC)
{
	*outputCRC = 0;
	uint32 readSize = 0, fourCC = 0,key = 0, dataSize = 0, data = 0;
	
	readSize = file->Read(&fourCC, sizeof(fourCC));
	readSize += file->Read(&key, sizeof(key));
	readSize += file->Read(&dataSize, sizeof(dataSize));
	if(fourCC == METADATA_CRC_TAG)
	{
		readSize += file->Read(&data, sizeof(data));
		*outputCRC = data;
	}
	else
	{
		readSize += dataSize;
		file->Seek(file->GetPos() + dataSize, File::SEEK_FROM_START);
	}
	
	return readSize;
}
	
uint32 LibPVRHelper::GetCRCFromFile(const FilePath &filePathname)
{
	uint32 crc = 0;
	bool success = GetCRCFromMetaData(filePathname, &crc);
	return success ? crc : CRC32::ForFile(filePathname);
}

PVRHeaderV3 LibPVRHelper::GetHeader(const FilePath &filePathname)
{
    File *file = File::Create(filePathname, File::READ | File::OPEN);
    if(!file)
    {
        Logger::Error("[LibPVRHelper::GetHeaderForFile] cannot open file %s", filePathname.GetAbsolutePathname().c_str());
        return PVRHeaderV3();
    }
    
    PVRHeaderV3 header = GetHeader(file);
    SafeRelease(file);
    return header;
}
    
PVRHeaderV3 LibPVRHelper::GetHeader(File *file)
{
    PVRHeaderV3 header;
    uint8 *headerData = new uint8[PVRTEX3_HEADERSIZE];
    if(headerData)
    {
        int32 readSize = file->Read(headerData, PVRTEX3_HEADERSIZE);
        if(PVRTEX3_HEADERSIZE == readSize)
        {
            header = GetHeader(headerData, PVRTEX3_HEADERSIZE);
        }
        SafeDeleteArray(headerData);
    }
    return header;
}


PVRHeaderV3 LibPVRHelper::GetHeader(const uint8* pvrData, const int32 pvrDataSize)
{
    bool dataPrepared = PreparePVRData((const char *)pvrData, pvrDataSize);
    if(dataPrepared)
    {
        PVRHeaderV3 header;
        //Check if it's an old header format
        if((*(uint32*)pvrData)!=PVRTEX3_IDENT)
        {
            PVRHeaderV2 *pvrV2Header = (PVRHeaderV2 *)pvrData;
            if(pvrV2Header->dwPVR == PVRTEX2_IDENT)
            {
                ConvertOldTextureHeaderToV3((PVRHeaderV2 *)pvrData, header);
            }
        }
        else
        {
            //Get the header from the main pointer.
            header = *(PVRHeaderV3*)pvrData;
        }
        return header;
    }
    
    return PVRHeaderV3();
}

bool LibPVRHelper::IsFormatSupported(const PixelFormatDescriptor &format)
{
    if(FORMAT_INVALID == format.formatID)
    {
        Logger::Error("[LibPVRHelper::IsFormatSupported] FORMAT_INVALID");
        return false;
    }
    
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();
    if ((FORMAT_RGBA16161616 == format.formatID) && !deviceCaps.isFloat16Supported)
    {
        Logger::Error("[LibPVRHelper::IsFormatSupported] FORMAT_RGBA16161616 is unsupported");
        return false;
    }
    
    if ((FORMAT_RGBA32323232 == format.formatID) && !deviceCaps.isFloat32Supported)
    {
        Logger::Error("[LibPVRHelper::IsFormatSupported] FORMAT_RGBA32323232 is unsupported");
        return false;
    }
    
    return true;
}

bool LibPVRHelper::IsPvrFile(DAVA::File *file)
{
	file->Seek(0, File::SEEK_FROM_START);
    PVRHeaderV3 header = GetHeader(file);
    return (PVRTEX3_IDENT == header.u32Version);
}

uint32 LibPVRHelper::GetMipMapLevelsCount(File *file)
{
	file->Seek(0, File::SEEK_FROM_START);
    PVRHeaderV3 header = GetHeader(file);
    return header.u32MIPMapCount;
}
	
uint32 LibPVRHelper::GetCubemapFaceCount(File* file)
{
	file->Seek(0, File::SEEK_FROM_START);
	PVRHeaderV3 header = GetHeader(file);
	return header.u32NumFaces;
}
    
bool LibPVRHelper::ReadFile(File *file, const Vector<Image *> &imageSet)
{
    uint32 fileSize = file->GetSize();
    uint8 *fileData = new uint8[fileSize];
    if(!fileData)
    {
        Logger::Error("[LibPVRHelper::ReadFile]: cannot allocate buffer for file data");
        return false;
    }
    
    uint32 readSize = file->Read(fileData, fileSize);
    if(readSize != fileSize)
    {
        Logger::Error("[LibPVRHelper::ReadFile]: cannot read from file");
        
        SafeDeleteArray(fileData);
        return false;
    }
    
    
    bool preloaded = LibPVRHelper::PreparePVRData((const char *)fileData, fileSize);
    if(!preloaded)
    {
        Logger::Error("[LibPVRHelper::ReadFile]: cannot prepare pvr data for parsing");
        SafeDeleteArray(fileData);
        return false;
    }

    bool read = true;
	uint32 mipmapLevelCount = LibPVRHelper::GetMipMapLevelsCount(file);
    for (uint32 i = 0; i < mipmapLevelCount; ++i)
    {
        read &= ReadMipMapLevel((const char *)fileData, fileSize, imageSet, i);
    }
    
    SafeDeleteArray(fileData);
    return read;
}

uint32 LibPVRHelper::GetCubemapLayout(PVRHeaderV3* pvrHeader, const char* pvrData, const int32 pvrDataSize)
{
	uint32 layout = 0;
	uint32 dataSize;
	const char* metadata = GetCubemapMetadata(pvrHeader, pvrData, pvrDataSize, &dataSize);
	if(NULL != metadata)
	{
		/*
		 The 6 bytes represent a six character string; 
		 this string shows the order the cube map faces are stored in the texture data, 
		 for example ‘XxYyZz’. Uppercase letters refer to a positive axis position; 
		 while lowercase refer to a negative axis position. 
		 Not all axes must be present.
		 */
		uint32 index = 0;
		while(index < dataSize &&
			  metadata[index] != '\0')
		{
			switch(metadata[index])
			{
				case 'X':
				{
					layout = layout | (Texture::CUBE_FACE_POSITIVE_X << (index * 4));
				}
					break;
					
				case 'x':
				{
					layout = layout | (Texture::CUBE_FACE_NEGATIVE_X << (index * 4));
				}
					break;

				case 'Y':
				{
					layout = layout | (Texture::CUBE_FACE_POSITIVE_Y << (index * 4));
				}
					break;
					
				case 'y':
				{
					layout = layout | (Texture::CUBE_FACE_NEGATIVE_Y << (index * 4));
				}
					break;

				case 'Z':
				{
					layout = layout | (Texture::CUBE_FACE_POSITIVE_Z << (index * 4));
				}
					break;
					
				case 'z':
				{
					layout = layout | (Texture::CUBE_FACE_NEGATIVE_Z << (index * 4));
				}
					break;

			}
			
			index++;
		}
	}
	else if(pvrHeader->u32NumFaces > 1)
	{
		static uint32 faces[] = {
			Texture::CUBE_FACE_POSITIVE_X,
			Texture::CUBE_FACE_NEGATIVE_X,
			Texture::CUBE_FACE_POSITIVE_Y,
			Texture::CUBE_FACE_NEGATIVE_Y,
			Texture::CUBE_FACE_POSITIVE_Z,
			Texture::CUBE_FACE_NEGATIVE_Z
		};
		for(uint32 i = 0; i < pvrHeader->u32NumFaces; ++i)
		{
			layout = layout | (faces[i] << (i * 4));
		}
	}
	
	return layout;
}
	
bool LibPVRHelper::IsCubemap(PVRHeaderV3* pvrHeader, const char* pvrData, const int32 pvrDataSize)
{
	uint32 dataSize;
	const char* metadata = GetCubemapMetadata(pvrHeader, pvrData, pvrDataSize, &dataSize);
	
	return (metadata != NULL);
}
	
const char* LibPVRHelper::GetCubemapMetadata(PVRHeaderV3* pvrHeader, const char* pvrData, const int32 pvrDataSize, uint32* outDataSize)
{
	const char* metadata = NULL;
	
	uint32 fourCC = 0;
	uint32 dataKey = 0;
	uint32 dataSize = 0;
	
	//minimal metadata is FOURCC+DataKey+DataSize
	if(pvrHeader->u32MetaDataSize > (sizeof(fourCC) + sizeof(dataKey) + sizeof(dataSize)))
	{
		uint32 index = 0;
		
		while(index < pvrHeader->u32MetaDataSize)
		{
			fourCC = *(uint32*)pvrData;
			pvrData += sizeof(fourCC);
			
			dataKey = *(uint32*)pvrData;
			pvrData += sizeof(dataKey);
			
			dataSize = *(uint32*)pvrData;
			pvrData += sizeof(dataSize);
			
			if(PVRTEX3_METADATAIDENT == fourCC &&
			   6 == dataKey &&
			   dataSize > 0)
			{
				metadata = pvrData;
				break;
			}
			else
			{
				pvrData += dataSize * sizeof(uint8);
				index += sizeof(fourCC) + sizeof(dataKey) + sizeof(dataSize) + dataSize * sizeof(uint8);
			}
		}
	}
	
	return metadata;
}
    
};

