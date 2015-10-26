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


#include <Base/BaseTypes.h>
#include <iostream>

#include "FontConvertor.h"
#include "TtfFont.h"

using namespace std;
using namespace DAVA;

Vector<String> split(const String& s, char8 delim)
{
    Vector<String> elems;
    stringstream ss(s);
    String item;
    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

void Usage()
{
    cout << endl;
    cout << "Usage: fontgenerator font=ttf_font_file [params...]" << endl;
    cout << endl;
    cout << "Params:" << endl;
    cout << endl;
    cout << "charlist=file_name - file in utf8 which contains set of characters to convert" << endl;
    cout << endl;
    cout << "maxchar=N - specifies maximum character code" << endl;
    cout << "            Will generate all available characters from 0 to N" << endl;
    cout << "            Default: 128. If charlist param is set - maxchar is ignored" << endl;
    cout << endl;
    cout << "spread=N - specifies spread value. Default: 2" << endl;
    cout << "           The spread specifies how many pixels the distance field should" << endl;
    cout << "           extend outside the character outline before it clamps to zero" << endl;
    cout << endl;
    cout << "scale=N - specifies scale value. Default: 16" << endl;
    cout << "          Original image will be N times bigger than resulting image" << endl;
    cout << "          Bigger scale = higher resulting image quality" << endl;
    cout << "          (Ignored for graphic font)" << endl;
    cout << endl;
    cout << "mode=(generate | adjustfont | adjusttexture) - specifies operation mode" << endl;
    cout << "         generate - creates output with specified texture and font sizes" << endl;
    cout << "         adjustfont - find maximum font size which fits specified texture size" << endl;
    cout << "         adjusttexture -  find minimum texture size which can contain all" << endl;
    cout << "                          the characters with specified font size" << endl;
    cout << "         Default: generate" << endl;
    cout << endl;
    cout << "fontsize=N - font size in pixels. Default: 32" << endl;
    cout << endl;
    cout << "texturesize=N - texture size. Default: 512" << endl;
    cout << endl;
    cout << "charmap=N - char map from TTF file. Values are 0..charmapCount-1. Default: 0" << endl;
    cout << endl;
    cout << "output=(dff | gf) - specifies output file format. Default: dff" << endl;
    cout << "         dff - distance field font" << endl;
    cout << "         gf - anti-aliasing graphic font" << endl;
    cout << endl;
    cout << endl;
}

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        Usage();
        return -1;
    }

    FontConvertor::Params params;

    for (auto i = 1; i < argc; ++i)
    {
        auto val = split(argv[i], '=');
        if (val.size() != 2)
        {
            cout << "Invalid token: " << val[0] << endl;
            Usage();
            return -1;
        }

        try
        {
            if (val[0] == "font")
            {
                params.filename = val[1];
            }
            else if (val[0] == "maxchar")
            {
                params.maxChar = stoi(val[1]);
            }
            else if (val[0] == "spread")
            {
                params.spread = stoi(val[1]);
            }
            else if (val[0] == "scale")
            {
                params.scale = stoi(val[1]);
            }
            else if (val[0] == "mode")
            {
                params.mode = FontConvertor::ModeFromString(val[1]);
            }
            else if (val[0] == "fontsize")
            {
                params.fontSize = stoi(val[1]);
            }
            else if (val[0] == "texturesize")
            {
                params.textureSize = stoi(val[1]);
            }
            else if (val[0] == "charmap")
            {
                params.charmap = stoi(val[1]);
            }
            else if (val[0] == "charlist")
            {
                params.charListFile = val[1];
            }
            else if (val[0] == "output")
            {
                params.output = FontConvertor::TypeFromString(val[1]);
            }
            else
            {
                cout << "Invalid token: " << val[0] << endl;
                Usage();
                return -1;
            }
        }
        catch (const invalid_argument&)
        {
            cerr << "Invalid argument" << endl;
            Usage();
            return -1;
        }
    }

    if (params.filename == "")
    {
        Usage();
        return -1;
    }

    FontConvertor f;
    f.InitWithParams(params);
    if (!f.Convert())
    {
        return -1;
    }

    return 0;
}
