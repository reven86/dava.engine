#include "Finder.h"

#include "UI/Find/PackageInformationBuilder.h"

#include "UI/UIPackageLoader.h"

using namespace DAVA;

Finder::Finder(const QStringList& files_, std::unique_ptr<FindFilter>&& filter_, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>* prototypes_)
    : files(files_)
    , filter(std::move(filter_))
    , prototypes(prototypes_)
{
}

Finder::~Finder()
{
}

void Finder::Process()
{
    files.sort();

    PackageInformationCache packagesCache;

    int filesProcessed = 0;
    for (const QString& pathStr : files)
    {
        QMutexLocker locker(&mutex);
        if (canceling)
        {
            break;
        }

        FilePath path(pathStr.toStdString());
        PackageInformationBuilder builder(&packagesCache);

        if (UIPackageLoader(*prototypes).LoadPackage(path, &builder))
        {
            const std::shared_ptr<PackageInformation>& package = builder.GetPackage();
            if (filter->CanAcceptPackage(package))
            {
                currentItem = FindItem(package->GetPath());

                for (const std::shared_ptr<ControlInformation>& control : package->GetControls())
                {
                    CollectControls(path, control, false);
                }
                for (const std::shared_ptr<ControlInformation>& prototype : package->GetPrototypes())
                {
                    CollectControls(path, prototype, true);
                }

                if (!currentItem.GetControlPaths().empty())
                {
                    emit ItemFound(currentItem);
                }
            }
        }
        filesProcessed++;
        emit ProgressChanged(filesProcessed, files.size());
    }

    emit Finished();
}

void Finder::Stop()
{
    QMutexLocker locker(&mutex);
    canceling = true;
}

void Finder::CollectControls(const FilePath& path, const std::shared_ptr<ControlInformation>& control, bool inPrototypeSection)
{
    if (filter->CanAcceptControl(control))
    {
        currentItem.AddPathToControl(control->GetPathToControl());
    }

    for (const std::shared_ptr<ControlInformation>& child : control->GetChildren())
    {
        CollectControls(path, child, inPrototypeSection);
    }
}
