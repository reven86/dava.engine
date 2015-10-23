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


#include "UI/UIYamlLoader.h"
#include "Base/ObjectFactory.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControl.h"
#include "UI/UIScrollBar.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FileSystem.h"
#include "Render/2D/GraphicFont.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/TextBlock.h"
#include "Render/2D/FTFont.h"
#include "Utils/Utils.h"
#include "UI/UIPackage.h"
#include "UI/DefaultUIPackageBuilder.h"
#include "UI/UIPackageLoader.h"
#include "UI/UIControlHelpers.h"
#include "UI/Layouts/UILayoutSystem.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
UIYamlLoader::UIYamlLoader() :
    BaseObject()
{
    // Default mode is to ASSERT if custom control isn't found.
    assertIfCustomControlNotFound = true;

    currentPath = FilePath();
}

int32 UIYamlLoader::GetDrawTypeFromNode(const YamlNode * drawTypeNode) const
{
    int32 ret = UIControlBackground::DRAW_ALIGNED;
    if(!drawTypeNode)
        return ret;

    const String & type = drawTypeNode->AsString();

    if("DRAW_ALIGNED" == type) ret = UIControlBackground::DRAW_ALIGNED;
    if("DRAW_SCALE_TO_RECT" == type) ret = UIControlBackground::DRAW_SCALE_TO_RECT;
    if("DRAW_SCALE_PROPORTIONAL" == type) ret = UIControlBackground::DRAW_SCALE_PROPORTIONAL;
    if("DRAW_SCALE_PROPORTIONAL_ONE" == type) ret = UIControlBackground::DRAW_SCALE_PROPORTIONAL_ONE;
    if("DRAW_FILL" == type) ret = UIControlBackground::DRAW_FILL;
    if("DRAW_STRETCH_HORIZONTAL" == type) ret = UIControlBackground::DRAW_STRETCH_HORIZONTAL;
    if("DRAW_STRETCH_VERTICAL" == type) ret = UIControlBackground::DRAW_STRETCH_VERTICAL;
    if("DRAW_STRETCH_BOTH" == type) ret = UIControlBackground::DRAW_STRETCH_BOTH;
    if("DRAW_TILED" == type) ret = UIControlBackground::DRAW_TILED;

    return ret;
}

String UIYamlLoader::GetDrawTypeNodeValue(int32 drawType) const
{
    String ret;
    switch (drawType) {
        case UIControlBackground::DRAW_ALIGNED:
            ret = "DRAW_ALIGNED";
            break;
        case UIControlBackground::DRAW_SCALE_TO_RECT:
            ret = "DRAW_SCALE_TO_RECT";
            break;
        case UIControlBackground::DRAW_SCALE_PROPORTIONAL:
            ret = "DRAW_SCALE_PROPORTIONAL";
            break;
        case UIControlBackground::DRAW_SCALE_PROPORTIONAL_ONE:
            ret = "DRAW_SCALE_PROPORTIONAL_ONE";
            break;
        case UIControlBackground::DRAW_FILL:
            ret = "DRAW_FILL";
            break;
        case UIControlBackground::DRAW_STRETCH_HORIZONTAL:
            ret = "DRAW_STRETCH_HORIZONTAL";
            break;
        case UIControlBackground::DRAW_STRETCH_VERTICAL:
            ret = "DRAW_STRETCH_VERTICAL";
            break;
        case UIControlBackground::DRAW_STRETCH_BOTH:
            ret = "DRAW_STRETCH_BOTH";
            break;
        case UIControlBackground::DRAW_TILED:
            ret = "DRAW_TILED";
            break;
        default:
            ret = "DRAW_ALIGNED";
            break;
    }
    return ret;
}

int32 UIYamlLoader::GetColorInheritTypeFromNode(const YamlNode * colorInheritNode) const
{
    int32 ret = UIControlBackground::COLOR_IGNORE_PARENT;
    if(!colorInheritNode)
        return ret;

    const String & type = colorInheritNode->AsString();

    if("COLOR_MULTIPLY_ON_PARENT" == type) ret = UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    if("COLOR_ADD_TO_PARENT" == type) ret = UIControlBackground::COLOR_ADD_TO_PARENT;
    if("COLOR_REPLACE_TO_PARENT" == type) ret = UIControlBackground::COLOR_REPLACE_TO_PARENT;
    if("COLOR_IGNORE_PARENT" == type) ret = UIControlBackground::COLOR_IGNORE_PARENT;
    if("COLOR_MULTIPLY_ALPHA_ONLY" == type) ret = UIControlBackground::COLOR_MULTIPLY_ALPHA_ONLY;
    if("COLOR_REPLACE_ALPHA_ONLY" == type) ret = UIControlBackground::COLOR_REPLACE_ALPHA_ONLY;

    return ret;
}

String UIYamlLoader::GetColorInheritTypeNodeValue(int32 colorInheritType) const
{
    String ret;
    switch (colorInheritType) {
        case UIControlBackground::COLOR_MULTIPLY_ON_PARENT:
            ret = "COLOR_MULTIPLY_ON_PARENT";
            break;
        case UIControlBackground::COLOR_ADD_TO_PARENT:
            ret = "COLOR_ADD_TO_PARENT";
            break;
        case UIControlBackground::COLOR_REPLACE_TO_PARENT:
            ret = "COLOR_REPLACE_TO_PARENT";
            break;
        case UIControlBackground::COLOR_IGNORE_PARENT:
            ret = "COLOR_IGNORE_PARENT";
            break;
        case UIControlBackground::COLOR_MULTIPLY_ALPHA_ONLY:
            ret = "COLOR_MULTIPLY_ALPHA_ONLY";
            break;
        case UIControlBackground::COLOR_REPLACE_ALPHA_ONLY:
            ret = "COLOR_REPLACE_ALPHA_ONLY";
            break;
        default:
            ret = "COLOR_IGNORE_PARENT";
            break;
    }
    return ret;
}

int32 UIYamlLoader::GetPerPixelAccuracyTypeFromNode(const YamlNode *perPixelAccuracyNode) const
{
	int32 ret = UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
	if(!perPixelAccuracyNode)
		return ret;

	const String & type = perPixelAccuracyNode->AsString();

	if("PER_PIXEL_ACCURACY_DISABLED" == type) ret = UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
	if("PER_PIXEL_ACCURACY_ENABLED" == type) ret = UIControlBackground::PER_PIXEL_ACCURACY_ENABLED;
	if("PER_PIXEL_ACCURACY_FORCED" == type) ret = UIControlBackground::PER_PIXEL_ACCURACY_FORCED;

	return ret;
}

String UIYamlLoader::GetPerPixelAccuracyTypeNodeValue(int32 perPixelAccuracyType) const
{
	String ret;
    switch (perPixelAccuracyType) {
        case UIControlBackground::PER_PIXEL_ACCURACY_DISABLED:
            ret = "PER_PIXEL_ACCURACY_DISABLED";
            break;
        case UIControlBackground::PER_PIXEL_ACCURACY_ENABLED:
            ret = "PER_PIXEL_ACCURACY_ENABLED";
            break;
        case UIControlBackground::PER_PIXEL_ACCURACY_FORCED:
            ret = "PER_PIXEL_ACCURACY_FORCED";
            break;
    }
    return ret;
}
	
int32 UIYamlLoader::GetAlignFromYamlNode(const YamlNode * alignNode) const
{
    if (!alignNode)return ALIGN_HCENTER | ALIGN_VCENTER;

    const Vector<YamlNode*> & vec = alignNode->AsVector();

    if (vec.size() == 1 && vec[0]->AsString() == "HJUSTIFY") return ALIGN_HJUSTIFY;
    if (vec.size() != 2)return ALIGN_HCENTER | ALIGN_VCENTER;

    const String & horzAlign = vec[0]->AsString();
    const String & vertAlign = vec[1]->AsString();

    int32 align = 0;
    if (horzAlign == "LEFT")align |= ALIGN_LEFT;
    else if (horzAlign == "HCENTER")align |= ALIGN_HCENTER;
    else if (horzAlign == "RIGHT")align |= ALIGN_RIGHT;

    if (vertAlign == "TOP")align |= ALIGN_TOP;
    else if (vertAlign == "VCENTER")align |= ALIGN_VCENTER;
    else if (vertAlign == "BOTTOM")align |= ALIGN_BOTTOM;

    return align;
}

int32 UIYamlLoader::GetFittingOptionFromYamlNode( const YamlNode * fittingNode ) const
{
    int32 fitting = TextBlock::FITTING_DISABLED;

    const Vector<YamlNode*> & vec = fittingNode->AsVector();

    for( uint32 index = 0; index < vec.size(); ++index )
    {
        const String &value = vec[index]->AsString();
        if( value == "DISABLED" )
        {
            fitting = TextBlock::FITTING_DISABLED;
            break;
        }
        else if( value == "ENLARGE" )
        {
            fitting |= TextBlock::FITTING_ENLARGE;
        }
        else if( value == "REDUCE" )
        {
            fitting |= TextBlock::FITTING_REDUCE;
        }
        else if( value == "POINTS" )
        {
            fitting |= TextBlock::FITTING_POINTS;
        }
    }

    return fitting;
}

//Vector<String> UIYamlLoader::GetAlignNodeValue(int32 align)
YamlNode * UIYamlLoader::GetAlignNodeValue(int32 align) const
{
    YamlNode *alignNode = YamlNode::CreateArrayNode(YamlNode::AR_FLOW_REPRESENTATION);
    String horzAlign = "HCENTER";
    String vertAlign = "VCENTER";

    if (align == ALIGN_HJUSTIFY)
    {
        alignNode->Add("HJUSTIFY");
        return alignNode;
    }

    if (align & ALIGN_LEFT)
    {
        horzAlign = "LEFT";
    }
    else if (align & ALIGN_HCENTER)
    {
        horzAlign = "HCENTER";
    }
    else if (align & ALIGN_RIGHT)
    {
        horzAlign = "RIGHT";
    }

    if (align & ALIGN_TOP)
    {
        vertAlign = "TOP";
    }
    else if (align & ALIGN_VCENTER)
    {
        vertAlign = "VCENTER";
    }
    else if (align & ALIGN_BOTTOM)
    {
        vertAlign = "BOTTOM";
    }

    alignNode->Add(horzAlign);
    alignNode->Add(vertAlign);

    return alignNode;
}

YamlNode * UIYamlLoader::GetFittingOptionNodeValue( int32 fitting ) const
{
    YamlNode *fittingNode = YamlNode::CreateArrayNode(YamlNode::AR_FLOW_REPRESENTATION);

    if( fitting == TextBlock::FITTING_DISABLED )
    {
        fittingNode->Add("DISABLED");
    }
    else
    {
        if( fitting & TextBlock::FITTING_ENLARGE )
        {
            fittingNode->Add("ENLARGE");
        }
        if( fitting & TextBlock::FITTING_REDUCE )
        {
            fittingNode->Add("REDUCE");
        }
        if( fitting & TextBlock::FITTING_POINTS )
        {
            fittingNode->Add("POINTS");
        }
    }
    return fittingNode;
}

bool UIYamlLoader::GetBoolFromYamlNode(const YamlNode * node, bool defaultValue) const
{
    if (!node)return defaultValue;

    const String & value = node->AsString();
    if (value == "yes")return true;
    else if (value == "no")return false;
    else if (value == "true")return true;

    return false;
}

int32 HexCharToInt(char c)
{
    if ((c >= '0') && (c <= '9'))return c - '0';
    else if ((c >= 'a') && (c <= 'f'))return c - 'a' + 10;
    else if ((c >= 'A') && (c <= 'F'))return c - 'A' + 10;
    return 0;
}

Color UIYamlLoader::GetColorFromYamlNode(const YamlNode * node) const
{
    if (node->GetType() == YamlNode::TYPE_ARRAY)
    {
        if (node->GetCount() == 4)
            return node->AsColor();
        else return Color::White;
    }else
    {
        const String & color = node->AsString();

        int r = HexCharToInt(color[0]) * 16 + HexCharToInt(color[1]);
        int g = HexCharToInt(color[2]) * 16 + HexCharToInt(color[3]);
        int b = HexCharToInt(color[4]) * 16 + HexCharToInt(color[5]);
        int a = HexCharToInt(color[6]) * 16 + HexCharToInt(color[7]);

        return Color((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
    }
}


Font * UIYamlLoader::GetFontByName(const String & fontName) const
{
    return FontManager::Instance()->GetFont(fontName);
}

void UIYamlLoader::LoadFonts(const FilePath & yamlPathname)
{
    ScopedPtr<UIYamlLoader> loader( new UIYamlLoader() );
    YamlNode * rootNode = loader->CreateRootNode(yamlPathname);
    if (!rootNode)
    {
        // Empty YAML file.
        Logger::Warning("yaml file: %s is empty", yamlPathname.GetAbsolutePathname().c_str());
        return;
    }
    loader->LoadFontsFromNode(rootNode);
    SafeRelease(rootNode);
}

bool UIYamlLoader::SaveFonts(const FilePath & yamlPathname)
{
    const auto& fontMap = FontManager::Instance()->GetFontMap();
    ScopedPtr<YamlNode> fontsNode( new YamlNode(YamlNode::TYPE_MAP) );
    for (const auto &pair : fontMap)
    {
        Font* font = pair.second;
        if (nullptr == font)
            continue;
        fontsNode->AddNodeToMap(pair.first, font->SaveToYamlNode());
    }
    return YamlEmitter::SaveToYamlFile(yamlPathname, fontsNode, File::CREATE | File::WRITE);
}

void UIYamlLoader::Load(UIControl * rootControl, const FilePath & yamlPathname, bool assertIfCustomControlNotFound /* = true */)
{
    DefaultUIPackageBuilder builder;
    UIPackageLoader().LoadPackage(yamlPathname, &builder);
    if (builder.GetPackage())
    {
        DVASSERT(builder.GetPackage()->GetControlsCount() == 1);
        UIControl *control = builder.GetPackage()->GetControl(0);
        DVASSERT(control);
        while (!control->GetChildren().empty())
        {
            rootControl->AddControl(control->GetChildren().front());
        }

        if (rootControl->GetSize() != control->GetSize())
            rootControl->UpdateLayout();
        return;
    }

    UIYamlLoader * loader = new UIYamlLoader();
    loader->SetAssertIfCustomControlNotFound(assertIfCustomControlNotFound);

    loader->ProcessLoad(rootControl, yamlPathname);

    loader->Release();
}

bool UIYamlLoader::Save(UIControl * rootControl, const FilePath & yamlPathname, bool skipRootNode)
{
    UIYamlLoader * loader = new UIYamlLoader();

    bool savedOK = loader->ProcessSave(rootControl, yamlPathname, skipRootNode);

    loader->Release();
    return savedOK;
}

YamlNode *UIYamlLoader::CreateRootNode(const FilePath & yamlPathname)
{
    YamlParser * parser = YamlParser::Create(yamlPathname);
    if (!parser)
    {
        Logger::Error("Failed to open yaml file: %s", yamlPathname.GetAbsolutePathname().c_str());
        return NULL;
    }
    currentPath = yamlPathname.GetDirectory();
    YamlNode * rootNode = SafeRetain(parser->GetRootNode());
    SafeRelease(parser);
    return rootNode;
}

void UIYamlLoader::ProcessLoad(UIControl * rootControl, const FilePath & yamlPathname)
{
    uint64 t1 = SystemTimer::Instance()->AbsoluteMS();

    YamlNode * rootNode = CreateRootNode(yamlPathname);
    if (!rootNode)
    {
        // Empty YAML file.
        Logger::Warning("yaml file: %s is empty", yamlPathname.GetAbsolutePathname().c_str());
        return;
    }

    const YamlNode *childrenNode = rootNode->Get("children");
    if (!childrenNode)
        childrenNode = rootNode;

    LoadFromNode(rootControl, childrenNode, false);

    SafeRelease(rootNode);
	
	// After the scene is fully loaded, apply the align settings
	// to position child controls correctly.
    UIControlSystem::Instance()->GetLayoutSystem()->ApplyLayout(rootControl);
    
    PostLoad(rootControl);
    
	uint64 t2 = SystemTimer::Instance()->AbsoluteMS();
	Logger::FrameworkDebug("Load of %s time: %lld", yamlPathname.GetAbsolutePathname().c_str(), t2 - t1);    
}

void UIYamlLoader::PostLoad(UIControl * rootControl)
{
    //Find ScrollBars and set delegates
    SetScrollBarDelegates(rootControl);
}

void UIYamlLoader::SetScrollBarDelegates(UIControl * rootControl)
{
    Map<UIScrollBar*,String>::iterator it = scrollsToLink.begin();
    for (; it!=scrollsToLink.end(); ++it)
    {
        UIControl * control = UIControlHelpers::GetControlByPath(it->second, rootControl);
        it->first->SetDelegate( dynamic_cast<UIScrollBarDelegate*>(control));
    }
    scrollsToLink.clear();
}

bool UIYamlLoader::ProcessSave(UIControl * rootControl, const FilePath & yamlPathname, bool skipRootNode)
{
    uint64 t1 = SystemTimer::Instance()->AbsoluteMS();

    DVASSERT(rootControl);
    YamlNode* resultNode = SaveToNode(rootControl, NULL);

    uint32 fileAttr = File::CREATE | File::WRITE;

    // Save the resulting YAML file to the path passed.
    bool savedOK = YamlEmitter::SaveToYamlFile(yamlPathname, resultNode, fileAttr);

    SafeRelease(resultNode);

    uint64 t2 = SystemTimer::Instance()->AbsoluteMS();
    Logger::FrameworkDebug("Save of %s time: %lld", yamlPathname.GetAbsolutePathname().c_str(), t2 - t1);

    return savedOK;
}

void UIYamlLoader::LoadFontsFromNode(const YamlNode * rootNode)
{
    for (MultiMap<String, YamlNode*>::const_iterator t = rootNode->AsMap().begin(); t != rootNode->AsMap().end(); ++t)
    {
        YamlNode * node = t->second;
        
        Font* font = CreateFontFromYamlNode(node);

        if (font)
        {
            FontManager::Instance()->SetFontName(font, t->first);
            SafeRelease(font);
        }
    }
}

Font* UIYamlLoader::CreateFontFromYamlNode(const YamlNode* node)
{
    const YamlNode * typeNode = node->Get("type");
    if (!typeNode)
        return nullptr;

    const String & type = typeNode->AsString();
    Font* font = nullptr;

    if (type == "FTFont")
    {
        const YamlNode * fontNameNode = node->Get("name");
        if (!fontNameNode)
            return nullptr;

        font = FTFont::Create(fontNameNode->AsString());
        if (!font)
        {
            return nullptr;
        }
    }
    else if (type == "GraphicFont")
    {
        const YamlNode * fontNameNode = node->Get("name");
        if (!fontNameNode)
        {
            return nullptr;
        }

        const YamlNode * texNameNode = node->Get("texture");
        if (!fontNameNode)
        {
            return nullptr;
        }

        font = GraphicFont::Create(fontNameNode->AsString(), texNameNode->AsString());
        if (!font)
        {
            return nullptr;
        }
    }

    float32 fontSize = 10.0f;
    const YamlNode * fontSizeNode = node->Get("size");
    if (fontSizeNode)
        fontSize = fontSizeNode->AsFloat();

    font->SetSize(fontSize);
    
    const YamlNode * fontVerticalSpacingNode = node->Get("verticalSpacing");
    if (fontVerticalSpacingNode)
    {
        font->SetVerticalSpacing(fontVerticalSpacingNode->AsInt32());
    }

    const YamlNode * fontFontAscendNode = node->Get("ascendScale");
    if (fontFontAscendNode)
    {
        font->SetAscendScale(fontFontAscendNode->AsFloat());
    }

    const YamlNode * fontFontDescendNode = node->Get("descendScale");
    if (fontFontDescendNode)
    {
        font->SetDescendScale(fontFontDescendNode->AsFloat());
    }

    return font;
}

void UIYamlLoader::LoadFromNode(UIControl * parentControl, const YamlNode * rootNode, bool needParentCallback)
{
	//for (Map<String, YamlNode*>::iterator t = rootNode->AsMap().begin(); t != rootNode->AsMap().end(); ++t)
	int cnt = rootNode->GetCount();
	for (int k = 0; k < cnt; ++k)
	{
		const YamlNode * node = rootNode->Get(k);
		const YamlNode * typeNode = node->Get("type");
        if (!typeNode)
            continue;

		const String & type = typeNode->AsString();

		// Base Type might be absent.
		const YamlNode* baseTypeNode = node->Get("baseType");
		const String baseType = baseTypeNode ? baseTypeNode->AsString() : String();

		// The control can be loaded either from its Type or from Base Type (depending on
		// whether the control is custom or not.
		UIControl* control = CreateControl(type, baseType);
		if (!control)
		{
			Logger::Warning("ObjectFactory haven't found object with type:%s, base type %s", type.c_str(), baseType.c_str());
			continue;
		}else
		{
			//Logger::FrameworkDebug("Create control with type:%s", type.c_str());
		}

		control->SetName(rootNode->GetItemKeyName(k));
		control->LoadFromYamlNode(node, this);
		parentControl->AddControl(control);

		const YamlNode *childrenNode = node->Get("children");
		if (!childrenNode)
		    childrenNode = node;

		LoadFromNode(control, childrenNode, true);
		SafeRelease(control);
	}
    
    if(needParentCallback)
    {
        parentControl->LoadFromYamlNodeCompleted();
    }
}

UIControl* UIYamlLoader::CreateControl(const String& type, const String& baseType)
{
    // Firstly try Type (Custom Control).
    UIControl * control = dynamic_cast<UIControl*> (ObjectFactory::Instance()->New<UIControl>(type));
    if (control)
    {
        return control;
    }

    // The control can't be loaded by its type - probably it is Custom Control and we are
    // running under UIEditor or other app which doesn't support custom controls. Verify this.
    if (this->assertIfCustomControlNotFound)
    {
        Logger::Error("Unable to load UI Control %s and 'ASSERT if Custom Control Not Found' flag is set to TRUE", type.c_str());
        DVASSERT(false);
    }

    // Retry with base type, if any.
    if (!baseType.empty())
    {
        control = dynamic_cast<UIControl*> (ObjectFactory::Instance()->New<UIControl>(baseType));
    }

    // A NULL might be here too.
    return control;
}

YamlNode* UIYamlLoader::SaveToNode(UIControl * parentControl, YamlNode * parentNode)
{
    // Save ourselves and all children.
    YamlNode* childNode = parentControl->SaveToYamlNode(this);
    if (parentNode && parentNode->GetType() == YamlNode::TYPE_MAP)
    {
        parentNode->AddNodeToMap(parentControl->GetName(), childNode);
    }

    SaveChildren(parentControl, childNode);

    return childNode;
}

void UIYamlLoader::SaveChildren(UIControl* parentControl, YamlNode * parentNode)
{
    const List<UIControl*>& children = parentControl->GetChildren();
    if (children.empty())
        return;

    YamlNode *childrenNode = YamlNode::CreateMapNode(false);

    for (List<UIControl*>::const_iterator childIter = children.begin(); childIter != children.end(); childIter ++)
    {
        UIControl* childControl = (*childIter);
        SaveToNode(childControl, childrenNode);
    }

    parentNode->Add("children", childrenNode);
}

void UIYamlLoader::SetAssertIfCustomControlNotFound(bool value)
{
    this->assertIfCustomControlNotFound = value;
}

const FilePath & UIYamlLoader::GetCurrentPath() const
{
    return currentPath;
}

void UIYamlLoader::AddScrollBarToLink(UIScrollBar* scroll, const String& delegatePath)
{
    scrollsToLink.insert(std::pair<UIScrollBar*,String>(scroll,delegatePath));
}

}