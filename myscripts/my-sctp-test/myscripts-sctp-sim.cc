#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <ios>



using std::string;	
using std::fstream;
NS_LOG_COMPONENT_DEFINE ("HelloSimulator");


template <typename T>
std::string to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}
		
using namespace ns3;	

static void RunIp (Ptr<Node> node, Time at, std::string str)
{
	DceApplicationHelper process;
	ApplicationContainer apps;
	process.SetBinary ("ip");
	process.SetStackSize (1<<16);
	process.ResetArguments();
	process.ParseArguments(str.c_str ());
	apps = process.Install (node);
	apps.Start (at);
}

int main(int argc, char *argv[]) {
	int number_of_nodes = 2; // NOTE: must be at least 2
	int data_rate = 1;
	int data_delay = 30;
	int time_to_live, number_of_streams, retransmission_timeout, hb_interval = 0;
	//uint32_t nPackets = 1;
	//CommandLine cmd;
	//cmd.AddValue("nPackets", "Number of packets to echo", nPackets);
	//cmd.Parse(argc, argv);
	std::string number_of_nodes_str = to_string(number_of_nodes);
	std::string data_rate_str = to_string(data_rate);
	std:string data_delay_str = to_string(data_delay);

	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

	NodeContainer nodes;
	nodes.Create(number_of_nodes);
	NS_LOG_UNCOND("Number of nodes created: " + number_of_nodes_str);
	
	NetDeviceContainer devices;

	//PointToPointHelper p2p;
	//NS_LOG_UNCOND("Data rate set to: " + data_rate_str + "Mbps");
	//p2p.SetDeviceAttribute("DataRate", StringValue(data_rate_str + "Mbps"));
	//NS_LOG_UNCOND("Data delay set to: " + data_delay_str + "ms");
	//p2p.SetChannelAttribute("Delay", StringValue(data_delay_str +"ms"));
	//NS_LOG_UNCOND("pre node installation");
	//devices = p2p.Install(nodes);
	//NS_LOG_UNCOND("post node installation");
	//p2p.EnablePcapAll("myscripts-sctp-sim");
	
	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue(data_rate_str + "Mbps"));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(data_delay)));
	devices = csma.Install(nodes);
	
	csma.EnablePcapAll ("myscripts-sctp-sim");

	DceManagerHelper processManager;
	processManager.SetTaskManagerAttribute("FiberManagerType", StringValue("UcontextFiberManager"));
	// processManager.SetLoader ("ns3::DlmLoaderFactory");
	processManager.SetNetworkStack("ns3::LinuxSocketFdFactory", "Library", StringValue("liblinux.so"));
	LinuxStackHelper stack;
	stack.Install(nodes);

	Ipv4AddressHelper address;
	address.SetBase("10.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer interfaces = address.Assign(devices);

	processManager.Install(nodes);
	NS_LOG_UNCOND("Installed nodes");

	for (int n = 0; n < number_of_nodes; n++) {
		RunIp(nodes.Get(n), Seconds(0.2), "link show");
		RunIp(nodes.Get(n), Seconds(0.3), "route show table all");
		RunIp(nodes.Get(n), Seconds(0.4), "addr list");
	}

	DceApplicationHelper process;
	ApplicationContainer apps;

	// Output pcap: myscripts-sctp-sim-0-0.pcap
	// Terminal output, run: cat files-0/var/log/*/stdout
	process.SetBinary("my-sctp-server");
	process.ResetArguments();
	process.SetStackSize(1 << 16);
	apps = process.Install(nodes.Get(0));
	apps.Start(Seconds(1.0));

	// Output pcap: myscripts-sctp-sim-1-0.pcap
	// Terminal output, run: cat files-1/var/log/*/stdout
	for(int i = 1; i < number_of_nodes; i++)
	{
		process.SetBinary("my-sctp-client");
		process.ResetArguments();
		process.ParseArguments("10.0.0.1");
		apps = process.Install(nodes.Get(i));
		apps.Start(Seconds(1.5));
	}

	Simulator::Stop (Seconds (1000.0));
	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}
