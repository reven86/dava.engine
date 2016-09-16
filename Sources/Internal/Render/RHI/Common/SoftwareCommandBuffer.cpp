#include "SoftwareCommandBuffer.h"

namespace rhi
{
const uint64 SoftwareCommandBufferUnpacked::EndCmd = 0xFFFFFFFF;
void SoftwareCommandBuffer::Begin()
{
    curUsedSize = 0;
}

void SoftwareCommandBuffer::End()
{
}

void SoftwareCommandBufferUnpacked::Begin()
{
    _cmd.clear();
}

void SoftwareCommandBufferUnpacked::End()
{
    _cmd.push_back(EndCmd);
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd)
{
    _cmd.push_back(cmd);
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd, uint64 arg1)
{
    _cmd.resize(_cmd.size() + 1 + 1);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 1);

    b[0] = cmd;
    b[1] = arg1;
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd, uint64 arg1, uint64 arg2)
{
    _cmd.resize(_cmd.size() + 1 + 2);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 2);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3)
{
    _cmd.resize(_cmd.size() + 1 + 3);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 3);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4)
{
    _cmd.resize(_cmd.size() + 1 + 4);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 4);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    b[4] = arg4;
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5)
{
    _cmd.resize(_cmd.size() + 1 + 5);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 5);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    b[4] = arg4;
    b[5] = arg5;
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6)
{
    _cmd.resize(_cmd.size() + 1 + 6);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 6);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    b[4] = arg4;
    b[5] = arg5;
    b[6] = arg6;
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6, uint64 arg7)
{
    _cmd.resize(_cmd.size() + 1 + 7);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 7);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    b[4] = arg4;
    b[5] = arg5;
    b[6] = arg6;
    b[7] = arg7;
}

void SoftwareCommandBufferUnpacked::Command(uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6, uint64 arg7, uint64 arg8)
{
    _cmd.resize(_cmd.size() + 1 + 8);

    std::vector<uint64>::iterator b = _cmd.end() - (1 + 8);

    b[0] = cmd;
    b[1] = arg1;
    b[2] = arg2;
    b[3] = arg3;
    b[4] = arg4;
    b[5] = arg5;
    b[6] = arg6;
    b[7] = arg7;
    b[8] = arg8;
}
}