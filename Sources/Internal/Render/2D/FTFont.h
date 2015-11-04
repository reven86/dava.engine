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


#ifndef __DAVAENGINE_FTFONT_H__
#define __DAVAENGINE_FTFONT_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/2D/Font.h"
#include "Concurrency/Mutex.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{	
	
class FontManager;
class FTInternalFont;

/** 
	\ingroup fonts
	\brief Freetype-based font implementation.
 
	Class is a wrapper to freetype2 library.
 */
class FTFont : public Font
{	
public:
	
	/**
		\brief Factory method.
		\param[in] path - path to freetype-supported file (.ttf, .otf)
		\returns constructed font
	*/
	static		FTFont * Create(const FilePath & path);
	
	/**
		\brief Function clears cache of internal fonts
	*/
	static void ClearCache();

	virtual		~FTFont();

	/**
		\brief Clone font.
	*/
	FTFont *	Clone() const;

	/**
		\brief Tests if two fonts are the same.
	*/
	virtual bool IsEqual(const Font *font) const;

	/**
		\brief Get string metrics.
		\param[in] str - processed string
		\param[in, out] charSizes - if present(not NULL), will contain widths of every symbol in str
		\returns StringMetrics structure
	 */
	virtual StringMetrics GetStringMetrics(const WideString & str, Vector<float32> *charSizes = NULL) const;

	/**
		\brief Get height of highest symbol in font.
		\returns height in pixels
	*/
	virtual uint32		GetFontHeight() const;
	
	/**
		\brief Checks if symbol is present in font.
		\param[in] ch - tested symbol
		\returns true if symbol is available, false otherwise
	*/
	virtual bool		IsCharAvaliable(char16 ch) const;

	/**
		\brief Draw string to memory buffer
		\param[in, out] buffer - destination buffer
		\param[in] bufWidth - buffer width in pixels
		\param[in] bufHeight - buffer height in pixels
		\param[in] offsetX - starting X offset
		\param[in] offsetY - starting Y offset
		\param[in] justifyWidth - TODO
		\param[in] spaceAddon - TODO
		\param[in] str - string to draw
		\param[in] contentScaleIncluded - TODO
		\returns bounding rect for string in pixels
	*/
	virtual StringMetrics DrawStringToBuffer(void * buffer, int32 bufWidth, int32 bufHeight, int32 offsetX, int32 offsetY, int32 justifyWidth, int32 spaceAddon, const WideString & str, bool contentScaleIncluded = false);

	virtual bool IsTextSupportsSoftwareRendering() const { return true; };

	//We need to return font path
	const FilePath & GetFontPath() const;
	// Put font properties into YamlNode
	virtual YamlNode * SaveToYamlNode() const;

    void SetAscendScale(float32 ascend) override;
    float32 GetAscendScale() const override;
    void SetDescendScale(float32 ascend) override;
    float32 GetDescendScale() const override;

protected:
	// Get the raw hash string (identical for identical fonts).
	virtual String GetRawHashString();

private:
	FTFont(FTInternalFont* internalFont);
	FTInternalFont	* internalFont;

    float32 ascendScale;
    float32 descendScale;
	
	FilePath fontPath;
};


	
};

#endif  //__DAVAENGINE_FTFONT_H__