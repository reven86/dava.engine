#pragma once

#include "Base/BaseTypes.h"
#include "FindFilter.h"
#include "FindItem.h"

class FileSystemCache;

class FindIterator
{
public:
    FindIterator();
    ~FindIterator();

    void CollectFiles(FileSystemCache* cache, const FindFilter& filter, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes);
    const DAVA::Vector<FindItem>& GetItems();

private:
    void CollectControls(const DAVA::FilePath& path, const std::shared_ptr<ControlInformation>& control, const FindFilter& filter, bool inPrototypeSection);

    DAVA::Vector<FindItem> items;
};
