#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    // Enable logging
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

    // Create nodes
    NodeContainer nodes;
    nodes.Create(6); // 2 end nodes + 4 routers

    // Create links between nodes using Point-to-Point
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices[5];
    devices[0] = p2p.Install(nodes.Get(0), nodes.Get(1)); // Client to Router1
    devices[1] = p2p.Install(nodes.Get(1), nodes.Get(2)); // Router1 to Router2
    devices[2] = p2p.Install(nodes.Get(2), nodes.Get(3)); // Router2 to Router3
    devices[3] = p2p.Install(nodes.Get(3), nodes.Get(4)); // Router3 to Router4
    devices[4] = p2p.Install(nodes.Get(4), nodes.Get(5)); // Router4 to Server

    // Install the Internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    Ipv4InterfaceContainer interfaces[5];
    for (uint32_t i = 0; i < 5; ++i) {
        std::ostringstream subnet;
        subnet << "10.1." << i+1 << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        interfaces[i] = address.Assign(devices[i]);
    }

    // Set up applications
    // Client (Node 0) -> Server (Node 5)
    uint16_t port = 9;
    Address serverAddress(InetSocketAddress(interfaces[4].GetAddress(1), port));

    // Install Server Application
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer serverApp = packetSinkHelper.Install(nodes.Get(5));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // Install Client Application
    OnOffHelper clientHelper("ns3::TcpSocketFactory", serverAddress);
    clientHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    clientHelper.SetAttribute("DataRate", StringValue("500Kbps"));
    ApplicationContainer clientApp = clientHelper.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(9.0));

    // Enable tracing
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream("point_to_point.tr"));
    p2p.EnablePcapAll("point_to_point");

    // Configure NetAnim
    AnimationInterface anim("point_to_point.xml");
    anim.SetConstantPosition(nodes.Get(0), 0.0, 50.0);  // Client
    anim.SetConstantPosition(nodes.Get(1), 20.0, 50.0); // Router 1
    anim.SetConstantPosition(nodes.Get(2), 40.0, 50.0); // Router 2
    anim.SetConstantPosition(nodes.Get(3), 60.0, 50.0); // Router 3
    anim.SetConstantPosition(nodes.Get(4), 80.0, 50.0); // Router 4
    anim.SetConstantPosition(nodes.Get(5), 100.0, 50.0); // Server


    // Run simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
