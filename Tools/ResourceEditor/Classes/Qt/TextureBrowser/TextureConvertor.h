#ifndef __TEXTURE_CONVERTOR_H__
#define __TEXTURE_CONVERTOR_H__

#include "Base/BaseTypes.h"

#include <QObject>
#include <QFutureWatcher>
#include <QProgressDialog>

#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"

#include "TextureInfo.h"
#include "TextureConvertorWork.h"
#include "TextureConvertMode.h"

#include "Tools/QtWaitDialog/QtWaitDialog.h"

#define CONVERT_JOB_COUNT 2

class TextureConvertor : public QObject, public DAVA::Singleton<TextureConvertor>
{
    Q_OBJECT

public:
    TextureConvertor();
    ~TextureConvertor();

    static DAVA::Vector<DAVA::Image*> ConvertFormat(DAVA::TextureDescriptor* descriptor, DAVA::eGPUFamily gpu,
                                                    eTextureConvertMode convertMode);

    int GetThumbnail(const DAVA::TextureDescriptor* descriptor);
    int GetOriginal(const DAVA::TextureDescriptor* descriptor);
    int GetConverted(const DAVA::TextureDescriptor* descriptor, DAVA::eGPUFamily gpu,
                     eTextureConvertMode convertMode = CONVERT_NOT_EXISTENT);
    int Reconvert(DAVA::Scene* scene, eTextureConvertMode convertMode);

    void WaitConvertedAll(QWidget* parent = nullptr);
    void CancelConvert();

signals:
    void ReadyThumbnail(const DAVA::TextureDescriptor* descriptor, const TextureInfo& image);
    void ReadyOriginal(const DAVA::TextureDescriptor* descriptor, const TextureInfo& image);
    void ReadyConverted(const DAVA::TextureDescriptor* descriptor, const DAVA::eGPUFamily gpu, const TextureInfo& image);
    void ReadyReconvert();

    void ReadyConvertedAll();

    void ConvertStatusImg(const QString& imgPath, int imgGpu);
    void ConvertStatusQueue(int curJob, int jobCount);

private:
    int jobIdCounter;
    int convertJobQueueSize;

    bool waitingComletion;
    QString waitStatusText;

    QFutureWatcher<TextureInfo> thumbnailWatcher;
    QFutureWatcher<TextureInfo> originalWatcher;
    QFutureWatcher<TextureInfo> convertedWatcher;

    JobStack jobStackThumbnail;
    JobStack jobStackOriginal;
    JobStack jobStackConverted;

    JobItem* curJobThumbnail;
    JobItem* curJobOriginal;
    JobItem* curJobConverted;

    QtWaitDialog* waitDialog;

    void jobRunNextConvert();
    void jobRunNextOriginal();
    void jobRunNextThumbnail();

    TextureInfo GetThumbnailThread(JobItem* item);
    TextureInfo GetOriginalThread(JobItem* item);
    TextureInfo GetConvertedThread(JobItem* item);

private slots:

    void waitCanceled();
    void threadThumbnailFinished();
    void threadOriginalFinished();
    void threadConvertedFinished();
};

#endif // __TEXTURE_CONVERTOR_H__
