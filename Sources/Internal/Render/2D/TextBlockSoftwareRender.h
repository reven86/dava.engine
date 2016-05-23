#ifndef __DAVAENGINE_TEXTBLOCK_SOFTWARE_RENDER_H__
#define __DAVAENGINE_TEXTBLOCK_SOFTWARE_RENDER_H__

#include "Render/2D/TextBlockRender.h"
#include "Render/2D/FTFont.h"

namespace DAVA
{
class TextBlockSoftwareRender : public TextBlockRender
{
public:
    TextBlockSoftwareRender(TextBlock*);
    ~TextBlockSoftwareRender();
    void Prepare() override;
    

#if defined(LOCALIZATION_DEBUG)
    //in physical coordinates
    Vector2 getTextOffsetTL();
    //in physical coordinates
    Vector2 getTextOffsetBR();
#endif
protected:
    Font::StringMetrics DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w) override;
    Font::StringMetrics DrawTextML(const WideString& drawText,
                                   int32 x, int32 y, int32 w,
                                   int32 xOffset, uint32 yOffset,
                                   int32 lineSize) override;

#if defined(LOCALIZATION_DEBUG)
    void CalculateTextBBox();
#endif

private:
    void Restore();

#if defined(LOCALIZATION_DEBUG)
    Vector2 textOffsetTL;
    Vector2 textOffsetBR;
    int32 bufHeight, bufWidth;
#endif
    int8* buf;
    FTFont* ftFont;
};

}; //end of namespace

#endif // __DAVAENGINE_TEXTBLOCK_SOFTWARE_RENDER_H__