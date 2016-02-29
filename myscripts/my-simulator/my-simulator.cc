#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
   
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <ios>
#include <ctime>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "DataParser.h"

using std::string;
using std::fstream;
NS_LOG_COMPONENT_DEFINE ("HelloSimulator");

template<typename T> std::string to_string(T value) {
	std::ostringstream os;
	os << value;
	return os.str();
}

enum Protocol {
	SCTP, TCP, UDP, DCCP
};
static const int NUM_PROTOCOLS = 4;
static const char * PROTOCOL_NAMES[] = { "sctp", "tcp", "udp", "dccp" };
static const char * PROROCOL_CLIENTS[] = { "dce-client-sctp", "dce-client-tcp", "dce-client-udp", "dce-client-dccp" };
static const char * PROROCOL_SERVERS[] = { "dce-server-sctp", "dce-server-tcp", "dce-server-udp", "dce-server-dccp" };
const char * getNameForProtocol(int p) {
	return PROTOCOL_NAMES[p];
}
const char * getClientForProtocol(int p) {
	return PROROCOL_CLIENTS[p];
}
const char * getServerForProtocol(int p) {
	return PROROCOL_SERVERS[p];
}

void my_handler(int s) {
	exit(1);
}

using namespace ns3;

static void RunIp(Ptr<Node> node, Time at, std::string str) {
	DceApplicationHelper process;
	ApplicationContainer apps;
	process.SetBinary("ip");
	process.SetStackSize(1 << 16);
	process.ResetArguments();
	process.ParseArguments(str.c_str());
	apps = process.Install(node);
	apps.Start(at);
}

std::string run_simulation(Protocol protocol, const char* output_dir, int number_of_clients, int data_rate,
		int data_delay, int transfer_data, int time_to_live, int number_of_streams, int unordered, int num_cycles,
		int time_between_cycles) {

	Time sim_stop_time = Seconds(10000.0);

	std::string number_of_clients_str = to_string(number_of_clients);
	std::string data_rate_str = to_string(data_rate);
	std::string data_delay_str = to_string(data_delay);
	std::string transfer_data_str = to_string(transfer_data);
	std::string time_to_live_str = to_string(time_to_live);
	std::string number_of_streams_str = to_string(number_of_streams);
	std::string unordered_str = to_string(unordered);
	std::string protocol_name_str = to_string(getNameForProtocol(protocol));
	std::string num_cycles_str = to_string(num_cycles);
	std::string time_between_cycles_str = to_string(time_between_cycles);
	std::string sim_id = to_string(clock());
	std::string output_filename = output_dir + protocol_name_str + "-data-" + transfer_data_str + "-sim-" + sim_id + "-node";

	cout << endl << "********************************************************************************" << endl;
	cout << left << setw(28) << "Simulation id:" << sim_id << endl;
	cout << left << setw(28) << "Protocol:" << protocol_name_str << endl;
	cout << left << setw(28) << "Client amount:" << number_of_clients_str << endl;
	cout << left << setw(28) << "Data rate:" << data_rate_str << " Mbps" << endl;
	cout << left << setw(28) << "Data delay:" << data_delay_str << " ms" << endl;
	cout << left << setw(28) << "Client payload:" << transfer_data_str << "*" << num_cycles << " bytes" << endl;
	cout << left << setw(28) << "Time to live:" << time_to_live_str << " ms" << endl;
	cout << left << setw(28) << "Streams:" << number_of_streams_str << endl;

	GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

	int number_of_nodes = number_of_clients + 1;
	NodeContainer nodes;
	nodes.Create(number_of_nodes);

	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	wifiMac.SetType ("ns3::AdhocWifiMac");
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());
	NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);
    wifiPhy.EnablePcap(output_filename, devices);
    	
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

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

	for (int n = 0; n < number_of_nodes; n++) {
		Ptr < Node > node = nodes.Get(n);
		RunIp(node, Seconds(0.2), "link show");
		RunIp(node, Seconds(0.3), "route show table all");
		RunIp(node, Seconds(0.4), "addr list");
	}

	DceApplicationHelper process;
	ApplicationContainer apps;

	process.SetBinary(getServerForProtocol(protocol));
	process.SetStackSize(1 << 16);
	process.ResetArguments();
	apps = process.Install(nodes.Get(0));
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));

	apps.Start(Seconds(4.0));

	for (int i = 1; i < number_of_nodes; i++) {
		process.SetBinary(getClientForProtocol(protocol));
		process.SetStackSize(1 << 16);
		process.ResetArguments();
		process.AddArguments("-a", "10.0.0.1");
		process.AddArguments("-d", transfer_data_str); // amount of data per stream in bytes
		process.AddArguments("-t", time_to_live_str); // packets time to live in milliseconds (0 == ttl disabled)
		process.AddArguments("-u", unordered_str); // un-ordered delivery of data
		process.AddArguments("-s", number_of_streams_str); // number of streams
		process.AddArguments("-b", time_between_cycles_str);
		process.AddArguments("-n", num_cycles_str);
		apps = process.Install(nodes.Get(i));
		apps.Start(Seconds(4.5));

		float x = 10 * (i - 1);
		float y = 40 * sin(3.14 * ((float) i - 1) / ((float) number_of_nodes - 2));
		positionAlloc->Add (Vector (x, y, 0.0));
		AnimationInterface::SetConstantPosition(nodes.Get(i), x, y);
	}
	// Setup NetAnim tracing
	AnimationInterface::SetConstantPosition(nodes.Get(0), ((double) number_of_nodes - 2) * 5.0, 0);
	AnimationInterface anim(output_filename + "-netanim.xml");
	anim.EnableIpv4L3ProtocolCounters(Seconds(0), sim_stop_time);
	anim.EnableQueueCounters(Seconds(0), sim_stop_time);
	anim.UpdateNodeColor(0, 255.0, 255.0, 0.0); // Yellow
	anim.UpdateNodeDescription(0, "server");
	for (int i = 1; i < number_of_nodes; i++) {
		anim.UpdateNodeDescription(i, "client" + to_string(i));
	}
	mobility.SetPositionAllocator (positionAlloc);
	mobility.Install (nodes);
	Simulator::Stop(sim_stop_time);
	Simulator::Run();
	Simulator::Destroy();

	// Parse packet capture file
	std::string server_pcap = output_filename + "-0-0";
	std::string simtotal = output_dir + protocol_name_str + "_simtotal";
	cout << left << setw(28) << "Protocol totals file:" << simtotal << endl;
	cout << left << setw(28) << "Server pcap:" << server_pcap << ".pcap" << endl;
	start_data_parser(protocol_name_str, number_of_clients, transfer_data * num_cycles, server_pcap, simtotal, "-print");

	return output_filename;
}

int main(int argc, char *argv[]) {

	// Number of client network nodes. There is only one server node.
	int number_of_clients = 4;

	// CSMA settings
	int data_rate = 5; // Data rate for simulation in Mbps
	int data_delay = 2; // Server delay in ms

	// Amount of bytes to send
	int transfer_data_start = 102400;
	int transfer_data_inc = 1024;
	int transfer_data_end = 102400;

	// SCTP settings
	int time_to_live = 0; // Time to live of packets in milliseconds (0 == ttl disabled)
	int number_of_streams = 4; // Number of streams
	int unordered = 0;	// If packets should be sent in order

	// How many cycles to run in on/off-source. Each added cycle multplies the amount of data sent.
	int num_cycles = 2; // Amount of cycles
	int time_between_cycles = 2; // How long to wait between cycles in seconds

	// Packet capture, NetAnim and parsing files are put into this directory
	const char* output_dir = "my-simulator-output/";

	// Catch ctrl+c
	signal(SIGINT, my_handler);
	// Clear output directory if it exists
	std::string command = "rm -rf " + std::string(output_dir);
	system(command.c_str());
	mkdir(output_dir, 0750);
	// Clear DCE node file system
	system("rm -rf files-*");
	// Run the simulation
	int i;
	for (i = transfer_data_start; i <= transfer_data_end; i += transfer_data_inc) {
		run_simulation(SCTP, output_dir, number_of_clients, data_rate, data_delay, i, time_to_live, number_of_streams, unordered, num_cycles, time_between_cycles);
		run_simulation(TCP, output_dir, number_of_clients, data_rate, data_delay, i, time_to_live, number_of_streams, unordered, num_cycles, time_between_cycles);
		run_simulation(UDP, output_dir, number_of_clients, data_rate, data_delay, i, time_to_live, number_of_streams, unordered, num_cycles, time_between_cycles);
		//~ run_simulation(DCCP, output_dir, number_of_nodes, data_rate, data_delay, i, time_to_live, number_of_streams, unordered);
	}

}
