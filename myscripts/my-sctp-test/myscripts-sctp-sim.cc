#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <ios>

using namespace ns3;
using std::string;	
using std::fstream;

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
	int number_of_nodes, data_rate, data_delay, time_to_live, number_of_streams, retransmission_timeout, hb_interval = 0;
	std::string file_path = "";
	string nodes_line, data_rate_line, data_delay_line, ttl_line, streams_line, rto_line, hb_line;

	//Read config file path from command line interface.
	CommandLine cmd;
	cmd.AddValue("config_file", "Path to config file", file_path);
	cmd.Parse(argc, argv);
	
	//get setup information from file
	std::fstream config_file;

	config_file.open("/home/d0020e/ns3-dce-linux/source/ns-3-dce/myscripts/my-sctp-test/config", std::fstream::in | std::fstream::out | std::fstream::app);

	// Check if file is open
	if(config_file.is_open())
	{
		getline(config_file, nodes_line, '\n');
		getline(config_file, data_rate_line, '\n');
		getline(config_file, data_delay_line, '\n');
		getline(config_file, ttl_line, '\n');
		getline(config_file, streams_line, '\n');
		getline(config_file, rto_line, '\n');
		getline(config_file, hb_line, '\n');
	}else
	{
		return 0;
	}
	config_file.close();

	// Assign values
	number_of_nodes = atoi(nodes_line.c_str());
	data_rate = atoi(data_rate_line.c_str());
	data_delay = atoi(data_delay_line.c_str());
	time_to_live = atoi(ttl_line.c_str());
	number_of_streams = atoi(streams_line.c_str());
	retransmission_timeout = atoi(rto_line.c_str());
	hb_interval = atoi(hb_line.c_str());
	

	NodeContainer nodes;
	// Old setup
	//nodes.Create(2);
	nodes.Create(number_of_nodes);
	NetDeviceContainer devices;

	PointToPointHelper p2p;
	/* Old setup
	p2p.SetDeviceAttribute("DataRate", StringValue("5Gbps"));
	p2p.SetChannelAttribute("Delay", StringValue("1ms"));
	*/
	p2p.SetDeviceAttribute("DataRate", StringValue(data_rate + "Mbps"));
	p2p.SetChannelAttribute("Delay", StringValue(data_delay + "ms"));
	devices = p2p.Install(nodes);
	p2p.EnablePcapAll("myscripts-sctp-sim");

	DceManagerHelper processManager;
	processManager.SetTaskManagerAttribute("FiberManagerType", StringValue("UcontextFiberManager"));
	// processManager.SetLoader ("ns3::DlmLoaderFactory");
	processManager.SetNetworkStack("ns3::LinuxSocketFdFactory", "Library", StringValue("liblinux.so"));
	LinuxStackHelper stack;
	stack.Install(nodes);

	Ipv4AddressHelper address;
	address.SetBase("10.0.0.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = address.Assign(devices);

	processManager.Install(nodes);

	for (int n = 0; n < number_of_nodes; n++) {
		RunIp(nodes.Get(n), Seconds(0.2 + n), "link show");
		RunIp(nodes.Get(n), Seconds(0.3 + n), "route show table all");
		RunIp(nodes.Get(n), Seconds(0.4 + n), "addr list");
	}

	DceApplicationHelper process;
	ApplicationContainer apps;

	// Output pcap: myscripts-sctp-sim-0-0.pcap
	// Terminal output, run: cat files-0/var/log/*/stdout
	process.SetBinary("my-sctp-server");
	process.ResetArguments();
	process.SetStackSize(1 << 16);
	apps = process.Install(nodes.Get(0));
	apps.Start(Seconds(number_of_nodes + 2));

	// Output pcap: myscripts-sctp-sim-1-0.pcap
	// Terminal output, run: cat files-1/var/log/*/stdout

	for(int i = 1; i <= number_of_nodes; i++)
	{
		process.SetBinary("my-sctp-client-" + i);
		process.ResetArguments();
		process.ParseArguments("10.0.0." + i);
		apps = process.Install(nodes.Get(i));
		apps.Start(Seconds(number_of_nodes + 2 +i));
	}	

	/* Old setup for clients
	process.SetBinary("my-sctp-client");
	process.ResetArguments();
	process.ParseArguments("10.0.0.1");
	apps = process.Install(nodes.Get(1));
	apps.Start(Seconds(1.5));
	*/
	Simulator::Stop (Seconds (1000.0));
	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}
