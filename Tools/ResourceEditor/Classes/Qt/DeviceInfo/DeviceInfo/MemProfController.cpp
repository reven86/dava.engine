#include <QMessageBox>
#include <QFileDialog>

#include <unordered_map>

#include <Utils/UTF8Utils.h>

#include "Base/FunctionTraits.h"

#include "MemProfWidget.h"
#include "DumpViewWidget.h"
#include "MemProfController.h"
#include "BacktraceSymbolTable.h"

using namespace DAVA;
using namespace DAVA::Net;

MemProfController::MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *_parentWidget, QObject* parent)
    : QObject(parent)
    , parentWidget(_parentWidget)
    , peer(peerDescr)
{
    ShowView();
    netClient.SetCallbacks(MakeFunction(this, &MemProfController::ChannelOpen),
                           MakeFunction(this, &MemProfController::ChannelClosed),
                           MakeFunction(this, &MemProfController::CurrentStat),
                           MakeFunction(this, &MemProfController::Dump),
                           MakeFunction(this, &MemProfController::DumpDone));
}

MemProfController::~MemProfController() {}

void MemProfController::OnDumpPressed()
{
    netClient.RequestDump();
}

void MemProfController::OnViewDump()
{
    if (!dumpData.empty())
    {
        DumpViewWidget* w = new DumpViewWidget(dumpData, parentWidget, Qt::Window);
        w->resize(640, 480);
        w->show();
    }
}

void MemProfController::OnViewFileDump()
{
    QString filename = QFileDialog::getOpenFileName(parentWidget, "Select dump file", "d:\\temp\\dumps\\4");
    if (!filename.isEmpty())
    {
        std::string s = filename.toStdString();
        DumpViewWidget* w = new DumpViewWidget(s.c_str(), parentWidget, Qt::Window);
        w->resize(640, 480);
        w->show();
    }
}

void MemProfController::ShowView()
{
    if (NULL == view)
    {
        const QString title = QString("%1 (%2 %3)")
            .arg(peer.GetName().c_str())
            .arg(peer.GetPlatformString().c_str())
            .arg(peer.GetVersion().c_str());

        view = new MemProfWidget(parentWidget);
        view->setWindowFlags(Qt::Window);
        view->setWindowTitle(title);

        connect(view, SIGNAL(OnDumpButton()), this, SLOT(OnDumpPressed()));
        connect(view, SIGNAL(OnViewDumpButton()), this, SLOT(OnViewDump()));
        connect(view, SIGNAL(OnViewFileDumpButton()), this, SLOT(OnViewFileDump()));
        connect(this, &QObject::destroyed, view, &QObject::deleteLater);
    }
    view->showNormal();
    view->activateWindow();
    view->raise();
}

void MemProfController::ChannelOpen(const DAVA::MMStatConfig* config)
{
    view->ChangeStatus("connected", nullptr);
    view->ClearStat();

    Logger::Debug("MemProfController::ChannelOpen");
    if (config)
    {
        Logger::Debug("   maxTags=%u, ntags=%u", config->maxTagCount, config->tagCount);
        for (uint32 i = 0;i < config->tagCount;++i)
            Logger::Debug("      %d, %s", i, config->names[i].name);
        Logger::Debug("   maxPools=%u, npools=%u", config->maxAllocPoolCount, config->allocPoolCount);
        for (uint32 i = 0;i < config->allocPoolCount;++i)
            Logger::Debug("      %d, %s", i, config->names[i + config->tagCount].name);
    }
    view->SetStatConfig(config);
    
}

void MemProfController::ChannelClosed(const char8* message)
{
    view->ChangeStatus("disconnected", message);
}

void MemProfController::CurrentStat(const DAVA::MMStat* stat)
{
    view->UpdateStat(stat);
}

void MemProfController::Dump(size_t total, size_t recv)
{
    view->UpdateProgress(total, recv);
}

template<typename T>
inline T* Offset(void* ptr, size_t byteOffset)
{
    return reinterpret_cast<T*>(static_cast<uint8*>(ptr)+byteOffset);
}

template<typename T>
inline const T* Offset(const void* ptr, size_t byteOffset)
{
    return reinterpret_cast<const T*>(static_cast<const uint8*>(ptr)+byteOffset);
}

uint32 BacktraceHash(const MMBacktrace& backtrace)
{
    uint32 hash = HashValue_N(reinterpret_cast<const char*>(backtrace.frames), sizeof(backtrace.frames));
    return hash;
}

void MemProfController::DumpDone(const DAVA::MMDump* dump, size_t packedSize, Vector<uint8>& dumpV)
{
    BacktraceSymbolTable table;

    static int dumpIndex = 1;

    view->UpdateProgress(100, 100);

    dumpData.swap(dumpV);

    const size_t bktraceSize = dump->backtraceDepth * sizeof(uint64) + sizeof(uint32) * 4;

    const MMBlock* blocks = Offset<MMBlock>(dump, sizeof(MMDump));
    const MMBacktrace* bt = Offset<MMBacktrace>(blocks, sizeof(MMBlock) * dump->blockCount);
    const MMSymbol* symbols = Offset<MMSymbol>(bt, dump->backtraceCount * bktraceSize);

    for (size_t i = 0, n = dump->symbolCount;i < n;++i)
    {
        table.AddSymbol(symbols[i].addr, symbols[i].name);
    }

    const MMBacktrace* p = bt;
    for (size_t i = 0, n = dump->backtraceCount;i < n;++i)
    {
        table.AddBacktrace(p->hash, p->frames, dump->backtraceDepth);
        p = Offset<MMBacktrace>(p, bktraceSize);
    }

    char fname[512];
    FilePath fp("~doc:");
    {
        
        Snprintf(fname, COUNT_OF(fname), "%sdump_%d.bin", fp.GetAbsolutePathname().c_str(), dumpIndex);
        FILE* f = fopen(fname, "wb");
        if (f)
        {
            size_t dumpSize = sizeof(MMDump) 
                + sizeof(MMBlock) * dump->blockCount 
                + bktraceSize * dump->backtraceCount
                + sizeof(MMSymbol) * dump->symbolCount;
            fwrite(dump, 1, dumpSize, f);
            fclose(f);
        }
    }
    Snprintf(fname, COUNT_OF(fname), "%sdump_%d.log", fp.GetAbsolutePathname().c_str(), dumpIndex);
    dumpIndex += 1;
    FILE* f = fopen(fname, "wb");
    if (f)
    {
        fprintf(f, "General info\n");
        fprintf(f, "  collectTime=%u ms\n", dump->collectTime);
        fprintf(f, "  zipTime=%u ms\n", dump->zipTime);
        fprintf(f, "  packedSize=%u\n", packedSize);
        fprintf(f, "  blockCount=%u\n", dump->blockCount);
        fprintf(f, "  backtraceCount=%u\n", dump->backtraceCount);
        fprintf(f, "  nameCount=%u\n", dump->symbolCount);
        fprintf(f, "  blockBegin=%u\n", dump->blockBegin);
        fprintf(f, "  blockEnd=%u\n", dump->blockEnd);
        fprintf(f, "  backtraceDepth=%u\n", dump->backtraceDepth);

        fprintf(f, "Blocks\n");
        for (uint32 i = 0;i < dump->blockCount;++i)
        {
            fprintf(f, "%4d: allocByApp=%u, allocTotal=%u, orderNo=%u, pool=%u, hash=%u\n", i + 1,
                    blocks[i].allocByApp, blocks[i].allocTotal, blocks[i].orderNo, blocks[i].pool,
                    blocks[i].backtraceHash);
            //fprintf(f, "%4d: addr=%08llX, allocByApp=%u, allocTotal=%u, orderNo=%u, pool=%u, hash=%u\n", i + 1,
            //        blocks[i].addr,
            //        blocks[i].allocByApp, blocks[i].allocTotal, blocks[i].orderNo, blocks[i].pool,
            //        blocks[i].backtraceHash);
            const Vector<const char8*>& fr = table.GetFrames(blocks[i].backtraceHash);
            for (auto s : fr)
            {
                fprintf(f, "        %s\n", s);
            }
            /*auto x = traceMap.find(blocks[i].backtraceHash);
            if (x != traceMap.end())
            {
                const MMBacktrace& z = (*x).second;
                for (size_t j = 0;j < dump->backtraceDepth;++j)
                {
                    auto u = symbolMap.find(z.frames[j]);
                    if (u != symbolMap.end())
                    {
                        fprintf(f, "        %08llX    %s\n", z.frames[j], (*u).second.c_str());
                    }
                    else
                    {
                        fprintf(f, "        %08llX\n", z.frames[j]);
                    }
                }
            }*/
        }

        /*fprintf(f, "Symbols\n");
        int isym = 0;
        for (auto& x : symbolMap)
        {
            fprintf(f, "  %4d: %08llX; %s\n", isym + 1, x.first, x.second.c_str());
            isym += 1;
        }*/

        /*fprintf(f, "Backtraces\n");
        for (uint32 h = 0;h < dump->backtraceCount;++h)
        {
            uint32 hash = bt[h].hash;
            fprintf(f, "hash = %u (%08X)\n", hash, hash);
            for (auto ff : bt[h].frames)
            {
                fprintf(f, "  %08llX\n", ff);
            }
        }*/
        fclose(f);
    }
}

void MemProfController::Output(const String& msg)
{
    view->AppendText(msg.c_str());
}
