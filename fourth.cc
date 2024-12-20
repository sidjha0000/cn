#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h" // Include for NetAnim

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BusTopologyExample");

int main(int argc, char *argv[]) {
// Command line arguments
CommandLine cmd;
cmd.Parse(argc, argv);

// Create nodes (5 nodes for the bus topology)
NodeContainer nodes;
nodes.Create(5); // Create 5 nodes for the bus topology

// Create a CSMA channel (shared medium for bus topology)
CsmaHelper csma;
csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

// Install CSMA devices on nodes
NetDeviceContainer devices;
devices = csma.Install(nodes);

// Install the internet stack on nodes (necessary for IP-based communication)
InternetStackHelper stack;
stack.Install(nodes);

// Assign IP addresses to the devices (ensure each node can communicate)
Ipv4AddressHelper address;
address.SetBase("10.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer interfaces = address.Assign(devices);

// Create a UDP server on node 4 (Node 4 will act as the server)
UdpServerHelper udpServer(9);
ApplicationContainer serverApp = udpServer.Install(nodes.Get(4)); // Node 4 as the server
serverApp.Start(Seconds(1.0));
serverApp.Stop(Seconds(1000.0));

// Create a UDP client on node 0 (sending data to server on node 4)
UdpClientHelper udpClient(interfaces.GetAddress(4), 9); // Node 0 sends data to Node 4
udpClient.SetAttribute("MaxPackets", UintegerValue(320));
udpClient.SetAttribute("Interval", TimeValue(MilliSeconds(50)));
udpClient.SetAttribute("PacketSize", UintegerValue(1024));

ApplicationContainer clientApp = udpClient.Install(nodes.Get(0)); // Installing client app on Node 0
clientApp.Start(Seconds(2.0));
clientApp.Stop(Seconds(1000.0));

// Enable packet capture (optional)
csma.EnablePcapAll("bus-topology");

// Enable logging (optional)
LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
LogComponentEnable("UdpServer", LOG_LEVEL_INFO);

// Setup NetAnim for graphical visualization
AnimationInterface anim("bus___topology.xml");

// Set constant positions for nodes in the animation (to visualize the bus topology)
anim.SetConstantPosition(nodes.Get(0), 100, 100); // Node 0
anim.SetConstantPosition(nodes.Get(1), 200, 100); // Node 1
anim.SetConstantPosition(nodes.Get(2), 300, 100); // Node 2
anim.SetConstantPosition(nodes.Get(3), 400, 100); // Node 3
anim.SetConstantPosition(nodes.Get(4), 500, 100); // Node 4

// Run the simulation
Simulator::Run();
Simulator::Destroy();

return 0;
}
