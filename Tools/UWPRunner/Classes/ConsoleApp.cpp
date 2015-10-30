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
#include <QFileInfo>

#include "Core/Core.h"
#include "FileSystem/FileSystem.h"
#include "TexturePacker/CommandLineParser.h"
#include "Utils/Utils.h"

#include "UWPRunner.h"

using namespace DAVA;

void ShowUsage();
PackageOptions ParseShortFormArgs(const String& packagePath);
PackageOptions ParseLongFormArgs(const Vector<String>& arguments);

PackageOptions ParseCommandLine()
{
    const Vector<String>& arguments = Core::Instance()->GetCommandLine();

    //no args
    if (arguments.size() == 1)
    {
        return PackageOptions();
    }
    //short form (only file)
    else if (arguments.size() == 2)
    {
        return ParseShortFormArgs(arguments[1]);
    }
    //long form
    else
    {
        return ParseLongFormArgs(arguments);
    }
}

PackageOptions ParseShortFormArgs(const String& packagePath)
{
    Vector<String> packageInfo;
    Split(FilePath(packagePath).GetBasename(), "_", packageInfo);

    PackageOptions options;
    options.package = packagePath;
    options.dependencies = FilePath(packagePath).GetDirectory().GetAbsolutePathname();

    return options;
}

PackageOptions ParseLongFormArgs(const Vector<String>& arguments)
{
    PackageOptions out;

    CommandLineParser parser;
    parser.SetArguments(arguments);

    //parse parameters 'arch', 'package', 'profile' and 'dependencies'
    if (parser.IsFlagSet("--arch"))
    {
        out.architecture = parser.GetParamForFlag("--arch");
    }

    if (parser.IsFlagSet("--package"))
    {
        out.package = parser.GetParamForFlag("--package");
    }

    if (parser.IsFlagSet("--profile"))
    {
        String profile = parser.GetParamForFlag("--profile");
        std::transform(profile.begin(), profile.end(), profile.begin(), ::tolower);
        out.profile = std::move(profile);
    }

    if (parser.IsFlagSet("--dependencies"))
    {
        out.dependencies = parser.GetParamForFlag("--dependencies");
    }
    else if (!out.package.empty())
    {
        out.dependencies = FilePath(out.package).GetDirectory().GetAbsolutePathname();
    }

    if (parser.IsFlagSet("--tc_test"))
    {
        out.useTeamCityTestOutput = true;
    }

    return out;
}

void ShowUsage()
{
    String message = 
        "UWPRunner is a utility for installing, running and collection output "
        "of universal windows applications.\n"
        "UWPRunner may need administrative rights for configuring of IpOverUsb service\n"
        "Usage: \n"
        "    --package [path to appx package]\n"
        "    --dependencies [path to package dependencies dir]\n"
        "    --profile (local/phone) [target device for package]\n"
        "    --arch [architecture of launching package, only for bundle]\n"
        "    --tc_test [use teamcity test output]\n";

    std::cout << message;
}

bool CheckPackageOption(const String& package)
{
    QFileInfo file(QString::fromStdString(package));

    if (package.empty())
    {
        std::cout << "Package file is not set";
        return false;
    }
    else if (!file.exists() || !file.isFile())
    {
        std::cout << "Cannot find the specified file: " << package;
        return false;
    }

    return true;
}

bool CheckDependenciesOption(const String& dependencies)
{
    FileSystem* fs = FileSystem::Instance();

    if (dependencies.empty())
    {
        std::cout << "Package dependencies path is not set";
        return false;
    }
    else if (!fs->IsDirectory(dependencies))
    {
        std::cout << "Cannot find the specified path: " << dependencies;
        return false;
    }

    return true;
}

bool CheckOptions(const PackageOptions& options)
{
    bool packageIsOk = CheckPackageOption(options.package);
    bool dependenciesIsOk = CheckDependenciesOption(options.dependencies);

    bool allIsOk = packageIsOk && dependenciesIsOk;

    if (!allIsOk)
        ShowUsage();

    return allIsOk;
}