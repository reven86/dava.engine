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


#ifndef QUAZIP_QUAZIODEVICE_H
#define QUAZIP_QUAZIODEVICE_H

#include <QIODevice>
#include "quazip_global.h"

#include <zlib.h>

class QuaZIODevicePrivate;

/// A class to compress/decompress QIODevice.
/**
  This class can be used to compress any data written to QIODevice or
  decompress it back. Compressing data sent over a QTcpSocket is a good
  example.
  */
class QUAZIP_EXPORT QuaZIODevice: public QIODevice {
  Q_OBJECT
public:
  /// Constructor.
  /**
    \param io The QIODevice to read/write.
    \param parent The parent object, as per QObject logic.
    */
  QuaZIODevice(QIODevice *io, QObject *parent = NULL);
  /// Destructor.
  ~QuaZIODevice();
  /// Flushes data waiting to be written.
  /**
    Unfortunately, as QIODevice doesn't support flush() by itself, the
    only thing this method does is write the compressed data into the
    device using Z_SYNC_FLUSH mode. If you need the compressed data to
    actually be flushed from the buffer of the underlying QIODevice, you
    need to call its flush() method as well, providing it supports it
    (like QTcpSocket does). Example:
    \code
    QuaZIODevice dev(&sock);
    dev.open(QIODevice::Write);
    dev.write(yourDataGoesHere);
    dev.flush();
    sock->flush(); // this actually sends data to network
    \endcode

    This may change in the future versions of QuaZIP by implementing an
    ugly hack: trying to cast the QIODevice using qobject_cast to known
    flush()-supporting subclasses, and calling flush if the resulting
    pointer is not zero.
    */
  virtual bool flush();
  /// Opens the device.
  /**
    \param mode Neither QIODevice::ReadWrite nor QIODevice::Append are
    not supported.
    */
  virtual bool open(QIODevice::OpenMode mode);
  /// Closes this device, but not the underlying one.
  /**
    The underlying QIODevice is not closed in case you want to write
    something else to it.
    */
  virtual void close();
  /// Returns the underlying device.
  QIODevice *getIoDevice() const;
  /// Returns true.
  virtual bool isSequential() const;
protected:
  /// Implementation of QIODevice::readData().
  virtual qint64 readData(char *data, qint64 maxSize);
  /// Implementation of QIODevice::writeData().
  virtual qint64 writeData(const char *data, qint64 maxSize);
private:
  QuaZIODevicePrivate *d;
};
#endif // QUAZIP_QUAZIODEVICE_H
