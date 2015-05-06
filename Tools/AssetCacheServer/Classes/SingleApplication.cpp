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


#include <QTimer>
#include <QByteArray>
#include <QFile>
#include <QWidget>

#include "SingleApplication.h"

SingleApplication::SingleApplication(int argc, char *argv[])
    : QApplication(argc, argv)
{
    QString uniqueKey(qApp->applicationName());
    sharedMemory.setKey(uniqueKey);

    // when  can create it only if it doesn't exist
    if (sharedMemory.create(maxBufferSize()))
    {
        sharedMemory.lock();
        static_cast<char*>(sharedMemory.data())[0] = '\0';
        sharedMemory.unlock();

        alreadyExists = false;

        // start checking for messages of other instances.
        QTimer *pTimer = new QTimer(this);
        connect(pTimer, SIGNAL(timeout()), this, SLOT(checkForMessage()));
        pTimer->start(200);
    }
    // it exits, so we can attach it?!
    else if (sharedMemory.attach())
    {
        sendMessage("--open");
        exit(EXIT_SUCCESS);
    }
    else
    {
        // error
    }
}

int SingleApplication::maxBufferSize()
{
    return 0xffff; //it must be maximum file name length, but who cares;
}

void SingleApplication::checkForMessage()
{
    QStringList arguments;

    sharedMemory.lock();
    char *from = static_cast<char*>(sharedMemory.data());

    while (*from != '\0')
    {
        unsigned char sizeToRead = *from;
        ++from;

        QByteArray byteArray = QByteArray(from, sizeToRead);
        byteArray.append('\0');
        from += sizeToRead;

        arguments << QString::fromUtf8(byteArray.constData());
    }

    static_cast<char*>(sharedMemory.data())[0] = '\0';
    sharedMemory.unlock();

    for (QString arg : arguments)
    {
        if (arg.compare("--opened") == 0)
        {
            QApplication::activeWindow()->raise();
            QApplication::activeWindow()->show();
        }
    }
}

bool SingleApplication::sendMessage(const QString &message)
{
    //we cannot send mess if we are master process!
    if (!alreadyExists)
    {
        return false;
    }

    QByteArray byteArray;
    byteArray.append(message.toUtf8());
    byteArray.prepend(byteArray.size());
    byteArray.append('\0');
    if (byteArray.size() > maxBufferSize())
    {
        int size = byteArray.size();
        byteArray.clear();
        byteArray.append(QString("data more than buffer size. Data size: $%1").arg(size));
    }

    sharedMemory.lock();
    char *to = static_cast<char*>(sharedMemory.data());
    while (*to != '\0')
    {
        int sizeToRead = int(*to);
        to += sizeToRead + 1;
    }

    const char *from = byteArray.data();
    memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
    sharedMemory.unlock();

    return true;
}
