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

    void CollectFiles(FileSystemCache* cache, const FindFilter& filter);

private:
    DAVA::Vector<FindItem> findItems;
};
