#pragma once

#include "Analytics/Analytics.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
namespace Analytics
{
/**
Simple analytics events logger. Log to console and file (if set)
*/
class LoggingBackend : public IBackend
{
public:
    LoggingBackend(const FilePath& path = FilePath());

    void ConfigChanged(const KeyedArchive& config) override;
    void ProcessEvent(const AnalyticsEvent& event) override;

private:
    FilePath filePath;
};

} // namespace Analytics
} // namespace DAVA
