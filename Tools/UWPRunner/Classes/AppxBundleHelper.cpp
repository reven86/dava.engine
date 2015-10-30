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


#include "AppxBundleHelper.h"

#include <QFile>
#include <QXmlStreamReader>

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include "ArchiveExtraction.h"

using namespace DAVA;

AppxBundleHelper::AppxBundleHelper(const FilePath &fileName)
{
    String dirName = fileName.GetBasename() + "_files/";
    bundlePackageDir = fileName.GetDirectory() + dirName;

    Logger::Info("Extracting bundle...");
    bool result = ExtractAllFromArchive(fileName.GetAbsolutePathname(), 
                                        bundlePackageDir.GetAbsolutePathname());

    DVASSERT_MSG(result, "Failed to extract files from bundle");

    if (result)
    {
        ParseBundleManifest();
    }
}

AppxBundleHelper::~AppxBundleHelper()
{
    FileSystem::Instance()->DeleteDirectory(bundlePackageDir);
}

bool AppxBundleHelper::IsBundle(const FilePath &fileName)
{
    return fileName.GetExtension() == ".appxbundle";
}

FilePath AppxBundleHelper::GetApplication(const String& name)
{
    for (const auto& packageInfo : storedPackages)
    {
        if (packageInfo.isApplication && packageInfo.name == name)
        {
            return packageInfo.path;
        }
    }
    return FilePath();
}

FilePath AppxBundleHelper::GetApplicationForArchitecture(const String& name)
{
    String architecture = name;
    std::transform(architecture.begin(), architecture.end(), architecture.begin(), ::tolower);

    for (const auto& packageInfo : storedPackages)
    {
        if (packageInfo.isApplication && packageInfo.architecture == architecture)
        {
            return packageInfo.path;
        }
    }
    return FilePath();
}

const Vector<AppxBundleHelper::PackageInfo>& AppxBundleHelper::GetPackages() const
{
    return storedPackages;
}

Vector<AppxBundleHelper::PackageInfo> AppxBundleHelper::GetApplications() const
{
    Vector<PackageInfo> applications;

    for (const auto& packageInfo : storedPackages)
    {
        if (packageInfo.isApplication)
        {
            applications.push_back(packageInfo);
        }
    }

    return applications;
}

Vector<AppxBundleHelper::PackageInfo> AppxBundleHelper::GetResources() const
{
    Vector<PackageInfo> resources;

    for (const auto& packageInfo : storedPackages)
    {
        if (!packageInfo.isApplication)
        {
            resources.push_back(packageInfo);
        }
    }

    return resources;
}

void AppxBundleHelper::ParseBundleManifest()
{
    FilePath manifest = bundlePackageDir + "AppxMetadata/AppxBundleManifest.xml";

    QFile file(QString::fromStdString(manifest.GetAbsolutePathname()));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token != QXmlStreamReader::StartElement ||
            xml.name() != QStringLiteral("Package"))
        {
            continue;
        }

        QXmlStreamAttributes attributes = xml.attributes();
        PackageInfo packageInfo;
        packageInfo.architecture = "any";
        packageInfo.isApplication = false;
            
        for (const auto& attribute : attributes)
        {
            if (attribute.name() == QStringLiteral("Type"))
            {
                QString value = attribute.value().toString().toLower();
                packageInfo.isApplication = value == QStringLiteral("application");
            }
            else if (attribute.name() == QStringLiteral("Architecture"))
            {
                packageInfo.architecture = attribute.value().toString().toLower().toStdString();
            }
            else if (attribute.name() == QStringLiteral("FileName"))
            {
                String fileName = attribute.value().toString().toStdString();
                packageInfo.name = fileName;
                packageInfo.path = bundlePackageDir + fileName;
            }
        }
        storedPackages.emplace_back(std::move(packageInfo));
    }
}