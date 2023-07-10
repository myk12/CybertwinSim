#ifndef CYBERTWIN_H
#define CYBERTWIN_H

#include "../cybertwin-common.h"
#include "../cybertwin-header.h"
#include "../devices/cybertwin-node.h"
#include "../networks/multipath-data-transfer-protocol.h"
#include "../networks/cybertwin-name-resolution-service.h"
#include "../cybertwin-tag.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/callback.h"

#include <queue>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

namespace ns3
{
class CybertwinEdgeServer;
class MultipathConnection;
class CybertwinDataTransferServer;
class Cybertwin : public Application
{
  public:
    typedef Callback<void, CybertwinHeader> CybertwinInitCallback;
#if MDTP_ENABLED
    typedef Callback<int, CYBERTWINID_t, MultipathConnection*, Ptr<const Packet>>
        CybertwinSendCallback;
#else
    typedef Callback<int, CYBERTWINID_t, Ptr<Socket>, Ptr<const Packet>>
        CybertwinSendCallback;
#endif
    typedef Callback<int, Ptr<Socket>, Ptr<const Packet>> CybertwinReceiveCallback;

    Cybertwin();
    Cybertwin(CYBERTWINID_t,
              CYBERTWIN_INTERFACE_LIST_t g_interfaces,
              const Address&,
              CybertwinInitCallback,
              CybertwinSendCallback,
              CybertwinReceiveCallback);
    Cybertwin(CYBERTWINID_t,
              uint16_t,
              CYBERTWIN_INTERFACE_LIST_t g_interfaces);

    ~Cybertwin();

    static TypeId GetTypeId();
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    void Initialize();

    void RecvFromSocket(Ptr<Socket>);
    void RecvLocalPacket(const CybertwinHeader&, Ptr<Packet>);
    void RecvGlobalPacket(const CybertwinHeader&, Ptr<Packet>);

    void ForwardLocalPacket(CYBERTWINID_t, CYBERTWIN_INTERFACE_LIST_t&);

    bool LocalConnRequestCallback(Ptr<Socket>, const Address&);
    void LocalConnCreatedCallback(Ptr<Socket>, const Address&);
    void LocalNormalCloseCallback(Ptr<Socket>);
    void LocalErrorCloseCallback(Ptr<Socket>);

    void UpdateRxSizePerSecond(
#if MDTP_ENABLED
        CYBERTWINID_t conn
#else
        Ptr<Socket> sock
#endif
    );

    void CybertwinServerBulkSend(
#if MDTP_ENABLED
        MultipathConnection* conn
#else
        Ptr<Socket> sock
#endif
    );

#if MDTP_ENABLED
    void NewMpConnectionCreatedCallback(MultipathConnection* conn);
    void NewMpConnectionErrorCallback(MultipathConnection* conn);
    void MpConnectionRecvCallback(MultipathConnection* conn);
    void MpConnectionClosedCallback(MultipathConnection* conn);

    void DownloadFileServer(MultipathConnection* conn);
#else
    bool NewSpConnectionRequestCallback(Ptr<Socket> sock);
    void NewSpConnectionCreatedCallback(Ptr<Socket> sock);
    void NewSpConnectionErrorCallback(Ptr<Socket> sock);
    void SpConnectionRecvCallback(Ptr<Socket> sock);
    void SpNormalCloseCallback(Ptr<Socket> sock);
    void SpErrorCloseCallback(Ptr<Socket> sock);
#endif

    void SendPendingPackets(CYBERTWINID_t);
    void SocketConnectWithResolvedCybertwinName(Ptr<Socket> sock,
                                                CYBERTWINID_t cyberid,
                                                CYBERTWIN_INTERFACE_LIST_t ifs);

    CybertwinInitCallback InitCybertwin;
    CybertwinSendCallback SendPacket;
    CybertwinReceiveCallback ReceivePacket;

    // buffer for handling packet fragments; also used for recording all accepted sockets
    std::unordered_map<Ptr<Socket>, Ptr<Packet>> m_streamBuffer;
    std::unordered_map<CYBERTWINID_t, Ptr<Socket>> m_txBuffer;

    // data transmission between cybertwins
    std::unordered_map<CYBERTWINID_t, std::queue<Ptr<Packet>>> m_txPendingBuffer;
#if MDTP_ENABLED // Multipath Connection
    std::unordered_map<CYBERTWINID_t, MultipathConnection*> m_txConnections;
    std::unordered_map<CYBERTWINID_t, MultipathConnection*> m_pendingConnections;
    std::unordered_map<CYBERTWINID_t, MultipathConnection*> m_rxConnections;
    std::unordered_map<CYBERTWINID_t, std::queue<Ptr<Packet>>> m_rxPendingBuffer;
    std::unordered_map<CYBERTWINID_t, TracedValue<uint64_t>> m_rxSizePerSecond;
#else  // Naive Socket
    std::unordered_map<CYBERTWINID_t, Ptr<Socket>> m_txConnections;
    std::unordered_map<Ptr<Socket>, CYBERTWINID_t> m_txConnectionsReverse;
    std::unordered_map<CYBERTWINID_t, Ptr<Socket>> m_pendingConnections;
    std::unordered_map<Ptr<Socket>, CYBERTWINID_t> m_pendingConnectionsReverse;
    std::unordered_set<Ptr<Socket>> m_rxConnections;
    std::unordered_map<Ptr<Socket>, Address> m_rxConnectionsReverse;
    std::unordered_map<Ptr<Socket>, std::queue<Ptr<Packet>>> m_rxPendingBuffer;
    std::unordered_map<Ptr<Socket>, TracedValue<uint64_t>> m_rxSizePerSecond;
#endif

private:
    // private member data    
    CYBERTWINID_t m_cybertwinId;
    Address m_address;

    // end related
    Ptr<Socket> m_localSocket;
    uint16_t m_localPort;

    // cloud related
    CYBERTWIN_INTERFACE_LIST_t m_interfaces;

    Ptr<NameResolutionService> m_cnrs;
    std::unordered_map<CYBERTWINID_t, CYBERTWIN_INTERFACE_LIST_t> nameResolutionCache;

    // Cybertwin multiple interfaces

#if MDTP_ENABLED
    // Cybertwin Connections
    CybertwinDataTransferServer* m_dtServer;
#else
    Ptr<Socket> m_dtServer;
#endif

    uint64_t m_serverTxBytes; //number of sent times

    std::ofstream m_MpLogFile;
    std::string m_MpLogFileName;
};

}; // namespace ns3

#endif