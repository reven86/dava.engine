/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __PROFILINGSESSION_H__
#define __PROFILINGSESSION_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"
#include "MemoryManager/MemoryManagerTypes.h"

namespace DAVA { namespace Net {
class PeerDescription;
}}

class StatItem final
{
public:
    StatItem(const DAVA::MMCurStat* rawStat, size_t poolCount, size_t tagCount)
    {
        Init(rawStat, poolCount, tagCount);
    }
    // TODO: implemet move semantic

    DAVA::uint64 Timestamp() const { return timestamp; }
    const DAVA::GeneralAllocStat& GeneralStat() const { return statGeneral; }
    const DAVA::Vector<DAVA::AllocPoolStat>& PoolStat() const { return statPools; }
    const DAVA::Vector<DAVA::TagAllocStat>& TagStat() const { return statTags; }

    const DAVA::AllocPoolStat& TotalStat() const { return statPools[0]; }

private:
    void Init(const DAVA::MMCurStat* rawStat, size_t poolCount, size_t tagCount);

private:
    DAVA::uint64 timestamp;
    DAVA::GeneralAllocStat statGeneral;
    DAVA::Vector<DAVA::AllocPoolStat> statPools;
    DAVA::Vector<DAVA::TagAllocStat> statTags;
};

class ProfilingSession
{
public:
    ProfilingSession(const DAVA::MMStatConfig* config, const DAVA::Net::PeerDescription& devInfo);
    ~ProfilingSession() = default;
    // TODO: implemet move semantic

    void AddStatItem(const DAVA::MMCurStat* rawStat);

    size_t AllocPoolCount() const { return allocPoolCount; }
    size_t TagCount() const { return tagCount; }
    size_t StatCount() const { return stat.size(); }

    const DAVA::String& AllocPoolName(size_t index) const;
    const DAVA::String& TagName(size_t index) const;
    const StatItem& Stat(size_t index) const;
    const StatItem& LastStat() const;

private:
    void Init(const DAVA::MMStatConfig* config);

private:
    size_t allocPoolCount = 0;
    size_t tagCount = 0;
    const DAVA::Net::PeerDescription& deviceInfo;
    DAVA::Vector<DAVA::String> allocPoolNames;
    DAVA::Vector<DAVA::String> tagNames;
    DAVA::Vector<StatItem> stat;
};

//////////////////////////////////////////////////////////////////////////
inline const DAVA::String& ProfilingSession::AllocPoolName(size_t index) const
{
    DVASSERT(0 <= index && index < allocPoolCount);
    return allocPoolNames[index];
}

inline const DAVA::String& ProfilingSession::TagName(size_t index) const
{
    DVASSERT(0 <= index && index < tagCount);
    return tagNames[index];
}

inline const StatItem& ProfilingSession::Stat(size_t index) const
{
    DVASSERT(0 <= index && index < stat.size());
    return stat[index];
}

inline const StatItem& ProfilingSession::LastStat() const
{
    DVASSERT(!stat.empty());
    return stat.back();
}

#endif  // __PROFILINGSESSION_H__
