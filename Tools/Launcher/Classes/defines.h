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


#ifndef DEFINES_H
#define DEFINES_H

#include <QtGlobal>

////Global launcher defines

#define LAUNCHER_VER "0.9 build 11"

#define LOCAL_CONFIG_NAME "localConfig.yaml"

////YAML config keys

#define CONFIG_STRINGS_KEY "strings"
#define CONFIG_LAUNCHER_KEY "launcher"
#define CONFIG_BRANCHES_KEY "branches"

#define CONFIG_LAUNCHER_VERSION_KEY "version"
#define CONFIG_LAUNCHER_WEBPAGE_KEY "webpage"
#define CONFIG_LAUNCHER_NEWSID_KEY "newsID"
#define CONFIG_LAUNCHER_REMOTE_URL_KEY "remoteConfigUrl"
#define CONFIG_LAUNCHER_FAVORITES_KEY "favorites"

#define CONFIG_APPVERSION_RUNPATH_KEY "runpath"
#define CONFIG_APPVERSION_CMD_KEY "cmd"

#define CONFIG_URL_KEY "url"

////Application table defines

#define COLUMN_APP_NAME 0
#define COLUMN_APP_INS 1
#define COLUMN_APP_AVAL 2
#define COLUMN_BUTTONS 3

#define DAVA_CUSTOM_PROPERTY_NAME "DAVA_ID"

#ifdef Q_OS_WIN
#define TABLE_STYLESHEET "QComboBox {margin-top: 7px; margin-bottom: 7px; padding-left: 5px;} QLabel {padding-left: 4px;padding-right: 4px;}"
#elif defined(Q_OS_DARWIN)
#define TABLE_STYLESHEET "QLabel {padding-left: 4px;padding-right: 4px;}"
#endif

////Update dialog defines

#define LOG_COLOR_COMPLETE QColor(0, 110, 0)
#define LOG_COLOR_PROGRESS QColor(0, 0, 110)
#define LOG_COLOR_FAIL QColor(240, 0, 0)

/////////////////////

template <class TYPE>
void SafeDelete(TYPE * &d)
{
    if (d)
    {
        delete d;
        d = 0;
    }
}

#endif // DEFINES_H
