#pragma once

#include <iosfwd>

namespace DAVA
{
class ProfilerCPU;
class ProfilerGPU;
class FilePath;
namespace ProfilerDump
{
void DumpCPUGPUTrace(ProfilerCPU* cpuProfiler, ProfilerGPU* gpuProfiler, std::ostream& stream);
void DumpCPUGPUTraceToFile(ProfilerCPU* cpuProfiler, ProfilerGPU* gpuProfiler, const FilePath& filePath);
}

}; //ns DAVA