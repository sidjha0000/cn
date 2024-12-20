#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create 4 nodes
    NodeContainer nodes;
    nodes.Create(4);

    // Configure CSMA channel with 100 Mbps and 6.5 microseconds delay
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6500)));

    // Install CSMA devices on nodes
    NetDeviceContainer devices;
    devices = csma.Install(nodes);

    // Install the internet stack on all nodes
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses to the CSMA devices
    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Set up UDP echo server on node 0
    UdpEchoServerHelper echoServer(9); // Port 9
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // Set up UDP echo client on node 1
    UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9); // Connect to server
    echoClient.SetAttribute("MaxPackets", UintegerValue(10));    // Send 10 packets
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); // 1 second interval
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));   // 1 KB packet size

    ApplicationContainer clientApp = echoClient.Install(nodes.Get(1));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Enable ASCII 
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll(ascii.CreateFileStream("csma.tr"));
    
    // Set up animation using NetAnim
    AnimationInterface anim("csma-animation.xml");
    anim.SetConstantPosition(nodes.Get(0), 1.0, 2.0);
    anim.SetConstantPosition(nodes.Get(1), 3.0, 4.0);
    anim.SetConstantPosition(nodes.Get(2), 5.0, 6.0);
    anim.SetConstantPosition(nodes.Get(3), 7.0, 8.0);

    // Start simulation
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

