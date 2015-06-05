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


#ifndef __DAVAENGINE_YAML_LOADER_H__
#define __DAVAENGINE_YAML_LOADER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
/**
    \ingroup controlsystem
    \brief Class to perform loading of controls from yaml configs

    This class can help you to load hierarhy of controls from yaml config file.
    Structure of yaml file is very simple and it allow you to modify the structure of the application easily using
    configuration files.
*/

class UIControl;
class UIScrollBar;
class YamlNode;
class Font;

class UIYamlLoader : public BaseObject
{
    friend class UIPackageLoader;
protected:
    ~UIYamlLoader(){}
    UIYamlLoader();
public:
    /**
     \brief	This is main function in UIYamlLoader and it loads fonts from yamlPathname file.

     \param[in] yamlPathName						we get config file using this pathname
     */
    static void LoadFonts(const FilePath & yamlPathname);
    static Font* CreateFontFromYamlNode(const YamlNode* node);

    /**
     \brief	This function saves fonts to the YAML file passed.

     \param[in]			yamlPathName	path to store fonts to
     \return            true if the save was successful
     */
    static bool SaveFonts(const FilePath & yamlPathname);
    /**
        \brief	This is main function in UIYamlLoader and it loads control hierarchy from yamlPathname file and add it to
                rootControl.

        \param[in, out]	rootControl					we add all created control classes to this control
        \param[in] yamlPathName						we get config file using this pathname
        \param[in] assertIfCustomControlNotFound	if this flag is set to true, ASSERT and stop app execution if the
                                                    custom control can't be loaded.
     */
    static void Load(UIControl * rootControl, const FilePath & yamlPathname, bool assertIfCustomControlNotFound = true);

    /**
     \brief	This function saves the UIControl's hierarchy to the YAML file passed.
     rootControl.

     \param[in, out]	rootControl		is used to take the configuration from
     \param[in]			yamlPathName	path to store hierarchy to
     \return            true if the save was successful
     */
    static bool Save(UIControl * rootControl, const FilePath & yamlPathname, bool skipRootNode);

    Font * GetFontByName(const String & fontName) const;

    int32 GetDrawTypeFromNode(const YamlNode * drawTypeNode) const;
    int32 GetColorInheritTypeFromNode(const YamlNode * colorInheritNode) const;
    int32 GetPerPixelAccuracyTypeFromNode(const YamlNode * perPixelAccuracyNode) const;
    int32 GetAlignFromYamlNode(const YamlNode * align) const;
    int32 GetFittingOptionFromYamlNode(const YamlNode * fittingNode) const;
    bool GetBoolFromYamlNode(const YamlNode * node, bool defaultValue) const;
    Color GetColorFromYamlNode(const YamlNode * node) const;

    String GetColorInheritTypeNodeValue(int32 colorInheritType) const;
    String GetPerPixelAccuracyTypeNodeValue(int32 perPixelAccuracyType) const;
    String GetDrawTypeNodeValue(int32 drawType) const;
    YamlNode * GetAlignNodeValue(int32 align) const;
    YamlNode * GetFittingOptionNodeValue(int32 fitting) const;

    void AddScrollBarToLink(UIScrollBar* scroll,const String& delegatePath);

    inline bool GetAssertIfCustomControlNotFound() const;

protected:
    //Internal functions that do actual loading and saving.
    void ProcessLoad(UIControl * rootControl, const FilePath & yamlPathname);
    YamlNode *CreateRootNode(const FilePath & yamlPathname);
    void LoadFontsFromNode(const YamlNode * node);
    void LoadFromNode(UIControl * rootControl, const YamlNode * node, bool needParentCallback);

    YamlNode* SaveToNode(UIControl * parentControl, YamlNode * rootNode);
    void SaveChildren(UIControl* parentControl, YamlNode * parentNode);

    bool ProcessSave(UIControl * rootControl, const FilePath & yamlPathname, bool skipRootNode);

    // Set the "ASSERT if custom control is not found during loading" flag.
    void SetAssertIfCustomControlNotFound(bool value);

    const FilePath & GetCurrentPath() const;

protected:
	// Create the control by its type or base type.
	UIControl* CreateControl(const String& type, const String& baseType);

    //Called after loading
    void PostLoad(UIControl * rootControl);
    void SetScrollBarDelegates(UIControl * rootControl);

    // ASSERTion flag for "Custom Control not found" state.
    bool assertIfCustomControlNotFound;

	FilePath currentPath;

    Map<UIScrollBar*,String> scrollsToLink;    
};
    
inline bool UIYamlLoader::GetAssertIfCustomControlNotFound() const
{
    return assertIfCustomControlNotFound;
}

};

#endif // __DAVAENGINE_YAML_LOADER_H__