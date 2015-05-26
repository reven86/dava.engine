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

#include "Base/RefPtr.h"
#include "Debug/DVAssert.h"
#include "FileSystem/File.h"

#include "BacktraceSymbolTable.h"
#include "MemorySnapshot.h"

using namespace DAVA;

bool MemorySnapshot::Load(BacktraceSymbolTable* symbolTable_)
{
    DVASSERT(symbolTable_ != nullptr);
    symbolTable = symbolTable_;

    if (LoadFile())
    {

    }
    return false;
}

void MemorySnapshot::Unload()
{
    symbolTable = nullptr;
    blockMap.clear();
    mblocks.clear();
    mblocks.shrink_to_fit();
}

bool MemorySnapshot::LoadFile()
{
    RefPtr<File> file(File::Create(fileName, File::OPEN | File::READ));
    if (file.Valid())
    {
        // Load and check file header
        MMDump rawDump;
        size_t nread = file->Read(&rawDump);
        if (sizeof(MMDump) == nread || file->GetSize() == rawDump.size)
        {
            // Load and skip statistics
            MMCurStat curStat;
            nread = file->Read(&curStat, sizeof(curStat));
            if (sizeof(MMCurStat) == nread)
            {
                file->Seek(curStat.size - sizeof(MMCurStat), File::SEEK_FROM_CURRENT);

                const uint32 bktraceSize = sizeof(MMBacktrace) + rawDump.bktraceDepth * sizeof(uint64);
                Vector<MMBlock> blocks(rawDump.blockCount, MMBlock());
                Vector<MMSymbol> symbols(rawDump.symbolCount, MMSymbol());
                Vector<uint8> bktrace(bktraceSize * rawDump.bktraceCount, 0);

                // Read memory blocks
                nread = file->Read(&*blocks.begin(), rawDump.blockCount * sizeof(MMBlock));
                if (rawDump.blockCount * sizeof(MMBlock) == nread)
                {
                    // Read symbols
                    nread = file->Read(&*symbols.begin(), rawDump.symbolCount * sizeof(MMSymbol));
                    if (rawDump.symbolCount * sizeof(MMSymbol) == nread)
                    {
                        // Read backtraces
                        nread = file->Read(&*bktrace.begin(), bktraceSize * rawDump.bktraceCount);
                        if (bktraceSize * rawDump.bktraceCount == nread)
                        {
                            for (auto& sym : symbols)
                            {
                                if (sym.name[0] == '\0')
                                {
                                    Snprintf(sym.name, COUNT_OF(sym.name), "%08llX", sym.addr);
                                }
                                symbolTable->AddSymbol(sym.addr, sym.name);
                            }

                            const uint8* curOffset = bktrace.data();
                            for (size_t i = 0, n = rawDump.bktraceCount;i < n;++i)
                            {
                                const MMBacktrace* curBktrace = reinterpret_cast<const MMBacktrace*>(curOffset);
                                const uint64* frames = OffsetPointer<uint64>(curBktrace, sizeof(MMBacktrace));
                                symbolTable->AddBacktrace(curBktrace->hash, frames, rawDump.bktraceDepth);
                                curOffset += bktraceSize;
                            }

                            mblocks.swap(blocks);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void MemorySnapshot::BuildBlockMap()
{
    for (MMBlock& curBlock : mblocks)
    {
        auto iterAt = blockMap.find(curBlock.bktraceHash);
        if (iterAt == blockMap.end())
        {
            iterAt = blockMap.emplace(curBlock.bktraceHash, DAVA::Vector<const DAVA::MMBlock*>()).first;
        }
        iterAt->second.push_back(&curBlock);
    }
}
