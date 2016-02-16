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


#ifndef APPX_BUNDLE_HELPER_H
#define APPX_BUNDLE_HELPER_H

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

class AppxBundleHelper
{
public:
    struct PackageInfo
    {
        DAVA::String name;
        DAVA::String architecture;
        bool isApplication;
        DAVA::FilePath path;
    };

    AppxBundleHelper(const DAVA::FilePath& fileName);
    ~AppxBundleHelper();

    void RemoveFiles();

    static bool IsBundle(const DAVA::FilePath& fileName);
    const DAVA::Vector<PackageInfo>& GetPackages() const;
    DAVA::Vector<PackageInfo> GetApplications() const;
    DAVA::Vector<PackageInfo> GetResources() const;

    DAVA::FilePath GetApplication(const DAVA::String& name);
    DAVA::FilePath GetApplicationForArchitecture(const DAVA::String& name);
    DAVA::FilePath GetResource(const DAVA::String& name);

private:
    void ParseBundleManifest();

    DAVA::FilePath bundlePackageDir;
    DAVA::Vector<PackageInfo> storedPackages;
};

#endif // APPX_BUNDLE_HELPER_H