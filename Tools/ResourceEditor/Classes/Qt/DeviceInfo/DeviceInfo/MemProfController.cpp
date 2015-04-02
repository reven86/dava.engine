#include <QMessageBox>
#include <QFileDialog>

#include <unordered_map>

#include <Utils/UTF8Utils.h>

#include "Base/FunctionTraits.h"

#include "MemProfWidget.h"
#include "MemProfController.h"
#include "ProfilingSession.h"
#include "BacktraceSymbolTable.h"

using namespace DAVA;
using namespace DAVA::Net;

MemProfController::MemProfController(const DAVA::Net::PeerDescription& peerDescr, QWidget *_parentWidget, QObject* parent)
    : QObject(parent)
    , parentWidget(_parentWidget)
    , peer(peerDescr)
{
    ShowView();
    netClient.SetCallbacks(MakeFunction(this, &MemProfController::OnChannelOpen),
                           MakeFunction(this, &MemProfController::OnChannelClosed),
                           MakeFunction(this, &MemProfController::OnCurrentStat),
                           MakeFunction(this, &MemProfController::OnDump));
}

MemProfController::~MemProfController() {}

void MemProfController::OnDumpPressed()
{
    netClient.RequestDump();
}

void MemProfController::OnViewDump()
{
    //if (!dumpData.empty())
    //{
    //    DumpViewWidget* w = new DumpViewWidget(dumpData, parentWidget, Qt::Window);
    //    w->resize(640, 480);
    //    w->show();
    //}
}

void MemProfController::OnViewFileDump()
{
    /*
#if defined(Q_OS_WIN32)
    QString filename = QFileDialog::getOpenFileName(parentWidget, "Select dump file", "d:\\temp\\dumps\\4");
#else
    QString filename = QFileDialog::getOpenFileName(parentWidget, "Select dump file");
#endif
    if (!filename.isEmpty())
    {
        std::string s = filename.toStdString();
        DumpViewWidget* w = new DumpViewWidget(s.c_str(), parentWidget, Qt::Window);
        w->resize(640, 480);
        w->show();
    }
    */
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

        connect(this, &MemProfController::ConnectionEstablished, view, &MemProfWidget::ConnectionEstablished);
        connect(this, &MemProfController::ConnectionLost, view, &MemProfWidget::ConnectionLost);
        connect(this, &MemProfController::StatArrived, view, &MemProfWidget::StatArrived);

        connect(view, SIGNAL(OnDumpButton()), this, SLOT(OnDumpPressed()));
        connect(view, SIGNAL(OnViewDumpButton()), this, SLOT(OnViewDump()));
        connect(view, SIGNAL(OnViewFileDumpButton()), this, SLOT(OnViewFileDump()));
        connect(this, &QObject::destroyed, view, &QObject::deleteLater);
    }
    view->showNormal();
    view->activateWindow();
    view->raise();
}

void MemProfController::OnChannelOpen(const DAVA::MMStatConfig* config)
{
    bool newConnection = false;
    if (config != nullptr)
    {
        newConnection = true;
        profilingSession = std::make_unique<ProfilingSession>(config, peer);
    }
    emit ConnectionEstablished(newConnection, profilingSession.get());
}

void MemProfController::OnChannelClosed(const char8* message)
{
    emit ConnectionLost(message);
}

void MemProfController::OnCurrentStat(const DAVA::MMCurStat* stat)
{
    profilingSession->AddStatItem(stat);
    emit StatArrived();
}

void MemProfController::OnDump(size_t total, size_t recv, Vector<uint8>* v)
{
    view->UpdateProgress(total, recv);
    if (total == recv)
    {
        DVASSERT(v != nullptr);

        dumpData.swap(*v);
        BacktraceSymbolTable table;

        const MMDump* dump = reinterpret_cast<MMDump*>(dumpData.data());
        const MMCurStat* stat = OffsetPointer<MMCurStat>(dump, sizeof(MMDump));
        OnCurrentStat(stat);

        const size_t bktraceSize = sizeof(MMBacktrace) + dump->bktraceDepth * sizeof(uint64);

        const MMBlock* blocks = OffsetPointer<MMBlock>(stat, stat->size);
        const MMSymbol* symbols = OffsetPointer<MMSymbol>(blocks, sizeof(MMBlock) * dump->blockCount);
        const MMBacktrace* bt = OffsetPointer<MMBacktrace>(symbols, sizeof(MMSymbol) * dump->symbolCount);

        for (size_t i = 0, n = dump->symbolCount;i < n;++i)
        {
            table.AddSymbol(symbols[i].addr, symbols[i].name);
        }
        const MMBacktrace* p = bt;
        for (size_t i = 0, n = dump->bktraceCount;i < n;++i)
        {
            const uint64* frames = OffsetPointer<uint64>(p, sizeof(MMBacktrace));
            table.AddBacktrace(p->hash, frames, dump->bktraceDepth);
            p = OffsetPointer<MMBacktrace>(p, bktraceSize);
        }

        static int dumpIndex = 1;
        {
            char fname[512];
            FilePath fp("~doc:");
            Snprintf(fname, COUNT_OF(fname), "%sdump_%d.bin", fp.GetAbsolutePathname().c_str(), dumpIndex);
            FILE* f = fopen(fname, "wb");
            if (f)
            {
                fwrite(dump, 1, dump->size, f);
                fclose(f);
            }
        }
        {
            char fname[512];
            FilePath fp("~doc:");
            Snprintf(fname, COUNT_OF(fname), "%sdump_%d.log", fp.GetAbsolutePathname().c_str(), dumpIndex);
            dumpIndex += 1;
            FILE* f = fopen(fname, "wb");
            if (f)
            {
                fprintf(f, "General info\n");
                fprintf(f, "  collectTime=%u ms\n", uint32(dump->collectTime * 10));
                fprintf(f, "  packTime=%u ms\n", uint32(dump->packTime * 10));
                //fprintf(f, "  packedSize=%u\n", packedSize);
                fprintf(f, "  blockCount=%u\n", dump->blockCount);
                fprintf(f, "  backtraceCount=%u\n", dump->bktraceCount);
                fprintf(f, "  nameCount=%u\n", dump->symbolCount);
                fprintf(f, "  backtraceDepth=%u\n", dump->bktraceDepth);

                fprintf(f, "Blocks\n");
                for (uint32 i = 0;i < dump->blockCount;++i)
                {
                    fprintf(f, "%4d: allocByApp=%u, allocTotal=%u, orderNo=%u, pool=%u, hash=%u, tags=%08X\n", i + 1,
                            blocks[i].allocByApp, blocks[i].allocTotal, blocks[i].orderNo, blocks[i].pool,
                            blocks[i].bktraceHash, blocks[i].tags);
                    const Vector<const char8*>& fr = table.GetFrames(blocks[i].bktraceHash);
                    for (auto s : fr)
                    {
                        fprintf(f, "        %s\n", s);
                    }
                }
                fclose(f);
            }
        }
    }
}

void MemProfController::Output(const String& msg)
{

}
