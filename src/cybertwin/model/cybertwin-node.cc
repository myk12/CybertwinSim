#include "cybertwin-node.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CybertwinNode");
NS_OBJECT_ENSURE_REGISTERED(CybertwinNode);

TypeId
CybertwinNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CybertwinNode")
            .SetParent<Node>()
            .SetGroupName("Cybertwin")
            .AddConstructor<CybertwinNode>()
            .AddAttribute("UpperNodeAddress",
                          "The address of the upper node",
                          Ipv4AddressValue(),
                          MakeIpv4AddressAccessor(&CybertwinNode::m_upperNodeAddress),
                          MakeIpv4AddressChecker())
            .AddAttribute("SelfNodeAddress",
                          "The address of the current node",
                          Ipv4AddressValue(),
                          MakeIpv4AddressAccessor(&CybertwinNode::m_selfNodeAddress),
                          MakeIpv4AddressChecker())
            .AddAttribute("SelfCuid",
                          "The cybertwin id of the current node",
                          UintegerValue(),
                          MakeUintegerAccessor(&CybertwinNode::m_cybertwinId),
                          MakeUintegerChecker<uint64_t>());
    return tid;
};

TypeId
CybertwinNode::GetInstanceTypeId() const
{
    return GetTypeId();
}

CybertwinNode::CybertwinNode()
{
    NS_LOG_DEBUG("Created a CybertwinNode.");
}

CybertwinNode::~CybertwinNode()
{
    NS_LOG_DEBUG("Destroyed a CybertwinNode.");
}

void
CybertwinNode::PowerOn()
{
    NS_LOG_DEBUG("Setup a CybertwinNode.");
}

void
CybertwinNode::SetAddressList(std::vector<Ipv4Address> addressList)
{
    ipv4AddressList = addressList;
}

void
CybertwinNode::SetName(std::string name)
{
    m_name = name;
}

void
CybertwinNode::SetLogDir(std::string logDir)
{
    m_logDir = logDir;
}

std::string
CybertwinNode::GetLogDir()
{
    return m_logDir;
}

std::string
CybertwinNode::GetName()
{
    return m_name;
}

void
CybertwinNode::AddParent(Ptr<Node> parent)
{
    for (auto it = m_parents.begin(); it != m_parents.end(); ++it)
    {
        if (*it == parent)
        {
            return;
        }
    }
    m_parents.push_back(parent);
}

Ipv4Address
CybertwinNode::GetUpperNodeAddress()
{
    return m_upperNodeAddress;
}

void
CybertwinNode::AddLocalIp(Ipv4Address localIp)
{
    m_localAddrList.push_back(localIp);
}

void
CybertwinNode::AddGlobalIp(Ipv4Address globalIp)
{
    m_globalAddrList.push_back(globalIp);
}

std::vector<Ipv4Address>
CybertwinNode::GetLocalIpList()
{
    return m_localAddrList;
}

std::vector<Ipv4Address>
CybertwinNode::GetGlobalIpList()
{
    return m_globalAddrList;
}

void
CybertwinNode::InstallCNRSApp()
{
    NS_LOG_DEBUG("[Load][InstallCNRSApp] Installing CNRS app to " << m_name << ".");
    Ptr<NameResolutionService> cybertwinCNRSApp = nullptr;
    if (!m_upperNodeAddress.IsInitialized() && m_parents.size() == 0)
    {
        NS_LOG_DEBUG("[Load][InstallCNRSApp] CNRS Root node " << m_name << ".");
        cybertwinCNRSApp = CreateObject<NameResolutionService>();
    }
    else
    {
        if (!m_upperNodeAddress.IsInitialized())
        {
            m_upperNodeAddress = m_parents[0]->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
        }

        // install CNRS application
        NS_LOG_DEBUG("[Load][InstallCNRSApp] CNRS Child node " << m_name << ".");
        cybertwinCNRSApp = CreateObject<NameResolutionService>(m_upperNodeAddress);
    }

    this->AddApplication(cybertwinCNRSApp);
    cybertwinCNRSApp->SetStartTime(Simulator::Now());
    m_cybertwinCNRSApp = cybertwinCNRSApp;
}

void
CybertwinNode::InstallCybertwinManagerApp(std::vector<Ipv4Address> localIpList,
                                          std::vector<Ipv4Address> globalIpList)
{
    NS_LOG_DEBUG("Installing Cybertwin Manager app.");
    if (!m_upperNodeAddress.IsInitialized())
    {
        NS_ASSERT_MSG(m_parents.size() > 0, "No parent node found.");
        Ptr<Node> upperNode = m_parents[0];
        m_upperNodeAddress = upperNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    }

    // install Cybertwin Manager application
    Ptr<CybertwinManager> cybertwinManagerApp =
        CreateObject<CybertwinManager>(localIpList, globalIpList);
    this->AddApplication(cybertwinManagerApp);
    cybertwinManagerApp->SetStartTime(Simulator::Now());
}

void
CybertwinNode::AddConfigFile(std::string filename, nlohmann::json config)
{
    m_configFiles[filename] = config;
}

Ptr<NameResolutionService>
CybertwinNode::GetCNRSApp()
{
    return m_cybertwinCNRSApp;
}

void
CybertwinNode::AddInstalledApp(Ptr<Application> app, Time startTime)
{
    m_installedApps[app] = startTime;
}

void
CybertwinNode::StartAllAggregatedApps()
{
    NS_LOG_INFO("["<<m_name<<"] Starting all installed applications.");
    for (auto it = m_installedApps.begin(); it != m_installedApps.end(); ++it)
    {
        NS_LOG_INFO("["<<m_name<<"] Starting application at "<<it->second);
        //TODO: fix me
        it->first->SetStartTime(it->second);
    }
}

//***************************************************************
//*               edge server node                              *
//***************************************************************

NS_OBJECT_ENSURE_REGISTERED(CybertwinEdgeServer);

TypeId
CybertwinEdgeServer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinEdgeServer")
                            .SetParent<CybertwinNode>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinEdgeServer>();

    return tid;
}

CybertwinEdgeServer::CybertwinEdgeServer()
    : m_cybertwinCNRSApp(nullptr),
      m_CybertwinManagerApp(nullptr)
{
}

CybertwinEdgeServer::~CybertwinEdgeServer()
{
}

void
CybertwinEdgeServer::PowerOn()
{
    NS_LOG_DEBUG("-------------- Powering on " << m_name);
    if (m_parents.size() == 0)
    {
        NS_LOG_ERROR("Edge server should have parents.");
        return;
    }

    InstallCNRSApp();
    // install Cybertwin Controller application
    InstallCybertwinManagerApp(m_localAddrList, m_globalAddrList);
}

Ptr<CybertwinManager>
CybertwinEdgeServer::GetCtrlApp()
{
    return m_CybertwinManagerApp;
}

//***************************************************************
//*               core server node                              *
//***************************************************************
NS_OBJECT_ENSURE_REGISTERED(CybertwinCoreServer);

TypeId
CybertwinCoreServer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinCoreServer")
                            .SetParent<Node>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinCoreServer>();
    return tid;
}

CybertwinCoreServer::CybertwinCoreServer()
    : cybertwinCNRSApp(nullptr)
{
    NS_LOG_DEBUG("Creating a CybertwinCoreServer.");
}

void
CybertwinCoreServer::PowerOn()
{
    NS_LOG_DEBUG("************** Powering on " << m_name);
    InstallCNRSApp(); 
}

CybertwinCoreServer::~CybertwinCoreServer()
{
    NS_LOG_DEBUG("[CybertwinCoreServer] destroy CybertwinCoreServer.");
}

//***************************************************************
//*               end host node                                 *
//***************************************************************
NS_OBJECT_ENSURE_REGISTERED(CybertwinEndHost);

TypeId
CybertwinEndHost::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinEndHost")
                            .SetParent<CybertwinNode>()
                            .SetGroupName("Cybertwin")
                            .AddConstructor<CybertwinEndHost>();
    return tid;
}

CybertwinEndHost::CybertwinEndHost()
{
    NS_LOG_DEBUG("[CybertwinEndHost] create CybertwinEndHost.");
    m_isConnected = false;    
}

CybertwinEndHost::~CybertwinEndHost()
{
    NS_LOG_DEBUG("[CybertwinEndHost] destroy CybertwinEndHost.");
}

Ptr<CybertwinEndHostDaemon>
CybertwinEndHost::GetEndHostDaemon()
{
    return m_cybertwinEndHostDaemon;
}

void
CybertwinEndHost::PowerOn()
{
    NS_LOG_DEBUG("-+-+-+-+-+-+ Powering on " << m_name);

    // Create initd
    Ptr<CybertwinEndHostDaemon> initd = CreateObject<CybertwinEndHostDaemon>();
    initd->SetAttribute("ManagerAddr", Ipv4AddressValue(m_upperNodeAddress));
    initd->SetAttribute("ManagerPort", UintegerValue(CYBERTWIN_MANAGER_PROXY_PORT));
    initd->SetStartTime(Simulator::Now());

    this->AddApplication(initd);

    m_cybertwinEndHostDaemon = initd;
}

void
CybertwinEndHost::SetCybertwinId(CYBERTWINID_t id)
{
    m_cybertwinId = id;
}

CYBERTWINID_t
CybertwinEndHost::GetCybertwinId()
{
    return m_cybertwinId;
}

void
CybertwinEndHost::SetCybertwinPort(uint16_t port)
{
    NS_LOG_INFO("[CybertwinEndHost] SetCybertwinPort: " << port);
    m_cybertwinPort = port;
}

uint16_t
CybertwinEndHost::GetCybertwinPort()
{
    return m_cybertwinPort;
}

void
CybertwinEndHost::SetCybertwinStatus(bool stat)
{
    m_isConnected = stat;
}

bool
CybertwinEndHost::isCybertwinCreated()
{
    return m_isConnected;
}

} // namespace ns3
