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


#ifndef __DEVICELISTCONTROLLER_H__
#define __DEVICELISTCONTROLLER_H__

#include <QObject>
#include <QPointer>
#include <vector>

#include <Network/NetCore.h>
#include <Network/Base/Endpoint.h>

#include <Network/PeerDesription.h>

class QStandardItemModel;
class QStandardItem;

class DeviceListWidget;
class DeviceLogController;
class MemProfController;

// Struct that holds network services for remote device
// For now only one service - log receiver
struct DeviceServices
{
    DeviceServices() : log(nullptr), memprof(nullptr) {}

    DeviceLogController* log;
    MemProfController* memprof;
};

// Register types for use with QVariant
Q_DECLARE_METATYPE(DAVA::Net::Endpoint);
Q_DECLARE_METATYPE(DAVA::Net::PeerDescription);
Q_DECLARE_METATYPE(DeviceServices);

class DeviceListController : public QObject
{
    Q_OBJECT

    enum
    {
        SERVICE_LOG = 0,
        SERVICE_MEMPROF = 1
    };

    enum DeviceDataRole
    {
        // Roles for each item in QStandardItemModel
        ROLE_CONNECTION_ID = Qt::UserRole + 1,  // Store NetCore::TrackId to track whether device is connected or no
        ROLE_SOURCE_ADDRESS,                    // Store endpoint announce has arrived from
        ROLE_PEER_DESCRIPTION,                  // Store device description recieved from announce
        ROLE_PEER_SERVICES                      // Store network services to communicate with remote device
    };

    static const DAVA::uint16 ANNOUNCE_PORT = 9999;
    static const DAVA::char8 announceMulticastGroup[];

public:
    explicit DeviceListController(QObject *parent = NULL);
    ~DeviceListController();

    void SetView(DeviceListWidget *view);
    void ShowView();

    // Method invoked when announce packet arrived
    void DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint);
    // Method invoked when all network controllers were stopped
    void AllStopped();

private slots:
    void OnConnectButtonPressed();
    void OnDisconnectButtonPressed();
    void OnShowLogButtonPressed();

private:
    void ConnectDeviceInternal(QModelIndex& index, size_t ifIndex);
    void DisonnectDeviceInternal(QModelIndex& index);

    // Methods to create and delete network services
    DAVA::Net::IChannelListener* CreateLogger(DAVA::uint32 serviceId, void* context);
    void DeleteLogger(DAVA::Net::IChannelListener*, void* context);
    
    DAVA::Net::IChannelListener* CreateMemProfiler(DAVA::uint32 serviceId, void* context);
    void DeleteMemProfiler(DAVA::Net::IChannelListener* obj, void* context);

    // Check whether device already has been discovered
    bool AlreadyInModel(const DAVA::Net::Endpoint& endp) const;

private:
    QPointer<QStandardItemModel> model;
    QPointer<DeviceListWidget> view;

private:
    static QStandardItem *CreateDeviceItem(const DAVA::Net::Endpoint& endp, const DAVA::Net::PeerDescription& peerDescr);
};



#endif // __DEVICELISTCONTROLLER_H__
