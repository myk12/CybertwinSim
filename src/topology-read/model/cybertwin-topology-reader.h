#ifndef CYBERTWIN_TOPOLOGY_READER_H
#define CYBERTWIN_TOPOLOGY_READER_H

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/topology-read-module.h"

#include "ns3/cybertwin-manager.h"
#include "ns3/cybertwin-node.h"

// using yaml-cpp
#include "yaml-cpp/yaml.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace ns3
{

// define node type
typedef enum NodeType
{
    HOST_SERVER,
    END_CLUSTER,
} NodeType_e;

typedef struct Link
{
    std::string target;
    std::string data_rate;
    std::string delay;
    std::string network;
} Link_t;

typedef struct Gateway
{
    std::string name;
    std::string data_rate;
    std::string delay;
    std::string network;
} Gateway_t;

// define node info
typedef struct NodeInfo
{
    Ptr<Node> node;
    NodeContainer nodes;
    std::string name;
    NodeType_e type;
    int32_t num_nodes;  // number of nodes in the cluster
    std::string network_type;
    std::string local_network; // only for end cluster
    std::vector<Link_t> links;
    std::vector<Gateway_t> gateways;
} NodeInfo_t;

class CybertwinTopologyReader : public TopologyReader
{
  public:
    static TypeId GetTypeId();

    CybertwinTopologyReader();
    ~CybertwinTopologyReader() override;

    CybertwinTopologyReader(const CybertwinTopologyReader &) = delete;
    CybertwinTopologyReader &operator=(const CybertwinTopologyReader &) = delete;

    NodeContainer Read();

  private:
    NodeType_e GetNodeType(const std::string &type);
    Ptr<Node> GetNodeByName(const std::string &name);
    std::vector<Link_t> ParseLinks(const YAML::Node &connections);
    std::vector<Gateway_t> ParseGateways(const YAML::Node &gateways);

    Ipv4InterfaceContainer CreateP2PLink(Ptr<Node> sourceNode, Ptr<Node> targetNode, std::string &data_rate, std::string &delay, std::string &network);
    NodeContainer CreateCsmaNetwork(NodeInfo *csma, Ptr<Node> &leader);
    NodeContainer CreateWifiNetwork(NodeInfo *wifi, Ptr<Node> &leader);
    Ipv4InterfaceContainer AssignIPAddresses(const NetDeviceContainer &devices, const std::string &network);

    void ParseCoreCloud(const YAML::Node &coreLayer);
    void ParseEdgeCloud(const YAML::Node &edgeLayer);
    void ParseAccessNetwork(const YAML::Node &accessLayer);

    std::string MaskNumberToIpv4Address(std::string mask);

    std::vector<NodeInfo_t *> m_coreNodesList;
    std::vector<NodeInfo_t *> m_edgeNodesList;
    std::vector<NodeInfo_t *> m_endNodesList;
  
    NodeContainer m_nodes;
    NetDeviceContainer m_devices;
    NodeContainer m_coreNodes;
    NetDeviceContainer m_coreDevices;
    NodeContainer m_edgeNodes;
    NetDeviceContainer m_edgeDevices;
    NodeContainer m_endNodes;
    NetDeviceContainer m_endDevices;
    std::unordered_map<std::string, NodeInfo_t *> m_nodeInfoMap;
    std::unordered_set<std::string> m_links;
};

}; // namespace ns3



#endif /* CYBERTWIN_TOPOLOGY_READER_H */