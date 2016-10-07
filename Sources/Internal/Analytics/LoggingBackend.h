#pragma once

#include "Analytics/Analytics.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
namespace Analytics
{
class LoggingBackend : public IBackend
{
public:
    LoggingBackend(const FilePath& path = FilePath());

    void ConfigChanged(const KeyedArchive& config) override;
    void ProcessEvent(const EventRecord& event) override;

private:
    FilePath filePath;
};

} // namespace Analytics
} // namespace DAVA
