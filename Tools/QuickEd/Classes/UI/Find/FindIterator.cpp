#include "FindIterator.h"

#include "QtTools/ProjectInformation/FileSystemCache.h"

#include "UI/Find/PackageInformationBuilder.h"

#include "UI/UIPackageLoader.h"

using namespace DAVA;

FindIterator::FindIterator()
{
}

FindIterator::~FindIterator()
{
}

void FindIterator::CollectFiles(FileSystemCache* cache, const FindFilter& filter)
{
    QStringList files = cache->GetFiles("yaml");
    int index = 0;

    PackageInformationCache packagesCache;

    for (QString& pathStr : files)
    {
        FilePath path(pathStr.toStdString());
        index++;
        int pct = (index * 100) / files.size();
        if (path.GetFrameworkPath().find("~res:/UI/TechTree/") == -1 &&
            path.GetFrameworkPath().find("~res:/UI/Fonts/") == -1)
        {
            PackageInformationBuilder builder(&packagesCache);

            if (UIPackageLoader().LoadPackage(path, &builder))
            {
                //Logger::Debug("  [loaded]");

                if (filter.CanAcceptPackage(builder))
                {
                    //findItems.push_back(FindItem(path, "", true));
                }
                else
                {
                }
            }
            else
            {
                DVASSERT(false);
                Logger::Debug("  [failed]");
            }
        }
    }
}
