#include "ns3/core-module.h"
#include "ns3/dce-module.h"
#include "ns3/netanim-module.h"
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

template<typename T> std::string to_string(T value) {
	std::ostringstream os;
	os << value;
	return os.str();
}

void sigint_handler(int s) {
	kill(::getpid(), 1);
	exit(1);
}

enum Protocol {
	SCTP, TCP, UDP, DCCP
};
static const char * PROTOCOL_NAMES[] = { "sctp", "tcp", "udp", "dccp" };
static const char * PROROCOL_CLIENTS[] = { "dce-client-sctp", "dce-client-tcp", "dce-client-udp", "dce-client-dccp" };
static const char * PROROCOL_SERVERS[] = { "dce-server-sctp", "dce-server-tcp", "dce-server-udp", "dce-server-dccp" };
const char * GetNameForProtocol(int p) {
	return PROTOCOL_NAMES[p];
}
const char * GetClientForProtocol(int p) {
	return PROROCOL_CLIENTS[p];
}
const char * GetServerForProtocol(int p) {
	return PROROCOL_SERVERS[p];
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

struct SimSettings {
	std::string output_dir;
	WifiPhyStandard wifi_std;
	std::string wifi_mode;
	double sim_stop_time_sec;
	int number_of_clients;
	int transfer_data_bytes;
	int sctp_ttl_ms;
	int num_sockets_streams;
	int sctp_unordered;
	int udp_send_rate_kbytes_sec;
	int dccp_send_rate_kbytes_sec;
	int num_cycles;
	int time_between_cycles_usec;
};

void RunSimulation(Protocol protocol, SimSettings &sim_settings) {

	Time sim_stop_time = Seconds(sim_settings.sim_stop_time_sec);
	std::string transfer_data_bytes_str = to_string(sim_settings.transfer_data_bytes);
	std::string sctp_ttl_ms_str = to_string(sim_settings.sctp_ttl_ms);
	std::string num_sockets_streams_str = to_string(sim_settings.num_sockets_streams);
	std::string sctp_unordered_str = to_string(sim_settings.sctp_unordered);
	std::string protocol_name_str = to_string(GetNameForProtocol(protocol));
	std::string num_cycles_str = to_string(sim_settings.num_cycles);
	std::string time_between_cycles_usec_str = to_string(sim_settings.time_between_cycles_usec);
	std::string udp_send_rate_kbytes_sec_str = to_string(sim_settings.udp_send_rate_kbytes_sec);
	std::string dccp_send_rate_kbytes_sec_str = to_string(sim_settings.dccp_send_rate_kbytes_sec);

	std::string sim_id = to_string(clock());
	std::string output_filename = sim_settings.output_dir + protocol_name_str + "-data-" + transfer_data_bytes_str + "-sim-" + sim_id + "-node";
	
	cout << endl << "********************************************************************************" << endl;
	cout << "NS-3 simulation ID(" << sim_id << "), " << protocol_name_str << " protocol.";
	cout << endl << "********************************************************************************" << endl;
	cout << left << setw(28) << "Client payload:" << transfer_data_bytes_str << "*" << sim_settings.num_cycles << " bytes" << endl;
	cout << left << setw(28) << "Client amount:" << to_string(sim_settings.number_of_clients) << endl;
	cout << left << setw(28) << "SCTP ttl:" << sctp_ttl_ms_str << " ms" << endl;
	cout << left << setw(28) << "SCTP streams:" << num_sockets_streams_str << endl;
	cout << left << setw(28) << "SCTP unordered:" << sctp_unordered_str << endl;
	cout << left << setw(28) << "DCCP target rate:" << dccp_send_rate_kbytes_sec_str << " KB/s" << endl;
	cout << left << setw(28) << "UDP target rate:" << udp_send_rate_kbytes_sec_str << " KB/s" << endl;
	cout << left << setw(28) << "Num send cycles:" << num_cycles_str << endl;
	cout << left << setw(28) << "Time between cycles:" << time_between_cycles_usec_str << " usec" << endl;

	GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

	int number_of_nodes = sim_settings.number_of_clients + 1;
	NodeContainer nodes;
	nodes.Create(number_of_nodes);

	// Setup wifi
	WifiHelper wifi = WifiHelper::Default();
	wifi.SetStandard(sim_settings.wifi_std);
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", 
		"DataMode", StringValue(sim_settings.wifi_mode),
		"ControlMode", StringValue(sim_settings.wifi_mode));
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
	wifiMac.SetType("ns3::AdhocWifiMac");
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
	wifiPhy.SetChannel(wifiChannel.Create());
	NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);
	wifiPhy.EnablePcap(output_filename, devices);

	// Setup mobility (random node movement within some limits)
	MobilityHelper mobilityAdhoc;
	int64_t streamIndex = 0; // used to get consistent mobility across scenarios
	ObjectFactory pos;
	pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
	pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"));
	pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"));
	Ptr < PositionAllocator > taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
	streamIndex += taPositionAlloc->AssignStreams(streamIndex);
	std::string ssSpeed = "ns3::UniformRandomVariable[Min=0.0|Max=20]"; // Node mobility speed in m/s
	std::string ssPause = "ns3::ConstantRandomVariable[Constant=0]"; // Node mobility pause in seconds
	mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel", "Speed", StringValue(ssSpeed), "Pause", StringValue(ssPause), "PositionAllocator", PointerValue(taPositionAlloc));
	mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
	mobilityAdhoc.Install(nodes);
	streamIndex += mobilityAdhoc.AssignStreams(nodes, streamIndex);

	// Setup DCE
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

	// Print IP stats for the nodes, to stdout (stored in the DCE files-* folders)
	for (int n = 0; n < number_of_nodes; n++) {
		Ptr < Node > node = nodes.Get(n);
		RunIp(node, Seconds(0.2), "link show");
		RunIp(node, Seconds(0.3), "route show table all");
		RunIp(node, Seconds(0.4), "addr list");
	}

	DceApplicationHelper process;
	ApplicationContainer apps;

	// Start DCE server program
	process.SetBinary(GetServerForProtocol(protocol));
	process.SetStackSize(1 << 16);
	process.ResetArguments();
	apps = process.Install(nodes.Get(0));
	apps.Start(Seconds(4.0));

	// Start DCE client programs
	for (int i = 1; i < number_of_nodes; i++) {
		process.SetBinary(GetClientForProtocol(protocol));
		process.SetStackSize(1 << 16);
		process.ResetArguments();
		process.AddArguments("-a", "10.0.0.1");
		process.AddArguments("-d", transfer_data_bytes_str);
		process.AddArguments("-t", sctp_ttl_ms_str);
		process.AddArguments("-u", sctp_unordered_str);
		process.AddArguments("-s", num_sockets_streams_str);
		process.AddArguments("-b", time_between_cycles_usec_str);
		process.AddArguments("-n", num_cycles_str);
		process.AddArguments("-p", udp_send_rate_kbytes_sec_str);
		process.AddArguments("-r", dccp_send_rate_kbytes_sec_str);
		apps = process.Install(nodes.Get(i));
		apps.Start(Seconds(4.5));
	}

	// NetAnim tracing
	AnimationInterface anim(output_filename + "-netanim.xml");
	anim.EnableIpv4L3ProtocolCounters(Seconds(0), sim_stop_time);
	anim.EnableQueueCounters(Seconds(0), sim_stop_time);
	anim.UpdateNodeColor(0, 255, 255, 0); // Yellow colored server
	anim.UpdateNodeDescription(0, "server");
	for (int i = 1; i < number_of_nodes; i++) {
		anim.UpdateNodeDescription(i, "client" + to_string(i));
	}

	Simulator::Stop(sim_stop_time);
	Simulator::Run();
	Simulator::Destroy();

	// Parse packet capture file
	std::string server_pcap = output_filename + "-0-0";
	std::string simtotal = sim_settings.output_dir + protocol_name_str + "_simtotal";
	cout << left << setw(28) << "Protocol totals file:" << simtotal << endl;
	cout << left << setw(28) << "Server pcap:" << server_pcap << ".pcap" << endl;
	start_data_parser(protocol_name_str, sim_settings.number_of_clients, sim_settings.transfer_data_bytes * sim_settings.num_cycles, sim_settings.num_sockets_streams, server_pcap, simtotal, "-print");
}

int main(int argc, char *argv[]) {
	
	SimSettings sim_settings;
	
	// Wifi settings
	sim_settings.wifi_std = WIFI_PHY_STANDARD_80211a;
	sim_settings.wifi_mode = "OfdmRate54Mbps";
	
	// Max time to run a simulation, in seconds
	sim_settings.sim_stop_time_sec = 10000;
	
	// Number of client network nodes. There is only one server node.
	sim_settings.number_of_clients = 1;

	// Amount of data each client sends, in bytes.
	sim_settings.transfer_data_bytes = 1024;
	
	// Number of streams to create for SCTP simulation
	// and/or sockets to create for UDP, TCP, DCCP.
	sim_settings.num_sockets_streams = 4;

	// SCTP settings
	sim_settings.sctp_ttl_ms = 0; // Time to live of packets in milliseconds (0 == ttl disabled)
	sim_settings.sctp_unordered = 0; // Unordered packet delivery

	// UDP and DCCP approximate target send rate in kilobytes/second.
	// Will not be exact since packet header size is not accounted for.
	sim_settings.udp_send_rate_kbytes_sec = 700;
	sim_settings.dccp_send_rate_kbytes_sec = 700;

	// How many cycles to run in on/off-source.
	// Total data sent per client = transfer_data_bytes * num_cycles
	sim_settings.num_cycles = 1;
	// How long to wait between cycles in usec
	sim_settings.time_between_cycles_usec = 2000000;

	// Packet capture, NetAnim and pcap summary files are put into this directory
	sim_settings.output_dir = "my-simulator-output/";

	// Clear output directory if it exists
	std::string command = "rm -rf " + sim_settings.output_dir;
	system(command.c_str());
	mkdir(sim_settings.output_dir.c_str(), 0750);
	// Clear DCE node file systems
	system("rm -rf files-*");
	// Catch ctrl+c to force kill the process, exit does not seem to be enough...
	signal(SIGINT, sigint_handler);

	// Run the simulations over a variable span.
	int min = 1024 * 1000;
	int inc = 1024 * 10;
	int max = 1024 * 10000;
	int var;
	for (var = min; var <= max; var += inc) {
		sim_settings.transfer_data_bytes = var;
		RunSimulation(TCP, sim_settings);
		RunSimulation(SCTP, sim_settings);
		RunSimulation(UDP, sim_settings);
		RunSimulation(DCCP, sim_settings);
	}

}
