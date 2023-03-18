/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Morteza Kheirkhah
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Morteza Kheirkhah <m.kheirkhah@sussex.ac.uk>
 */

#ifndef MP_TCP_SUBFLOW_H
#define MP_TCP_SUBFLOW_H

#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/rtt-estimator-old.h"
#include "ns3/sequence-number.h"
#include "ns3/tcp-socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"
#include "ns3/uinteger.h"
#include "mp-tcp-typedefs.h"

#include <list>
#include <map>
#include <queue>
#include <set>
#include <stdint.h>
#include <vector>

// using namespace std;

namespace ns3
{

class MpTcpSubFlow : public Object
{
  public:
    typedef enum
    {
        CLOSED = 0,
        LISTEN,
        SYN_SENT,
        SYN_RCVD,
        ESTABLISHED,
        CLOSE_WAIT,
        LAST_ACK,
        FIN_WAIT_1,
        FIN_WAIT_2,
        CLOSING,
        TIME_WAIT,
        LAST_STATE
    } TcpStates_t;

    static TypeId GetTypeId(void);

    MpTcpSubFlow();
    ~MpTcpSubFlow();

    void AddDSNMapping(uint8_t sFlowIdx,
                       uint64_t dSeqNum,
                       uint16_t dLvlLen,
                       uint32_t sflowSeqNum,
                       uint32_t ack /*, Ptr<Packet> pkt*/);
    void StartTracing(std::string traced);
    void CwndTracer(uint32_t oldval, uint32_t newval);
    void SetFinSequence(const SequenceNumber32& s);
    bool Finished();
    DSNMapping* GetunAckPkt();

    uint16_t routeId;           // Subflow's ID
    bool connected;             // Subflow's connection status
    TcpStates_t state;          // Subflow's connection state
    Ipv4Address sAddr;          // Source Ip address
    uint16_t sPort;             // Source port
    Ipv4Address dAddr;          // Destination address
    uint16_t dPort;             // Destination port
    uint32_t oif;               // interface related to the subflow's sAddr
    EventId retxEvent;          // Retransmission timer
    EventId m_lastAckEvent;     // Timer for last ACK
    EventId m_timewaitEvent;    // Timer for closing connection at sender side
    uint32_t MSS;               // Maximum Segment Size
    uint32_t cnCount;           // Count of remaining connection retries
    uint32_t cnRetries;         // Number of connection retries before giving up
    Time cnTimeout;             // Timeout for connection retry
    TracedValue<uint32_t> cwnd; // Congestion window (in bytes)
    uint32_t ssthresh;          // Slow start threshold
    uint32_t maxSeqNb; // Highest sequence number of a sent byte. Equal to (TxSeqNumber - 1) until a
                       // retransmission occurs
    uint32_t highestAck;           // Highest received ACK for the subflow level sequence number
    uint64_t bandwidth;            // Link's bandwidth
    uint32_t m_initialCWnd;        // Initial cWnd value
    SequenceNumber32 m_recover;    // Previous highest Tx seqNb for fast recovery
    uint32_t m_retxThresh;         // Fast Retransmit threshold
    bool m_inFastRec;              // Currently in fast recovery
    bool m_limitedTx;              // perform limited transmit
    uint32_t m_dupAckCount;        // DupACK counter
    Ipv4EndPoint* m_endPoint;      // L4 stack object
    std::list<DSNMapping*> mapDSN; // List of all sent packets
    std::multiset<double> measuredRTT;
    Ptr<RttMeanDeviation2> rtt; // RTT calculator
    Time lastMeasuredRtt;       // Last measured RTT, used for plotting
    uint32_t TxSeqNumber;       // Subflow's next expected sequence number to send
    uint32_t RxSeqNumber;       // Subflow's next expected sequence number to receive
    uint64_t PktCount;          // number of sent packets
    bool m_gotFin;              // Whether FIN is received
    SequenceNumber32 m_finSeq;  // SeqNb of received FIN
    bool AccumulativeAck;
    uint32_t m_limitedTxCount;
    uint32_t initialSequnceNumber; // Plotting

    // plotting
    std::vector<std::pair<double, uint32_t>> cwndTracer;
    std::vector<std::pair<double, uint32_t>> sstTracer;
    std::vector<std::pair<double, double>> rtoTracer;
    std::vector<std::pair<double, double>> rttTracer;

    std::vector<std::pair<double, double>> ssthreshtrack;
    std::vector<std::pair<double, double>> CWNDtrack;
    std::vector<std::pair<double, uint32_t>> DATA;
    std::vector<std::pair<double, uint32_t>> ACK;
    std::vector<std::pair<double, uint32_t>> DROP;
    std::vector<std::pair<double, uint32_t>> RETRANSMIT;
    std::vector<std::pair<double, uint32_t>> DUPACK;
    std::vector<std::pair<double, double>> _ss;
    std::vector<std::pair<double, double>> _ca;
    std::vector<std::pair<double, double>> _FR_FA;
    std::vector<std::pair<double, double>> _FR_PA;
    std::vector<std::pair<double, double>> _FReTx;
    std::vector<std::pair<double, double>> _TimeOut;
    std::vector<std::pair<double, double>> _RTT;
    std::vector<std::pair<double, double>> _AvgRTT;
    std::vector<std::pair<double, double>> _RTO;
};

} // namespace ns3
#endif /* MP_TCP_SUBFLOW */
