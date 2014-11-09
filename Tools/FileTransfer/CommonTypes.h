#ifndef __DAVAENGINE_COMMONTYPES_H__
#define __DAVAENGINE_COMMONTYPES_H__

#if defined(__DAVAENGINE_WIN32__) && defined(_MSC_VER)
#define SIZET_FMT   "%Iu"
#else
#define SIZET_FMT   "%zu"
#endif

namespace DAVA
{

struct InitRequest
{
    uint32 sign;
    uint32 fileSize;
};

struct InitReply
{
    uint32 sign;
    uint32 status;
};

struct FileReply
{
    uint32 sign;
    uint32 status;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_COMMONTYPES_H__
