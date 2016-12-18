#pragma once

#include "Base/BaseTypes.h"
#include "FindFilter.h"
#include "FindItem.h"

#include <QObject>

class FileSystemCache;

class FindCollector : public QObject
{
    Q_OBJECT

public:
    FindCollector(const FileSystemCache* cache, std::unique_ptr<FindFilter> filter, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>* prototypes);
    ~FindCollector();

    const DAVA::Vector<FindItem>& GetItems() const;

public slots:
    void CollectFiles();

signals:
    void finished();

private:
    void CollectControls(const DAVA::FilePath& path, const std::shared_ptr<ControlInformation>& control, bool inPrototypeSection);

    const FileSystemCache* cache;
    std::unique_ptr<FindFilter> filter;
    const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>* prototypes;

    DAVA::Vector<FindItem> items;
};
