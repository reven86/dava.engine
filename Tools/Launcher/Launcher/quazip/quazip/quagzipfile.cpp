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


#include <QFile>

#include "quagzipfile.h"

/// \cond internal
class QuaGzipFilePrivate {
    friend class QuaGzipFile;
    QString fileName;
    gzFile gzd;
    inline QuaGzipFilePrivate(): gzd(NULL) {}
    inline QuaGzipFilePrivate(const QString &fileName): 
        fileName(fileName), gzd(NULL) {}
    template<typename FileId> bool open(FileId id, 
        QIODevice::OpenMode mode, QString &error);
    gzFile open(int fd, const char *modeString);
    gzFile open(const QString &name, const char *modeString);
};

gzFile QuaGzipFilePrivate::open(const QString &name, const char *modeString)
{
    return gzopen(QFile::encodeName(name).constData(), modeString);
}

gzFile QuaGzipFilePrivate::open(int fd, const char *modeString)
{
    return gzdopen(fd, modeString);
}

template<typename FileId>
bool QuaGzipFilePrivate::open(FileId id, QIODevice::OpenMode mode, 
                              QString &error)
{
    char modeString[2];
    modeString[0] = modeString[1] = '\0';
    if ((mode & QIODevice::Append) != 0) {
        error = QuaGzipFile::trUtf8("QIODevice::Append is not "
                "supported for GZIP");
        return false;
    }
    if ((mode & QIODevice::ReadOnly) != 0
            && (mode & QIODevice::WriteOnly) != 0) {
        error = QuaGzipFile::trUtf8("Opening gzip for both reading"
            " and writing is not supported");
        return false;
    } else if ((mode & QIODevice::ReadOnly) != 0) {
        modeString[0] = 'r';
    } else if ((mode & QIODevice::WriteOnly) != 0) {
        modeString[0] = 'w';
    } else {
        error = QuaGzipFile::trUtf8("You can open a gzip either for reading"
            " or for writing. Which is it?");
        return false;
    }
    gzd = open(id, modeString);
    if (gzd == NULL) {
        error = QuaGzipFile::trUtf8("Could not gzopen() file");
        return false;
    }
    return true;
}
/// \endcond

QuaGzipFile::QuaGzipFile():
d(new QuaGzipFilePrivate())
{
}

QuaGzipFile::QuaGzipFile(QObject *parent):
QIODevice(parent),
d(new QuaGzipFilePrivate())
{
}

QuaGzipFile::QuaGzipFile(const QString &fileName, QObject *parent):
  QIODevice(parent),
d(new QuaGzipFilePrivate(fileName))
{
}

QuaGzipFile::~QuaGzipFile()
{
  if (isOpen()) {
    close();
  }
  delete d;
}

void QuaGzipFile::setFileName(const QString& fileName)
{
    d->fileName = fileName;
}

QString QuaGzipFile::getFileName() const
{
    return d->fileName;
}

bool QuaGzipFile::isSequential() const
{
  return true;
}

bool QuaGzipFile::open(QIODevice::OpenMode mode)
{
    QString error;
    if (!d->open(d->fileName, mode, error)) {
        setErrorString(error);
        return false;
    }
    return QIODevice::open(mode);
}

bool QuaGzipFile::open(int fd, QIODevice::OpenMode mode)
{
    QString error;
    if (!d->open(fd, mode, error)) {
        setErrorString(error);
        return false;
    }
    return QIODevice::open(mode);
}

bool QuaGzipFile::flush()
{
    return gzflush(d->gzd, Z_SYNC_FLUSH) == Z_OK;
}

void QuaGzipFile::close()
{
  QIODevice::close();
  gzclose(d->gzd);
}

qint64 QuaGzipFile::readData(char *data, qint64 maxSize)
{
    return gzread(d->gzd, (voidp)data, (unsigned)maxSize);
}

qint64 QuaGzipFile::writeData(const char *data, qint64 maxSize)
{
    if (maxSize == 0)
        return 0;
    int written = gzwrite(d->gzd, (voidp)data, (unsigned)maxSize);
    if (written == 0)
        return -1;
    else
        return written;
}
