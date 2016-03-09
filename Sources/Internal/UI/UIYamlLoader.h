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

class YamlNode;
class Font;

class UIYamlLoader : public BaseObject
{
protected:
    ~UIYamlLoader() = default;
    UIYamlLoader() = default;

public:
    /**
     \brief	This is main function in UIYamlLoader and it loads fonts from yamlPathname file.

     \param[in] yamlPathName						we get config file using this pathname
     */
    static void LoadFonts(const FilePath& yamlPathname);
    static Font* CreateFontFromYamlNode(const YamlNode* node);

    /**
     \brief	This function saves fonts to the YAML file passed.

     \param[in]			yamlPathName	path to store fonts to
     \return            true if the save was successful
     */
    static bool SaveFonts(const FilePath& yamlPathname);

    Font* GetFontByName(const String& fontName) const;

protected:
    //Internal functions that do actual loading and saving.
    YamlNode* CreateRootNode(const FilePath& yamlPathname);
    void LoadFontsFromNode(const YamlNode* node);
};
};

#endif // __DAVAENGINE_YAML_LOADER_H__