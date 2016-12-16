#pragma once

#include "Base/BaseTypes.h"
#include "FindFilter.h"
#include "FindItem.h"

class FileSystemCache;

class FindCollector
{
    Q_OBJECT

public:
    FindCollector(const FileSystemCache* cache, const FindFilter& filter, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes);
    ~FindCollector();

    void CollectFiles();
    const DAVA::Vector<FindItem>& GetItems() const;

signals:
    void finished();

private:
    void CollectControls(const DAVA::FilePath& path, const std::shared_ptr<ControlInformation>& control, const FindFilter& filter, bool inPrototypeSection);

    const FileSystemCache* cache;
    const FindFilter& filter;
    const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes;

    DAVA::Vector<FindItem> items;
};
