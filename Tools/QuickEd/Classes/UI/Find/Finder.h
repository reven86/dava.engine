#pragma once

#include "Base/BaseTypes.h"
#include "FindFilter.h"
#include "FindItem.h"

#include <QObject>

class Finder : public QObject
{
    Q_OBJECT

public:
    Finder(const QStringList& files_, std::unique_ptr<FindFilter>&& filter, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>* prototypes);
    ~Finder() override;

    void Process();
    void Stop();

signals:
    void Finished();
    void ItemFound(FindItem item);
    void ProgressChanged(int filesProcessed, int totalFiles);

private:
    void CollectControls(const DAVA::FilePath& path, const std::shared_ptr<ControlInformation>& control, bool inPrototypeSection);

    QStringList files;
    std::unique_ptr<FindFilter> filter;
    const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>* prototypes;
    FindItem currentItem;
    bool canceling = false;
};
