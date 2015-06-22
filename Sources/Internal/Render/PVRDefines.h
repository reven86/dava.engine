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


#ifndef __DAVAENGINE_PVRDEFINES_H__
#define __DAVAENGINE_PVRDEFINES_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    
    
#define PVRT_MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define PVRT_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define PVRT_CLAMP(x, l, h)      (PVRT_MIN((h), PVRT_MAX((x), (l))))

    
    
/*****************************************************************************
* Texture related constants and enumerations. 
*****************************************************************************/
// V3 Header Identifiers.
const uint32 PVRTEX3_IDENT			= 0x03525650;	// 'P''V''R'3
const uint32 PVRTEX3_IDENT_REV		= 0x50565203;
// If endianness is backwards then PVR3 will read as 3RVP, hence why it is written as an int.

//Current version texture identifiers
const uint32 PVRTEX_CURR_IDENT		= PVRTEX3_IDENT;
const uint32 PVRTEX_CURR_IDENT_REV	= PVRTEX3_IDENT_REV;

// PVR Header file flags.										Condition if true. If false, opposite is true unless specified.
const uint32 PVRTEX3_FILE_COMPRESSED	= (1<<0);		//	Texture has been file compressed using PVRTexLib (currently unused)
const uint32 PVRTEX3_PREMULTIPLIED		= (1<<1);		//	Texture has been premultiplied by alpha value.	

// Mip Map level specifier constants. Other levels are specified by 1,2...n
const uint32 PVRTEX_TOPMIPLEVEL			= 0;
const uint32 PVRTEX_ALLMIPLEVELS			= -1; //This is a special number used simply to return a total of all MIP levels when dealing with data sizes.

//values for each meta data type that we know about. Texture arrays hinge on each surface being identical in all but content, including meta data. 
//If the meta data varies even slightly then a new texture should be used. It is possible to write your own extension to get around this however.
enum EPVRTMetaData
{
	ePVRTMetaDataTextureAtlasCoords=0,
	ePVRTMetaDataBumpData,
	ePVRTMetaDataCubeMapOrder,
	ePVRTMetaDataTextureOrientation,
	ePVRTMetaDataBorderData,
	ePVRTMetaDataPadding,
	ePVRTMetaDataNumMetaDataTypes
};

enum EPVRTAxis
{
	ePVRTAxisX = 0,
	ePVRTAxisY = 1,
	ePVRTAxisZ = 2
};

enum EPVRTOrientation
{
	ePVRTOrientLeft	= 1<<ePVRTAxisX,
	ePVRTOrientRight= 0,
	ePVRTOrientUp	= 1<<ePVRTAxisY,
	ePVRTOrientDown	= 0,
	ePVRTOrientOut	= 1<<ePVRTAxisZ,
	ePVRTOrientIn	= 0
};

enum EPVRTColourSpace
{
	ePVRTCSpacelRGB,
	ePVRTCSpacesRGB,
	ePVRTCSpaceNumSpaces
};

//Compressed pixel formats
enum EPVRTPixelFormat
{
	ePVRTPF_PVRTCI_2bpp_RGB,
	ePVRTPF_PVRTCI_2bpp_RGBA,
	ePVRTPF_PVRTCI_4bpp_RGB,
	ePVRTPF_PVRTCI_4bpp_RGBA,
	ePVRTPF_PVRTCII_2bpp,
	ePVRTPF_PVRTCII_4bpp,
	ePVRTPF_ETC1,
	ePVRTPF_DXT1,
	ePVRTPF_DXT2,
	ePVRTPF_DXT3,
	ePVRTPF_DXT4,
	ePVRTPF_DXT5,

	//These formats are identical to some DXT formats.
	ePVRTPF_BC1 = ePVRTPF_DXT1,
	ePVRTPF_BC2 = ePVRTPF_DXT3,
	ePVRTPF_BC3 = ePVRTPF_DXT5,

	//These are currently unsupported:
	ePVRTPF_BC4,
	ePVRTPF_BC5,
	ePVRTPF_BC6,
	ePVRTPF_BC7,

	//These are supported
	ePVRTPF_UYVY,
	ePVRTPF_YUY2,
	ePVRTPF_BW1bpp,
	ePVRTPF_SharedExponentR9G9B9E5,
	ePVRTPF_RGBG8888,
	ePVRTPF_GRGB8888,
	ePVRTPF_ETC2_RGB,
	ePVRTPF_ETC2_RGBA,
	ePVRTPF_ETC2_RGB_A1,
	ePVRTPF_EAC_R11,
	ePVRTPF_EAC_RG11,

	//Invalid value
	ePVRTPF_NumCompressedPFs
};

//Variable Type Names
enum EPVRTVariableType
{
	ePVRTVarTypeUnsignedByteNorm,
	ePVRTVarTypeSignedByteNorm,
	ePVRTVarTypeUnsignedByte,
	ePVRTVarTypeSignedByte,
	ePVRTVarTypeUnsignedShortNorm,
	ePVRTVarTypeSignedShortNorm,
	ePVRTVarTypeUnsignedShort,
	ePVRTVarTypeSignedShort,
	ePVRTVarTypeUnsignedIntegerNorm,
	ePVRTVarTypeSignedIntegerNorm,
	ePVRTVarTypeUnsignedInteger,
	ePVRTVarTypeSignedInteger,
	ePVRTVarTypeSignedFloat,	ePVRTVarTypeFloat=ePVRTVarTypeSignedFloat, //the name ePVRTVarTypeFloat is now deprecated.
	ePVRTVarTypeUnsignedFloat,
	ePVRTVarTypeNumVarTypes
};

//A 64 bit pixel format ID & this will give you the high bits of a pixel format to check for a compressed format.
static const uint64 PVRTEX_PFHIGHMASK=0xffffffff00000000ull;


/*****************************************************************************
* Legacy (V2 and V1) ENUMS
*****************************************************************************/

	enum PVRTPixelType
	{
		MGLPT_ARGB_4444 = 0x00,
		MGLPT_ARGB_1555,
		MGLPT_RGB_565,
		MGLPT_RGB_555,
		MGLPT_RGB_888,
		MGLPT_ARGB_8888,
		MGLPT_ARGB_8332,
		MGLPT_I_8,
		MGLPT_AI_88,
		MGLPT_1_BPP,
		MGLPT_VY1UY0,
		MGLPT_Y1VY0U,
		MGLPT_PVRTC2,
		MGLPT_PVRTC4,

		// OpenGL version of pixel types
		OGL_RGBA_4444= 0x10,
		OGL_RGBA_5551,
		OGL_RGBA_8888,
		OGL_RGB_565,
		OGL_RGB_555,
		OGL_RGB_888,
		OGL_I_8,
		OGL_AI_88,
		OGL_PVRTC2,
		OGL_PVRTC4,
		OGL_BGRA_8888,
		OGL_A_8,
		OGL_PVRTCII4,	//Not in use
		OGL_PVRTCII2,	//Not in use

		// S3TC Encoding
		D3D_DXT1 = 0x20,
		D3D_DXT2,
		D3D_DXT3,
		D3D_DXT4,
		D3D_DXT5,

		//RGB Formats
		D3D_RGB_332,
		D3D_AL_44,
		D3D_LVU_655,
		D3D_XLVU_8888,
		D3D_QWVU_8888,
		
		//10 bit integer - 2 bit alpha
		D3D_ABGR_2101010,
		D3D_ARGB_2101010,
		D3D_AWVU_2101010,

		//16 bit integers
		D3D_GR_1616,
		D3D_VU_1616,
		D3D_ABGR_16161616,

		//Float Formats
		D3D_R16F,
		D3D_GR_1616F,
		D3D_ABGR_16161616F,

		//32 bits per channel
		D3D_R32F,
		D3D_GR_3232F,
		D3D_ABGR_32323232F,
		
		// Ericsson
		ETC_RGB_4BPP,
		ETC_RGBA_EXPLICIT,				// unimplemented
		ETC_RGBA_INTERPOLATED,			// unimplemented
		
		D3D_A8 = 0x40,
		D3D_V8U8,
		D3D_L16,
				
		D3D_L8,
		D3D_AL_88,

		//Y'UV Colourspace
		D3D_UYVY,
		D3D_YUY2,
		
		// DX10
		DX10_R32G32B32A32_FLOAT= 0x50,
		DX10_R32G32B32A32_UINT , 
		DX10_R32G32B32A32_SINT,

		DX10_R32G32B32_FLOAT,
		DX10_R32G32B32_UINT,
		DX10_R32G32B32_SINT,

		DX10_R16G16B16A16_FLOAT ,
		DX10_R16G16B16A16_UNORM,
		DX10_R16G16B16A16_UINT ,
		DX10_R16G16B16A16_SNORM ,
		DX10_R16G16B16A16_SINT ,

		DX10_R32G32_FLOAT ,
		DX10_R32G32_UINT ,
		DX10_R32G32_SINT ,

		DX10_R10G10B10A2_UNORM ,
		DX10_R10G10B10A2_UINT ,

		DX10_R11G11B10_FLOAT ,				// unimplemented

		DX10_R8G8B8A8_UNORM , 
		DX10_R8G8B8A8_UNORM_SRGB ,
		DX10_R8G8B8A8_UINT ,
		DX10_R8G8B8A8_SNORM ,
		DX10_R8G8B8A8_SINT ,

		DX10_R16G16_FLOAT , 
		DX10_R16G16_UNORM , 
		DX10_R16G16_UINT , 
		DX10_R16G16_SNORM ,
		DX10_R16G16_SINT ,

		DX10_R32_FLOAT ,
		DX10_R32_UINT ,
		DX10_R32_SINT ,

		DX10_R8G8_UNORM ,
		DX10_R8G8_UINT ,
		DX10_R8G8_SNORM , 
		DX10_R8G8_SINT ,

		DX10_R16_FLOAT ,
		DX10_R16_UNORM ,
		DX10_R16_UINT ,
		DX10_R16_SNORM ,
		DX10_R16_SINT ,

		DX10_R8_UNORM, 
		DX10_R8_UINT,
		DX10_R8_SNORM,
		DX10_R8_SINT,

		DX10_A8_UNORM, 
		DX10_R1_UNORM, 
		DX10_R9G9B9E5_SHAREDEXP,	// unimplemented
		DX10_R8G8_B8G8_UNORM,		// unimplemented
		DX10_G8R8_G8B8_UNORM,		// unimplemented

		DX10_BC1_UNORM,	
		DX10_BC1_UNORM_SRGB,

		DX10_BC2_UNORM,	
		DX10_BC2_UNORM_SRGB,

		DX10_BC3_UNORM,	
		DX10_BC3_UNORM_SRGB,

		DX10_BC4_UNORM,				// unimplemented
		DX10_BC4_SNORM,				// unimplemented

		DX10_BC5_UNORM,				// unimplemented
		DX10_BC5_SNORM,				// unimplemented

		// OpenVG

		/* RGB{A,X} channel ordering */
		ePT_VG_sRGBX_8888  = 0x90,
		ePT_VG_sRGBA_8888,
		ePT_VG_sRGBA_8888_PRE,
		ePT_VG_sRGB_565,
		ePT_VG_sRGBA_5551,
		ePT_VG_sRGBA_4444,
		ePT_VG_sL_8,
		ePT_VG_lRGBX_8888,
		ePT_VG_lRGBA_8888,
		ePT_VG_lRGBA_8888_PRE,
		ePT_VG_lL_8,
		ePT_VG_A_8,
		ePT_VG_BW_1,

		/* {A,X}RGB channel ordering */
		ePT_VG_sXRGB_8888,
		ePT_VG_sARGB_8888,
		ePT_VG_sARGB_8888_PRE,
		ePT_VG_sARGB_1555,
		ePT_VG_sARGB_4444,
		ePT_VG_lXRGB_8888,
		ePT_VG_lARGB_8888,
		ePT_VG_lARGB_8888_PRE,

		/* BGR{A,X} channel ordering */
		ePT_VG_sBGRX_8888,
		ePT_VG_sBGRA_8888,
		ePT_VG_sBGRA_8888_PRE,
		ePT_VG_sBGR_565,
		ePT_VG_sBGRA_5551,
		ePT_VG_sBGRA_4444,
		ePT_VG_lBGRX_8888,
		ePT_VG_lBGRA_8888,
		ePT_VG_lBGRA_8888_PRE,

		/* {A,X}BGR channel ordering */
		ePT_VG_sXBGR_8888,
		ePT_VG_sABGR_8888 ,
		ePT_VG_sABGR_8888_PRE,
		ePT_VG_sABGR_1555,
		ePT_VG_sABGR_4444,
		ePT_VG_lXBGR_8888,
		ePT_VG_lABGR_8888,
		ePT_VG_lABGR_8888_PRE,

		// max cap for iterating
		END_OF_PIXEL_TYPES,

		MGLPT_NOTYPE = 0xffffffff

	};

/*****************************************************************************
* Legacy constants (V1/V2)
*****************************************************************************/

const uint32 PVRTEX_MIPMAP			= (1<<8);		// has mip map levels
const uint32 PVRTEX_TWIDDLE			= (1<<9);		// is twiddled
const uint32 PVRTEX_BUMPMAP			= (1<<10);		// has normals encoded for a bump map
const uint32 PVRTEX_TILING			= (1<<11);		// is bordered for tiled pvr
const uint32 PVRTEX_CUBEMAP			= (1<<12);		// is a cubemap/skybox
const uint32 PVRTEX_FALSEMIPCOL		= (1<<13);		// are there false coloured MIP levels
const uint32 PVRTEX_VOLUME			= (1<<14);		// is this a volume texture
const uint32 PVRTEX_ALPHA			= (1<<15);		// v2.1 is there transparency info in the texture
const uint32 PVRTEX_VERTICAL_FLIP	= (1<<16);		// v2.1 is the texture vertically flipped

const uint32 PVRTEX_PIXELTYPE		= 0xff;			// pixel type is always in the last 16bits of the flags
const uint32 PVRTEX_IDENTIFIER		= 0x21525650;	// the pvr identifier is the characters 'P','V','R'

const uint32 PVRTEX_V1_HEADER_SIZE	= 44;			// old header size was 44 for identification purposes

const uint32 PVRTC2_MIN_TEXWIDTH	= 16;
const uint32 PVRTC2_MIN_TEXHEIGHT	= 8;
const uint32 PVRTC4_MIN_TEXWIDTH	= 8;
const uint32 PVRTC4_MIN_TEXHEIGHT	= 8;
const uint32 ETC_MIN_TEXWIDTH		= 4;
const uint32 ETC_MIN_TEXHEIGHT		= 4;
const uint32 DXT_MIN_TEXWIDTH		= 4;
const uint32 DXT_MIN_TEXHEIGHT		= 4;


//Generate a 4 channel PixelID.
#define PVRTGENPIXELID4(C1Name, C2Name, C3Name, C4Name, C1Bits, C2Bits, C3Bits, C4Bits) ( ( (uint64)C1Name) + ( (uint64)C2Name<<8) + ( (uint64)C3Name<<16) + ( (uint64)C4Name<<24) + ( (uint64)C1Bits<<32) + ( (uint64)C2Bits<<40) + ( (uint64)C3Bits<<48) + ( (uint64)C4Bits<<56) )

//Generate a 1 channel PixelID.
#define PVRTGENPIXELID3(C1Name, C2Name, C3Name, C1Bits, C2Bits, C3Bits)( PVRTGENPIXELID4(C1Name, C2Name, C3Name, 0, C1Bits, C2Bits, C3Bits, 0) )

//Generate a 2 channel PixelID.
#define PVRTGENPIXELID2(C1Name, C2Name, C1Bits, C2Bits) ( PVRTGENPIXELID4(C1Name, C2Name, 0, 0, C1Bits, C2Bits, 0, 0) )

//Generate a 3 channel PixelID.
#define PVRTGENPIXELID1(C1Name, C1Bits) ( PVRTGENPIXELID4(C1Name, 0, 0, 0, C1Bits, 0, 0, 0))

   
#define FREE(X)		{ if(X) { free(X); (X) = 0; } }

    
    
/****************************************************************************
 ** swap template function
 ****************************************************************************/
/*!***************************************************************************
 @Function		PVRTswap
 @Input			a Type a
 @Input			b Type b
 @Description	A swap template function that swaps a and b
 *****************************************************************************/

template <typename T>
inline void PVRTswap(T& a, T& b)
{
    T temp = a;
    a = b;
    b = temp;
}

/*!***************************************************************************
 @Function		PVRTClamp
 @Input			val		Value to clamp
 @Input			min		Minimum legal value
 @Input			max		Maximum legal value
 @Description	A clamp template function that clamps val between min and max.
 *****************************************************************************/
template <typename T>
inline T PVRTClamp(const T& val, const T& min, const T& max)
{
    if(val > max)
        return max;
    if(val < min)
        return min;
    return val;
}

    
/*!***************************************************************************
 @Function		PVRTByteSwap
 @Input			pBytes A number
 @Input			i32ByteNo Number of bytes in pBytes
 @Description	Swaps the endianness of pBytes in place
 *****************************************************************************/
inline void PVRTByteSwap(unsigned char* pBytes, int i32ByteNo)
{
    int i = 0, j = i32ByteNo - 1;
    
    while(i < j)
        PVRTswap<unsigned char>(pBytes[i++], pBytes[j--]);
}

/*!***************************************************************************
 @Function		PVRTByteSwap32
 @Input			ui32Long A number
 @Returns		ui32Long with its endianness changed
 @Description	Converts the endianness of an unsigned int
 *****************************************************************************/
inline unsigned int PVRTByteSwap32(unsigned int ui32Long)
{
    return ((ui32Long&0x000000FF)<<24) + ((ui32Long&0x0000FF00)<<8) + ((ui32Long&0x00FF0000)>>8) + ((ui32Long&0xFF000000) >> 24);
}

/*!***************************************************************************
 @Function		PVRTByteSwap16
 @Input			ui16Short A number
 @Returns		ui16Short with its endianness changed
 @Description	Converts the endianness of a unsigned short
 *****************************************************************************/
inline unsigned short PVRTByteSwap16(unsigned short ui16Short)
{
    return (ui16Short>>8) | (ui16Short<<8);
}

/*!***************************************************************************
 @Function		PVRTIsLittleEndian
 @Returns		True if the platform the code is ran on is little endian
 @Description	Returns true if the platform the code is ran on is little endian
 *****************************************************************************/
inline bool PVRTIsLittleEndian()
{
    static bool bLittleEndian;
    static bool bIsInit = false;
    
    if(!bIsInit)
    {
        short int word = 0x0001;
        char *byte = (char*) &word;
        bLittleEndian = byte[0] ? true : false;
        bIsInit = true;
    }
    
    return bLittleEndian;
}

    
/*!***********************************************************************
 @struct       	MetaDataBlock
 @brief      	A struct containing a block of extraneous meta data for a texture.
 *************************************************************************/
struct MetaDataBlock
{
    uint32	DevFOURCC;		///< A 4cc descriptor of the data type's creator. Values equating to values between 'P' 'V' 'R' 0 and 'P' 'V' 'R' 255 will be used by our headers.
    uint32	u32Key;			///< A DWORD (enum value) identifying the data type, and thus how to read it.
    uint32	u32DataSize;	///< Size of the Data member.
    uint8*	Data;			///< Data array, can be absolutely anything, the loader needs to know how to handle it based on DevFOURCC and Key. Use new operator to assign to it.
    
    /*!***********************************************************************
     @fn       		MetaDataBlock
     @brief      	Meta Data Block Constructor
     *************************************************************************/
    MetaDataBlock() : DevFOURCC(0), u32Key(0), u32DataSize(0), Data(NULL)
    {}
    
    /*!***********************************************************************
     @fn       		MetaDataBlock
     @brief      	Meta Data Block Copy Constructor
     *************************************************************************/
    MetaDataBlock(const MetaDataBlock& rhs)  : DevFOURCC(rhs.DevFOURCC), u32Key(rhs.u32Key), u32DataSize(rhs.u32DataSize)
    {
        //Copy the data across.
        Data = new uint8[u32DataSize];
        for (uint32 uiDataAmt=0; uiDataAmt<u32DataSize; ++uiDataAmt)
        {
            Data[uiDataAmt]=rhs.Data[uiDataAmt];
        }
    }
    
    /*!***********************************************************************
     @fn       		~MetaDataBlock
     @brief      	Meta Data Block Destructor
     *************************************************************************/
    ~MetaDataBlock()
    {
        if (Data)
            delete [] Data;
        Data = NULL;
    }
    
    /*!***********************************************************************
     @fn       		SizeOfBlock
     @return			size_t Size (in a file) of the block.
     @brief      	Returns the number of extra bytes this will add to any output files.
     *************************************************************************/
    size_t SizeOfBlock() const
    {
        return sizeof(DevFOURCC)+sizeof(u32Key)+sizeof(u32DataSize)+u32DataSize;
    }
    
    /*!***********************************************************************
     @brief      	Assigns one MetaDataBlock to the other.
     @return			MetaDataBlock This MetaDataBlock after the operation.
     *************************************************************************/
    MetaDataBlock& operator=(const MetaDataBlock& rhs)
    {
        if (&rhs==this)
            return *this;
        
        //Remove pre-existing data.
        if (Data)
            delete [] Data;
        Data=NULL;
        
        //Copy the basic parameters
        DevFOURCC=rhs.DevFOURCC;
        u32Key=rhs.u32Key;
        u32DataSize=rhs.u32DataSize;
        
        //Copy the data across.
        if (rhs.Data)
        {
            Data = new uint8[u32DataSize];
            for (uint32 uiDataAmt=0; uiDataAmt<u32DataSize; ++uiDataAmt)
            {
                Data[uiDataAmt]=rhs.Data[uiDataAmt];
            }
        }
        
        return *this;
    }
    
    /*!***************************************************************************
     @fn       		ReadFromPtr
     @param[in]		pDataCursor		The data to read
     @brief      	Reads from a pointer of memory in to the meta data block.
     *****************************************************************************/
    bool ReadFromPtr(const unsigned char** pDataCursor);
};
    

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    
};

#endif //__DAVAENGINE_PVRDEFINES_H__
