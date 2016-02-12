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
#include "DataParser.h"


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
			
	Time sim_stop_time = Seconds (1000.0);
	
	std::string number_of_nodes_str = to_string(number_of_nodes);
	std::string data_rate_str = to_string(data_rate);
	std::string data_delay_str = to_string(data_delay);
	std::string transfer_data_str = to_string(transfer_data);
	std::string time_to_live_str = to_string(time_to_live);
	std::string number_of_streams_str = to_string(number_of_streams);
	std::string unordered_str = to_string(unordered);

	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	std:string timestamp = ("sim-"
							/*+ "-" + to_string(now->tm_year + 1900) 
							+ "-" + to_string(now->tm_mon) 
							+ "-" + to_string(now->tm_mday) 
							+ "_" + to_string(now->tm_hour) 
							+ "-" + to_string(now->tm_min) 
							+ "-" + to_string(now->tm_sec)*/
							+ to_string(transfer_data)
							+ "-" + to_string(clock()) + "_");

	NS_LOG_UNCOND("Simulation started\nNumber of nodes: " + number_of_nodes_str 
				+ "\nData rate: " + data_rate_str 
				+ " Mbps\nData delay: " + data_delay_str
				+ " ms\nNumber of bytes to transfer: " + transfer_data_str 
				+ "\nTime to live: " + time_to_live_str 
				+ "\nNumber of streams: " + number_of_streams_str + "\n");

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
		Ptr<Node> node = nodes.Get (n);
		RunIp(node, Seconds(0.2), "link show");
		RunIp(node, Seconds(0.3), "route show table all");
		RunIp(node, Seconds(0.4), "addr list");
	}

	DceApplicationHelper process;
	ApplicationContainer apps;

	// Server output pcap: myscripts-sctp-sim-0-0.pcap
	// Terminal output, run: cat files-0/var/log/*/stdout
	
	//process.SetBinary("my-tcp-receiver");
	process.SetBinary("my-sctp-receiver");
	process.ResetArguments();
		process.SetStackSize(1 << 16);
	apps = process.Install(nodes.Get(0));

	apps.Start(Seconds(1.0));

	// Clients output pcap: myscripts-sctp-sim-i-0.pcap
	// Terminal output, run: cat files-1/var/log/*/stdout
	for(int i = 1; i < number_of_nodes; i++) {
		//process.SetBinary("my-tcp-sender");
		process.SetBinary("my-sctp-sender");
		process.ResetArguments();
		process.AddArguments("-a", "10.0.0.1");
		process.AddArguments("-d", transfer_data_str); // amount of data per stream in bytes
		process.AddArguments("-t", time_to_live_str); // packets time to live in milliseconds (0 == ttl disabled)
		//process.AddArguments("-f", "bible.txt"); // TODO: this file needs to be copied to each client's files-* folder.
		process.AddArguments("-u", unordered_str); // un-ordered delivery of data
		process.AddArguments("-s", number_of_streams_str); // number of streams
		process.SetStackSize(1 << 16);
		apps = process.Install(nodes.Get(i));
		apps.Start(Seconds(1.5));
		
		AnimationInterface::SetConstantPosition (nodes.Get (i), 10*(i-1), 10);
	}
	// Setup NetAnim tracing
	AnimationInterface::SetConstantPosition (nodes.Get (0), ((double)number_of_nodes-2)*5.0, 0);
	AnimationInterface anim (timestamp + "-netanim.xml");
	anim.EnableIpv4L3ProtocolCounters (Seconds (0), sim_stop_time);
	anim.EnableQueueCounters (Seconds (0), sim_stop_time);
	anim.UpdateNodeColor (0, 255.0, 255.0, 0.0); // Yellow
	anim.UpdateNodeDescription (0, "server");
	for(int i = 1; i < number_of_nodes; i++) {
		anim.UpdateNodeDescription (i, "client" + to_string(i));
	}
	
	Simulator::Stop (sim_stop_time);
	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}

int main(int argc, char *argv[]) {
	int number_of_nodes = 2; // NOTE: must be at least 2
	int data_rate = 1; // Data rate for simulation in Mbps
	int data_delay = 30; // Server delay in ms
	int transfer_data_start = 1024; // Amount of bytes to send, starting value
	int transfer_data_end = 16384; // Amount of bytes to send, ending value
	int time_to_live = 0; // Time to live of packets in milliseconds (0 == ttl disabled)
	int number_of_streams = 1; // Number of sctp streams
	int unordered = 0;	// If packets should be sent in order
	
	int retransmission_timeout = 0;
	int hb_interval = 0;	
	
	int i;
	for(i = transfer_data_start; i <= transfer_data_end; i+=1024) {
		run_simulation(number_of_nodes, data_rate, data_delay, i, time_to_live, number_of_streams, unordered);
	}

	// Loop over all pcap files in current directory
	struct dirent **namelist;
	int num_files = scandir(".", &namelist, 0, alphasort);
	if (num_files <= 0) perror("Could not find pcap files!");
	else {
		for (i = 0; i < num_files; i++) {
			std::string filename = namelist[i]->d_name;
			// Parse only the server pcap files
			if(filename.substr(filename.find_last_of("_") + 1) == "-0-0.pcap") {
				// We found a pcap file, input it to graph handler
				size_t lastindex = filename.find_last_of("."); 
				std::string filename_no_ext = filename.substr(0, lastindex); 
				
				start_data_parser("sctp", filename_no_ext, "simtotal", "-printall");
			}
			free(namelist[i]);
		}
		free(namelist);
	}

}
