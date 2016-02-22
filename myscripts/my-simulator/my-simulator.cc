#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
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
	SCTP, TCP, DCCP
};
static const char * PROTOCOL_NAMES[] = { "sctp", "tcp", "dccp" };
static const char * PROROCOL_CLIENTS[] = { "dce-client-sctp", "dce-client-tcp", "dce-client-dccp" };
static const char * PROROCOL_SERVERS[] = { "dce-server-sctp", "dce-server-tcp", "dce-server-dccp" };
const char * getNameForProtocol(int p) {
	return PROTOCOL_NAMES[p];
}
const char * getClientForProtocol(int p) {
	return PROROCOL_CLIENTS[p];
}
const char * getServerForProtocol(int p) {
	return PROROCOL_SERVERS[p];
}

void my_handler(int s){
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

int run_simulation(Protocol protocol, const char* output_dir, int number_of_nodes, int data_rate, int data_delay,
		int transfer_data, int time_to_live, int number_of_streams, int unordered, int num_cycles, int time_between_cycles) {

	Time sim_stop_time = Seconds(10000.0);

	std::string number_of_nodes_str = to_string(number_of_nodes);
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
	std::string output_filename = output_dir + protocol_name_str + "-data-" + transfer_data_str + "-sim-" + sim_id
			+ "-node";

	NS_LOG_UNCOND("Simulation id:\t" + sim_id);
	NS_LOG_UNCOND("Protocol:\t" + protocol_name_str);
	NS_LOG_UNCOND("Node amount:\t" + number_of_nodes_str);
	NS_LOG_UNCOND("Data rate:\t" + data_rate_str + " Mbps");
	NS_LOG_UNCOND("Data delay:\t" + data_delay_str + " ms");
	NS_LOG_UNCOND("Client payload:\t" + transfer_data_str + " bytes");
	NS_LOG_UNCOND("Time to live:\t" + time_to_live_str + " ms");
	NS_LOG_UNCOND("Streams:\t" + number_of_streams_str + " per client");
	NS_LOG_UNCOND("");

	GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

	NodeContainer nodes;
	nodes.Create(number_of_nodes);

	NetDeviceContainer devices;

	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue(data_rate_str + "Mbps"));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(data_delay)));
	devices = csma.Install(nodes);

	csma.EnablePcapAll(output_filename);

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

	// Server output pcap: sim-[protocol]-[size]-[clock]_-0-0.pcap
	// Terminal output, run: cat files-0/var/log/*/stdout
	process.SetBinary(getServerForProtocol(protocol));
	process.SetStackSize(1 << 16);
	process.ResetArguments();
	apps = process.Install(nodes.Get(0));

	apps.Start(Seconds(4.0));

	// Clients output pcap: sim-[protocol]-[size]-[clock]_-[i]-0.pcap
	// Terminal output, run: cat files-i/var/log/*/stdout
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

	Simulator::Stop(sim_stop_time);
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}

int main(int argc, char *argv[]) { 
	int number_of_nodes = 5; // NOTE: must be at least 2, one server, one client
	int data_rate = 5; // Data rate for simulation in Mbps
	int data_delay = 2; // Server delay in ms
	int transfer_data_start = 102400; // Amount of bytes to send, starting value
	int transfer_data_end = 1024000; // Amount of bytes to send, ending value
	int time_to_live = 0; // Time to live of packets in milliseconds (0 == ttl disabled)
	int number_of_streams = 4; // Number of sctp streams
	int unordered = 0;	// If packets should be sent in order
	
	int num_cycles = 1; // How many cycles to run in on/off-mode, in seconds
	int time_between_cycles = 2; // How long to wait during on/off-mode, in seconds
	
	int retransmission_timeout = 0; // TODO
	int hb_interval = 0; // TODO

	const char* output_dir = "my-simulator-output/";

	// Catch ctrl+c
	signal (SIGINT, my_handler);
	// Clear output directory if it exists
	std::string command = "rm -rf " + std::string(output_dir);
	system(command.c_str());
	mkdir(output_dir, 0750);
	// Clear DCE file system
	system("rm -rf files-*");
	// Run the simulation
	int i, j;
	for (i = transfer_data_start; i <= transfer_data_end; i += 1024) {
		run_simulation(SCTP, output_dir, number_of_nodes, data_rate, data_delay, i, time_to_live, number_of_streams, unordered, num_cycles, time_between_cycles);
		run_simulation(TCP, output_dir, number_of_nodes, data_rate, data_delay, i, time_to_live, number_of_streams, unordered, num_cycles, time_between_cycles);
		//~ run_simulation(DCCP, output_dir, number_of_nodes, data_rate, data_delay, i, time_to_live, number_of_streams, unordered);
	}
	
	// Loop over all pcap files in current directory
	struct dirent **namelist;
	int num_files = scandir(output_dir, &namelist, 0, alphasort);
	if (num_files <= 0)
		perror("Could not find pcap files!");
	else {
		for (i = 0; i < num_files; i++) {
			std::string filename = std::string(output_dir) + namelist[i]->d_name;
			// Parse only the server pcap files
			if (filename.substr(filename.find_last_of("node") + 1) == "-0-0.pcap") {
				// We found a server pcap file, input it to graph handler
				size_t lastindex = filename.find_last_of(".");
				std::string filename_no_ext = filename.substr(0, lastindex);

				for (j = 0; j < 3; j++) {
					const char * protocol = PROTOCOL_NAMES[j];
					std::string protocol_str = std::string(protocol);
					std::string output_dir_str = std::string(output_dir);
					std::string filename_totals = output_dir_str + protocol_str + "_simtotal";
					if (filename.find(output_dir_str + protocol_str) == 0) {
						start_data_parser(protocol_str, filename_no_ext, filename_totals, "-print");
					}
				}

			}
		}
	}

}
