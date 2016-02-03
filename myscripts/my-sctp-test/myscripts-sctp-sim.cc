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
#include <ctime>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>

using std::string;	
using std::fstream;
NS_LOG_COMPONENT_DEFINE ("HelloSimulator");


template <typename T> std::string to_string(T value) {
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

using namespace ns3;	

static void RunIp (Ptr<Node> node, Time at, std::string str) {
	DceApplicationHelper process;
	ApplicationContainer apps;
	process.SetBinary ("ip");
	process.SetStackSize (1<<16);
	process.ResetArguments();
	process.ParseArguments(str.c_str ());
	apps = process.Install (node);
	apps.Start (at);
}

int run_simulation(int number_of_nodes, int data_rate, int data_delay, 
		int transfer_data, int time_to_live, int number_of_streams, int unordered) {

	std::string number_of_nodes_str = to_string(number_of_nodes);
	std::string data_rate_str = to_string(data_rate);
	std::string data_delay_str = to_string(data_delay);
	std::string transfer_data_str = to_string(transfer_data);
	std::string time_to_live_str = to_string(time_to_live);
	std::string number_of_streams_str = to_string(number_of_streams);
	std::string unordered_str = to_string(unordered);

	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	clock_t c = clock();
	std::string clock_str = to_string(c);
std:string timestamp = ("sim-" + to_string(now->tm_year + 1900) + "-" + to_string(now->tm_mon) + "-" + to_string(now->tm_mday) + "_" 
			+ to_string(now->tm_hour) + ":" + to_string(now->tm_min) + ":" + to_string(now->tm_sec) + ":" + clock_str);

	NS_LOG_UNCOND("Simulation started\nNumber of nodes: " + number_of_nodes_str + "\nData rate: " + data_rate_str + " Mbps\nData delay: " + data_delay_str
			+ " ms\nNumber of bytes to transfer: " + transfer_data_str + "\nTime to live: " + time_to_live_str + "\nNumber of streams: " + number_of_streams_str + "\n");

	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

	NodeContainer nodes;
	nodes.Create(number_of_nodes);

	NetDeviceContainer devices;

	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue(data_rate_str + "Mbps"));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(data_delay)));
	devices = csma.Install(nodes);

	csma.EnablePcapAll(timestamp);

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
		RunIp(nodes.Get(n), Seconds(0.2), "link show");
		RunIp(nodes.Get(n), Seconds(0.3), "route show table all");
		RunIp(nodes.Get(n), Seconds(0.4), "addr list");
	}

	DceApplicationHelper process;
	ApplicationContainer apps;

	// Server output pcap: myscripts-sctp-sim-0-0.pcap
	// Terminal output, run: cat files-0/var/log/*/stdout
	process.SetBinary("my-sctp-server");
	process.ResetArguments();

	process.AddArguments("-d", transfer_data_str); // amount of data per stream in bytes
	process.AddArguments("-t", time_to_live_str); // packets time to live in milliseconds (0 == ttl disabled)
	//process.AddArguments("-f", "bible.txt"); // file to send
	process.AddArguments("-u", unordered_str); // un-ordered delivery of data
	process.AddArguments("-s", number_of_streams_str); // number of streams

	process.SetStackSize(1 << 16);
	apps = process.Install(nodes.Get(0));
	apps.Start(Seconds(1.0));

	// Clients output pcap: myscripts-sctp-sim-i-0.pcap
	// Terminal output, run: cat files-1/var/log/*/stdout
	for(int i = 1; i < number_of_nodes; i++) {
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

int main(int argc, char *argv[]) {
	int number_of_nodes = 2; // NOTE: must be at least 2
	int data_rate = 1; // Data rate for simulation in Mbps
	int data_delay = 30; // Server delay in ms
	int transfer_data_start = 256; // Amount of bytes to send, starting value
	int transfer_data_end = 65536*1024; // Amount of bytes to send, ending value
	int time_to_live = 50; // Time to live of packets in milliseconds (0 == ttl disabled)
	int number_of_streams = 2; // Number of sctp streams
	int unordered = 0;	// If packets should be sent in order
	
	int retransmission_timeout = 0;
	int hb_interval = 0;	


	for(int i = transfer_data_start; i <= transfer_data_end; i*=2) {
		run_simulation(number_of_nodes, data_rate, data_delay, i, time_to_live, number_of_streams, unordered);
	}
	
	struct dirent **namelist;
	int num_files, i;
	num_files = scandir(".", &namelist, 0, alphasort);
	if (num_files <= 0)
		perror("The directory is empty");
	else {
		for (i = 0; i < num_files; i++) {
			std::string filename = namelist[i]->d_name;
			if(filename.substr(filename.find_last_of(".") + 1) == "pcap") {
				NS_LOG_UNCOND(filename);
			}
			free(namelist[i]);
		}
		free(namelist);
	}
}
