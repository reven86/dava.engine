#ifndef __DAVAENGINE_COMMONTYPES_H__
#define __DAVAENGINE_COMMONTYPES_H__

#if defined(__DAVAENGINE_WIN32__) && defined(_MSC_VER)
#define SIZET_FMT   "%Iu"
#else
#define SIZET_FMT   "%zu"
#endif

namespace DAVA
{

enum {
    PROTO_PING,
    PROTO_PONG,
    PROTO_CHUNK,
    PROTO_ACK
};

struct ProtoHeader
{
    uint32 size;
    uint32 type;
};

struct ProtoPing
{
    ProtoHeader hdr;
    uint32      fileSize;
};

struct ProtoPong
{
    ProtoHeader hdr;
};

struct ProtoChunk
{
    ProtoHeader hdr;
    uint32      chunkSize;
};

struct ProtoAck
{
    ProtoHeader hdr;
};

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
