#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include "DataParser.h"

using namespace std;
int NUM_OF_WORDS = 5;

template <typename T> std::string to_string(T value) {
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

//Constructor
DataParser::DataParser(string f, string p, string o, string k, string pr) {
	//Assign values to the variables within the object
	filename = f;
	protocol = p;
	file_out = o;
	sum_file_out = k;
	timeFix = -1;
	totalTime = 0;
	totalUsefulData = 0;
	totalData = 0;
	dataChunks = 0;
	packetCounter = 0;

	//Handle the printflag
	if (pr == "-printall"){
		PRINT_PACKET = true;
		PRINT_ALL = true;
	}
	else if (pr == "-print") {
		PRINT_PACKET = false;
		PRINT_ALL = true;
	}
	else if (pr == "-noprint") {
		PRINT_PACKET = false;
		PRINT_ALL = false;
	}
	else {
		cout << "Argument failure" << "\n";
		exit(1);
	}
}

//Simple graphical user interface function
void DataParser::GUI() {
	cout << GUI_line() << "\n";
	cout << "Data Parser for .pcap files (Version 2.0)" << "\n";
	cout << left << setw(17) << "Input file: " << filename << "\n";
	cout << left << setw(17) << "Output file: " << sum_file_out << "\n";
	cout << left << setw(17) << "Protocol: " << protocol << "\n";
	if (PRINT_PACKET == true) {
		cout << GUI_line() << "\n";
		cout << left << setw(7) << "Frame";
		cout << left << setw(11) << "Sender";
		cout << left << setw(11) << "Receiver";
		cout << left << setw(10) << "Time [s]";
		cout << left << setw(10) << "TTL [ms]";
		cout << left << setw(12) << "Length [b]";
		cout << left << setw(12) << "Data [b]";
		cout << left << setw(10) << "Chunks" << endl;
		cout << GUI_line() << "\n";
	}
}

//Function for printing out a line (for GUI)
string GUI_line() {
	string star_line;
	for (int i = 0; i < 80; i++) {
		star_line += "*";
	}
	return star_line;
}

//Searches for packets within a .pcap file
void DataParser::packetExtractor() {
	int current_frame = 1;
	ifstream reader(filename.c_str());
	ofstream erase(file_out.c_str());
	string line;
	string arr[NUM_OF_WORDS];

	if(reader.is_open()){

		while((getline(reader, line))) {
			string frame = to_string(current_frame) + ":";
			stringstream  str(line);
			int i = 0;
			while (str.good() && i < NUM_OF_WORDS) {
				str >> arr[i];
				i++;
			}
			if(arr[0] == "Frame" && arr[1] == frame) {
				dataExtractor(reader, current_frame);
				current_frame++;
			}
		}
	}
	else {
		cout << filename << " could not be opened" << "\n";
		exit(1);
	}
	cout << GUI_line() << "\n";

	insertTotalData(packetCounter, totalTime, totalData, totalUsefulData, dataChunks);

}

//Extracts internal parameters from packets
void DataParser::dataExtractor(ifstream& reader, int frame) {

	string line;
	string arr[NUM_OF_WORDS];

	bool CORRECT_PROTOCOL = true;
	bool LENGTH_GET, SENDER_GET, RECIEVER_GET, TIME_GET, TTL_GET, HEARTBEAT = false;

	double frame_length, epoch_time, time_to_live;
	int data_length = 0;
	int chunk_length = 0;
	int data_chunk_count = 0;
	string sender, reciever, stream_id;


	while((getline(reader, line))) {
		if(line == "") {
			break;
		}
		if (CORRECT_PROTOCOL == false) {
			return;
		}
		if (HEARTBEAT == true) {
			return;
		}

		int words = 0;
		stringstream  str(line);
		while (str.good() && words < NUM_OF_WORDS) {
			str >> arr[words];
			words++;
		}
		if ((arr[0] + " " + arr[1]) == "HEARTBEAT chunk") {
			HEARTBEAT = getHeartBeat(arr, frame);
		}
		else if ((arr[0] + " " + arr[1]) == "HEARTBEAT_ACK chunk") {
			HEARTBEAT = getHeartBeat(arr, frame);
		}
		else if((arr[0] + " " + arr[1] + " " + arr[2]) == "[Protocols in frame:") {
			CORRECT_PROTOCOL = getPacketProtocol(arr, frame);
		}
		else if ((arr[0] + " " + arr[1]) == "Epoch Time:") {
			epoch_time = getEpochTime(arr);
			TIME_GET = true;
		}
		else if((arr[0] + " " + arr[1]) == "Frame Length:") {
			frame_length = getPacketLength(arr);
			LENGTH_GET = true;
		}
		else if(arr[0] == "Source:") {
			sender = getSender(arr);
			SENDER_GET = true;
		}
		else if(arr[0] == "Destination:") {
			reciever = getReciever(arr);
			RECIEVER_GET = true;
		}
		else if((arr[0] + " " + arr[1] + " " + arr[2]) == "Time to live:") {
			time_to_live = getTTL(arr);
			TTL_GET = true;
		}
		else if((arr[0]) == "Data") {
			getline(reader, line);
			chunk_length = getPayload(arr);
			data_length += chunk_length;
			if (protocol == "sctp") {
				data_chunk_count++;
			}
		}
	}
	if(timeFix == -1){
		timeFix = epoch_time;
	}

	totalTime = (epoch_time - timeFix);
	totalUsefulData += data_length;
	totalData += frame_length;
	dataChunks += data_chunk_count;
	packetCounter += 1;

	if(LENGTH_GET == SENDER_GET == RECIEVER_GET == TIME_GET == TTL_GET == true) {
		insertPacketData(frame, sender, reciever, epoch_time, time_to_live, frame_length, data_length, data_chunk_count);
	}
	else {
		cout << left << setw(7) << frame;
		string errmsg = "Extraction failed";
		cout << left << setw(20) << errmsg;
		cout << "\n";
	}

}

double DataParser::getPacketLength(string arr[]) {
	double length = atof(arr[2].c_str());
	return length;
}

double DataParser::getEpochTime(string arr[]) {
	double time = atof(arr[2].c_str());
	return time;
}

double DataParser::getTTL(string arr[]) {
	double TTL = atof(arr[3].c_str());
	return TTL;
}

string DataParser::getStream(string arr[]) {
	string stream = arr[2];
	return stream;
}

int DataParser::getPayload(string arr[]) {
	string length;
	for(string::size_type i = 0; i< arr[1].size(); i++) {
		if(arr[1][i] != '(') {
			length += arr[1][i];
		}
	}
	int size = atof(length.c_str());
	return size;
}

string DataParser::getSender(string arr[]) {
	string sender = arr[1];
	return sender;
}

string DataParser::getReciever(string arr[]) {
	string reciever = arr[1];
	return reciever;
}

bool DataParser::getHeartBeat(string arr[], int packet) {
	if (PRINT_PACKET == true) {
		cout << left << setw(7) << packet;
		string errmsg = "Heartbeat detected";
		cout << left << setw(20) << errmsg;
		cout << "\n";
	}
	return true;
}

bool DataParser::getPacketProtocol(string arr[], int packet) {
	string frame_protocols = arr[3];
	string next_pr;
	for(string::size_type i = 0; i< frame_protocols.size(); i++) {
		if ((frame_protocols[i] != ':') && (frame_protocols[i] != ']')) {
			next_pr += frame_protocols[i];
		}
		else if (frame_protocols[i] == ':') {
			if (next_pr == protocol) {
				return true;
			}
			next_pr = "";
		}
		else if(frame_protocols[i] == ']') {
			if (next_pr == protocol) {
				return true;
			}
			else {
				if (PRINT_PACKET == true) {
					cout << left << setw(7) << packet;
					string errmsg = "NO " + protocol + " detected";
					cout << left << setw(20) << errmsg;
					cout << "\n";
				}
				return false;
			}
		}
	}
	return false;
}

void DataParser::insertPacketData(int frame, string sender, string reciever, double epoch_time, double time_to_live,
							int frame_length, int data_length, int data_chunk_count) {
	if (PRINT_PACKET == true) {
		cout << left << setw(7) << frame;
		cout << left << setw(11) << sender;
		cout << left << setw(11) << reciever;
		cout << left << setw(10) << epoch_time;
		cout << left << setw(10) << time_to_live;
		cout << left << setw(12) << frame_length;
		cout << left << setw(12) << data_length;
		cout << left << setw(10) << data_chunk_count;
		cout << "\n";
	}

	/*Writing to a dedicated file for the simulation (will most certainly be removed later)
	 * Current file format (by columns)
	 * [1] Frame number
	 * [2] Epoch time (adjusted from 0)
	 * [3] TTL
	 * [4] Frame length (with headers etc.)
	 * [5] Data amount
	 * [6] Number of chunks
	 * [7] Sender IP
	 * [8] Receiver IP
	 */
/*
	ofstream myfile;
	myfile.open(file_out.c_str(), ios::app);
	if(myfile.is_open()){
		myfile << frame << " " << (epoch_time - timeFix) << " " << time_to_live << " " << frame_length << " "
			   << data_length << " " << data_chunk_count << " " << sender << " " << reciever << "\n";
	}
	myfile.close();
	*/
}

void DataParser::insertTotalData(int packetCounter, double totalTime, double totalData,
								 double totalUsefulData, int dataChunks) {
	float dataPercentage = totalUsefulData / totalData * 100;
	double speed = totalData / totalTime / 1024;
	double dataChunkAvg = totalUsefulData / dataChunks;
	double frameSizeAvg = totalData/packetCounter;
	/*Write a line in a file for all simulations
	 * Current file format (by columns)
	 * [1] Number of frames
	 * [2] Total time
	 * [3] Data sent (with headers etc.)
	 * [4] Data sent (just useful data)
	 * [5] Percentage of useful data
	 * [6] Speed (in Mbytes/sec)
	 * [7] Average frame size
	 * [8] Number of data chunks
	 * [9] Average size of data chunks (header not included)
	 */
	ofstream myfile;
	myfile.open(sum_file_out.c_str(), ios::app);
	if(myfile.is_open()){
		myfile << packetCounter << " " << totalTime << " "  << totalData << " "
			  << totalUsefulData << " " << dataPercentage << " " << speed << " "
			  << frameSizeAvg << " " << dataChunks << " " << dataChunkAvg << "\n";
	}
	myfile.close();

	if (PRINT_ALL == true) {
		cout << "Extraction summary for " << filename << " (" << protocol << " protocol)" <<  "\n";
		cout << left << setw(28) << "Number of frames: " << packetCounter << " frames" << "\n";
		cout << left << setw(28) << "Total transmission time: " << totalTime << " s" << "\n";
		cout << left << setw(28) << "Data sent (with headers): " << totalData << " bytes" << "\n";
		cout << left << setw(28) << "Data sent (no headers): " << totalUsefulData << " bytes" << "\n";
		cout << left << setw(28) << "Percentage of data: " <<  dataPercentage << " %" << "\n";
		cout << left << setw(28) << "Transmission speed: " << speed << " Mbytes/s" << "\n";
		cout << left << setw(28) << "Average frame size: " << frameSizeAvg << " bytes" << "\n";
		cout << left << setw(28) << "Data chunk count: " << dataChunks << " chunks" << "\n";
		cout << left << setw(28) << "Averaga data per chunk: " << dataChunkAvg << " bytes" << "\n";
		cout << GUI_line() << "\n";
	}
}

void ARG_ERROR_MSG() {
	cout << GUI_line() << "\n";
	cout << "Program argument failure" << endl;
	cout << "The format of argument input looks as follows:" << endl;
	cout << "./DataParser protocol .pcap-file > .dat-file -printflag" << endl;
	cout << GUI_line() << "\n";
	exit(1);
}

void HELP_MSG() {
	cout << GUI_line() << "\n";
	cout << "This is a C++ program for parsing of .pcap Wireshark files" << endl;
	cout << "Current version of software is 2.0" << endl;
	cout << "The format of argument input looks as follows:" << "\n" << endl;
	cout << "./DataParser protocol .pcap-file to .dat-file -printflag" << "\n" << endl;
	cout << "Printing flags:" << endl;
	cout << left << setw(15) << "-noprint" << "prints no information" << endl;
	cout << left << setw(15) << "-print" << "prints only the information about the simulation" << endl;
	cout << left << setw(15) << "-printall" << "prints all the information (including info about packets)" << "\n" << endl;
	cout << "Supported protocols:" << endl;
	cout << left << setw(15) << "sctp" << "Stream Control Transmission Protocol" << "\n";
	cout << GUI_line() << "\n";
	exit(1);
}

void start_data_parser(string protocol, int data_bytes_per_node, string sourceFile, string targetFile, string print) {

	string convert = "tshark -V -r " + sourceFile + ".pcap > " + sourceFile + ".txt";
	system(convert.c_str());

	string input_file = sourceFile;
	string output_file = sourceFile + "-parse.txt";
	string sum_output_file = targetFile;
	sum_output_file += ".dat";
	input_file += ".txt";
	string prot = protocol;

	DataParser parser(input_file, prot, output_file, sum_output_file, print);
	parser.GUI();
	parser.packetExtractor();
}
