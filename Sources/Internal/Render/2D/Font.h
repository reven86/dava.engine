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


#ifndef __DAVAENGINE_FONT_H__
#define __DAVAENGINE_FONT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "Base/EventDispatcher.h"

namespace DAVA
{
/**
	\defgroup fonts Font
 */

class Texture;
class YamlNode;

/** 
	\ingroup fonts
	\brief This class is base class for all fonts.
 
	This class implements all base functions to handle properties of the font.
	All fonts in system should be inherited from this class.
 */
class Font : public BaseObject
{
public:
    enum eFontType
    {
        TYPE_FT = 0 //!< freetype-based
        ,
        TYPE_GRAPHIC //!< sprite-based
        ,
        TYPE_DISTANCE //!< distance-based
    };

    /**
		\brief Structure with sizes of string
		Contents draw rect (buffer, sprite bounds), height, baseline, width.
	*/
	struct StringMetrics
	{
        inline StringMetrics(): drawRect(), height(0), width(0), baseline(0) {}
		Rect2i drawRect;
		int32 height;
		int32 width;
		int32 baseline;
	};
protected:
	virtual ~Font();
public:
	Font();

	/**
		\brief Set global DPI(dots per inch).
		Default value is 72.
		\param[in] dpi DPI value
	*/
	static void SetDPI(int32 dpi);


	/**
		\brief Get global DPI.
	*/
	static int32 GetDPI();
	
	/**
		\brief Set size.
		\param[in] size in points
	*/
	virtual void SetSize(float32 size);

	/**
		\brief Get size.
		\returns size in pixels
	*/
	virtual float32	GetSize() const;

	/**
	 \brief Set vertical spacing.
	 Spacing value is added to vertical range between lines in multiline text.
	 \param[in] verticalSpacing value in pixels
	 */
	virtual void SetVerticalSpacing(int32 verticalSpacing);


	/**
	 \brief Get vertical spacing 
	 \returns vertical spacing value in pixels
	 */
	virtual int32 GetVerticalSpacing() const;

    /**
     \brief Set font ascend scale factor for FT font. Using for vertical align.
     \param[in] ascendScale ascend scale factor
    */
    virtual void SetAscendScale(float32 ascendScale);
    
    /**
    \brief Get font ascend scale factor for FT font. Using for vertical align.
    \returns ascend scale factor
    */
    virtual float32 GetAscendScale() const;

    /**
     \brief Set font descend scale factor for FT font. Using for vertical align.
     \param[in] descendScale descend scale factor
    */
    virtual void SetDescendScale(float32 descendScale);

    /**
     \brief Get font descend scale factor for FT font. Using for vertical align.
     \returns descend scale factor
    */
    virtual float32 GetDescendScale() const;

    /**
		\brief Get string size(rect).
		\param[in] str - processed string
		\param[in, out] charSizes - if present(not NULL), will contain widths of every symbol in str 
		\returns bounding rect for string in pixels
	*/
	virtual Size2i GetStringSize(const WideString & str, Vector<float32> *charSizes = 0);
	
	/**
	 \brief Get string metrics.
	 \param[in] str - processed string
	 \param[in, out] charSizes - if present(not NULL), will contain widths of every symbol in str
	 \returns StringMetrics structure
	 */
	virtual StringMetrics GetStringMetrics(const WideString & str, Vector<float32> *charSizes = 0) const = 0;

	/**
		\brief Checks if symbol is present in font.
		\param[in] ch - tested symbol
		\returns true if symbol is available, false otherwise
	*/
	virtual bool IsCharAvaliable(char16 ch) const = 0;

	/**
		\brief Get height of highest symbol in font.
		\returns height in pixels
	*/
	virtual uint32 GetFontHeight() const = 0;

	/**
		\brief Clone font.
	*/
	virtual Font * Clone() const = 0;

	/**
		\brief Tests if two fonts are the same.
	*/
	virtual bool IsEqual(const Font *font) const;

	//TODO: get rid of this
	virtual bool IsTextSupportsSoftwareRendering() const { return false; };
	virtual bool IsTextSupportsHardwareRendering() const { return false; };

	//This will allow to determine font type
	virtual eFontType GetFontType() const;

	/* Put font properties into YamlNode */
	virtual YamlNode * SaveToYamlNode() const;

	// Return the hashcode (identical for identical fonts).
	virtual uint32 GetHashCode();

protected:
    // Get the raw hash string (identical for identical fonts).
	virtual String GetRawHashString();
    
	static int32 globalFontDPI;
	
	float32	size;
	int32 verticalSpacing;
	eFontType fontType;
};
    
};

#endif // __DAVAENGINE_FONT_H__
