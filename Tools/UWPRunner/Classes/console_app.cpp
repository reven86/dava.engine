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


#include <iostream>

#include "Core/Core.h"
#include "FileSystem/FileSystem.h"
#include "TexturePacker/CommandLineParser.h"
#include "Utils/Utils.h"

#include "uwp_runner.h"

using namespace DAVA;

PackageOptions ParseCommandLine()
{
    PackageOptions out;

    CommandLineParser parser;
    parser.SetArguments(Core::Instance()->GetCommandLine());

    //parse parameters 'package', 'profile' and 'dependencies'
    if (parser.IsFlagSet("--package"))
        out.package = parser.GetParamForFlag("--package");

    if (parser.IsFlagSet("--profile"))
    {
        String profile = parser.GetParamForFlag("--profile");
        std::transform(profile.begin(), profile.end(), profile.begin(), ::tolower);
        out.profile = std::move(profile);
    }

    if (parser.IsFlagSet("--dependencies"))
        out.dependencies = parser.GetParamForFlag("--dependencies");

    return out;
}

void ShowUsage()
{

}

bool CheckPackageOption(const Optional<String>& package)
{
    FileSystem* fs = FileSystem::Instance();

    if (!package.IsSet())
    {
        std::cout << "Package file is not set";
        return false;
    }
    else if (!fs->IsFile(package.Get()))
    {
        std::cout << "Cannot find the specified file: " << package.Get();
        return false;
    }
    else if (FilePath(package.Get()).GetExtension() != ".appx")
    {
        std::cout << "File is not a UWP package: " << package.Get();
        return false;
    }

    return true;
}

bool CheckProfileOption(const Optional<String>& profile)
{
    return profile == "local" || profile == "phone";
}

bool CheckDependenciesOption(const Optional<String>& dependencies)
{
    FileSystem* fs = FileSystem::Instance();

    if (!dependencies.IsSet())
    {
        std::cout << "Package dependencies path is not set";
        return false;
    }
    else if (!fs->IsDirectory(dependencies.Get()))
    {
        std::cout << "Cannot find the specified path: " << dependencies.Get();
        return false;
    }

    return true;
}

bool CheckOptions(const PackageOptions& options)
{
    bool packageIsOk = CheckPackageOption(options.package);
    bool profileIsOk = CheckProfileOption(options.profile);
    bool dependenciesIsOk = CheckDependenciesOption(options.dependencies);

    bool allIsOk = packageIsOk && profileIsOk && dependenciesIsOk;

    if (!allIsOk)
        ShowUsage();

    return allIsOk;
}