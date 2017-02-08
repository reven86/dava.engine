#include "Finder.h"

#include "UI/Find/StaticPackageInformationBuilder.h"

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
        if (cancelling)
        {
            break;
        }

        FilePath path(pathStr.toStdString());
        StaticPackageInformationBuilder builder(&packagesCache);

        if (UIPackageLoader(*prototypes).LoadPackage(path, &builder))
        {
            const std::shared_ptr<PackageInformation>& package = builder.GetPackage();
            if (filter->CanAcceptPackage(package.get()))
            {
                currentItem = FindItem(package->GetPath());

                package->VisitControls(
                [this, &path](const ControlInformation* control)
                {
                    CollectControls(path, control, false);
                });
                package->VisitPrototypes(
                [this, &path](const ControlInformation* prototype)
                {
                    CollectControls(path, prototype, true);
                });

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
    cancelling = true;
}

void Finder::CollectControls(const FilePath& path, const ControlInformation* control, bool inPrototypeSection)
{
    if (filter->CanAcceptControl(control))
    {
        currentItem.AddPathToControl(ControlInformationHelpers::GetPathToControl(control));
    }

    control->VisitChildren(
    [this, &path, inPrototypeSection](const ControlInformation* child)
    {
        CollectControls(path, child, inPrototypeSection);
    });
}
