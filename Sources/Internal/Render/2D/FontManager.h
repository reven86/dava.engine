#ifndef __DAVAENGINE_FONTMANAGER_H__
#define __DAVAENGINE_FONTMANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/BaseMath.h"

struct FT_LibraryRec_;
using FT_Library = struct FT_LibraryRec_*;

namespace DAVA
{
class Font;
class FTFont;
class FTInternalFont;
class Sprite;
class UIStaticText;

class FontManager : public Singleton<FontManager>
{
    FT_Library library;

public:
    FontManager();
    virtual ~FontManager();

    FT_Library GetFTLibrary()
    {
        return library;
    }

    /**
	 \brief Register font.
	 */
    void RegisterFont(Font* font);
    /**
	 \brief Unregister font.
	 */
    void UnregisterFont(Font* font);
    /**
	 \brief Register all fonts.
	 */
    void RegisterFonts(const Map<String, Font*>& fonts);
    /**
	 \brief Unregister all fonts.
	 */
    void UnregisterFonts();

    /**
	 \brief Set font name.
	 */
    void SetFontName(Font* font, const String& name);

    /**
	 \brief Get traked font name. Add font to track list.
	 */
    String GetFontName(Font* font) const;

    /**
	 \brief Get font by name.
	 */
    Font* GetFont(const String& name) const;

    /**
	 \brief Get registered fonts.
	 */
    const Map<Font*, String>& GetRegisteredFonts() const;

    /**
     \brief Get name->font map.
     */
    const Map<String, Font*>& GetFontMap() const;

private:
    String GetFontHashName(Font* font) const;

private:
    Map<Font*, String> registeredFonts;
    Map<String, Font*> fontMap;
};
};


#endif //__DAVAENGINE_FONTMANAGER_H__
