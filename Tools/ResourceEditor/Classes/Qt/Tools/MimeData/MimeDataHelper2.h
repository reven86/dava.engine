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


#ifndef __MIMEDATA_HELPER2_H__
#define __MIMEDATA_HELPER2_H__

#include "DAVAEngine.h"
#include <QMimeData>
#include <QDataStream>

template <class T>
class MimeDataHelper2
{
public:
    static QMimeData * EncodeMimeData(const QVector<T *> &data);
    static QVector<T *> DecodeMimeData(const QMimeData * data);

    static bool IsValid(const QMimeData * mimeData);
	static inline const QString GetMimeType();
};

template <class T>
QMimeData * MimeDataHelper2<T>::EncodeMimeData(const QVector<T *> & data)
{
	if(data.size() > 0)
	{
		QByteArray encodedData;
		QDataStream stream(&encodedData, QIODevice::WriteOnly);
        
        QMimeData *mimeData = new QMimeData();
		for (int i = 0; i < data.size(); ++i)
		{
			stream.writeRawData((char *) &data[i], sizeof(T *));
		}
        
		mimeData->setData(MimeDataHelper2<T>::GetMimeType(), encodedData);
        return mimeData;
	}
    
    return NULL;
}

template <class T>
QVector<T *> MimeDataHelper2<T>::DecodeMimeData(const QMimeData * data)
{
	QVector<T *> ret;

    QString format = MimeDataHelper2<T>::GetMimeType();
	if(data->hasFormat(format))
	{
		QByteArray encodedData = data->data(format);
		QDataStream stream(&encodedData, QIODevice::ReadOnly);
        
		while(!stream.atEnd())
		{
            T * object = NULL;
			stream.readRawData((char *) &object, sizeof(T *));
			ret.push_back(object);
		}
	}
    
	return ret;
}

template<class T>
bool MimeDataHelper2<T>::IsValid(const QMimeData * mimeData)
{
    return ((mimeData != NULL) && mimeData->hasFormat(MimeDataHelper2<T>::GetMimeType()));
}

template<> inline const QString MimeDataHelper2<DAVA::Entity>::GetMimeType()		{ return "application/dava.entity"; }
template<> inline const QString MimeDataHelper2<DAVA::NMaterial>::GetMimeType()		{ return "application/dava.nmaterial"; }
template<> inline const QString MimeDataHelper2<DAVA::ParticleEmitter>::GetMimeType()	{ return "application/dava.particleemitter"; }
template<> inline const QString MimeDataHelper2<DAVA::ParticleLayer>::GetMimeType()	{ return "application/dava.particlelayer"; }
template<> inline const QString MimeDataHelper2<DAVA::ParticleForce>::GetMimeType()	{ return "application/dava.particleforce"; }

template<class T> inline const QString MimeDataHelper2<T>::GetMimeType() { return QString(); }


#endif  //#ifndef __MIMEDATA_HELPER2_H__