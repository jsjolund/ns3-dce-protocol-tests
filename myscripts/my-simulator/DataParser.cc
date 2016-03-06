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
DataParser::DataParser(string sourceFile, string protocol1, string targetFile, string summaryTargetFile, string print, int expectedData1, int numClients1, int numberOfStreamsPerClient1) {
	
	//Assign values to the variables within the object
	filename = sourceFile;
	protocol = protocol1;
	file_out = targetFile;
	sum_file_out = summaryTargetFile;

	timeFix = -1;
	totalTime = 0;
	totalUsefulData = 0;
	totalData = 0;
	dataChunks = 0;
	packetCounter = 0;

	expectedData = expectedData1;
	numberOfStreamsPerClient = numberOfStreamsPerClient1;
	numClients = numClients1;

	//Handle the printflag
	if (print == "-printall"){
		PRINT_PACKET = true;
		PRINT_ALL = true;
	}
	else if (print == "-print") {
		PRINT_PACKET = false;
		PRINT_ALL = true;
	}
	else if (print == "-noprint") {
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
	cout << left << setw(28) << "Input file: " << filename << "\n";
	cout << left << setw(28) << "Output file: " << sum_file_out << "\n";
	cout << left << setw(28) << "Protocol: " << protocol << "\n";
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
	remove(file_out.c_str());
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
	}else {
		cout << filename << " could not be opened" << "\n";
		exit(1);
	}
	cout << GUI_line() << "\n";

	insertTotalData();

}

//Extracts internal parameters from packets
void DataParser::dataExtractor(ifstream& reader, int frame) {
	string line;
	string arr[NUM_OF_WORDS];

	bool CORRECT_PROTOCOL = true;
	bool TTL_GET, HEARTBEAT = false;

	double frame_length, epoch_time, time_to_live;
	int data_length = 0, data_chunk_count = 0;
	string sender, reciever;


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
		}
		else if((arr[0] + " " + arr[1]) == "Frame Length:") {
			frame_length = getPacketLength(arr);
		}
		else if(arr[0] == "Source:") {
			sender = getSender(arr);
		}
		else if(arr[0] == "Destination:") {
			reciever = getReciever(arr);
		}
		else if(((arr[0] + " " + arr[1] + " " + arr[2]) == "Time to live:")) {
			time_to_live = getTTL(arr);
			TTL_GET = true;
		}
		else if((arr[0]) == "Data") {
			getline(reader, line);
			data_length += getPayload(arr);
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


	if (PRINT_PACKET == true) {
		cout << left << setw(7) << frame;
		cout << left << setw(11) << sender;
		cout << left << setw(11) << reciever;
		cout << left << setw(10) << epoch_time;
		
		if(protocol == "sctp"){
			cout << left << setw(10) << time_to_live;
		}else{
			cout << left << setw(10) << "NaN";
		}

		cout << left << setw(12) << frame_length;
		cout << left << setw(12) << data_length;

		if(protocol == "sctp"){
			cout << left << setw(10) << data_chunk_count;
		}else{
			cout << left << setw(10) << "NaN";
		}
		
		cout << "\n";
	}


}

//Gets wanted data and formats it accordingly.

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
		}else if (frame_protocols[i] == ':') {
			if (next_pr == protocol) {
				return true;
			}
			next_pr = "";
		}else if(frame_protocols[i] == ']') {
			if (next_pr == protocol) {
				return true;
			}
		}else {
			if (PRINT_PACKET == true) {
				cout << left << setw(7) << packet;
				string errmsg = "No " + protocol + " detected";
				cout << left << setw(20) << errmsg;
				cout << "\n";
			}
			return false;
		}
	}
	return false;
}

//Inserts information in file. Will insert the total of all packets.
	/*Write a line in a file for all simulations
	 * Current file format (by columns)
	 * [1] Number of frames
	 * [2] Total simulated time
	 * [3] Data sent (with headers etc.)
	 * [4] Data sent (just useful data)
	 * [5] Expected amount of data
	 * [6] Percentage of useful data
	 * [7] Percentage of data loss
	 * [8] Transmission Speed (in Mbytes/sec)
	 * [9] Average frame size
	 * [10] Number of data chunks
	 * [11] Average size of data chunks (header not included)
	 * [12] Number of clients
	 * [13] Streams to server per client
	 */

void DataParser::insertTotalData() {
	float dataPercentage = totalUsefulData / totalData * 100;
	double speed = totalData / totalTime / (1024*1024);
	double dataChunkAvg = totalUsefulData / dataChunks;
	double frameSizeAvg = totalData/packetCounter;
	double dataLossPercent = 100 - (totalUsefulData / expectedData * 100);
	expectedData = expectedData / (1024*1024);
	totalData = totalData / (1024*1024);
	totalUsefulData = totalUsefulData / (1024*1024);

	ofstream myfile;
	myfile.open(sum_file_out.c_str(), ios::app);
	if(myfile.is_open()){
		if(protocol == "sctp"){
			myfile << packetCounter << " " << totalTime << " "  << totalData << " "
				  << totalUsefulData << " " << expectedData << " " << dataPercentage << " " << dataLossPercent << " " << speed << " "
				  << frameSizeAvg << " " << dataChunks << " " << dataChunkAvg << " " << numClients << " " << numberOfStreamsPerClient << "\n";
		}else{
			myfile << packetCounter << " " << totalTime << " "  << totalData << " "
				  << totalUsefulData << " " << expectedData << " " << dataPercentage << " " << dataLossPercent << " " << speed << " "
				  << frameSizeAvg << " " << "NaN" << " " << "NaN" << " " << numClients << " " << numberOfStreamsPerClient << "\n";
		}
	}
	myfile.close();

	if (PRINT_ALL == true) {
		cout << "Extraction summary for " << filename << " (" << protocol << " protocol)" <<  "\n";
		cout << left << setw(28) << "Number of frames: " << packetCounter << " frames" << "\n";
		cout << left << setw(28) << "Total transmission time: " << totalTime << " s" << "\n";
		cout << left << setw(28) << "Data sent (with headers): " << totalData << " MB" << "\n";
		cout << left << setw(28) << "Data sent (no headers): " << totalUsefulData << " MB" << "\n";
		cout << left << setw(28) << "Expected amount of data: " << expectedData << " MB" << "\n";
		cout << left << setw(28) << "Percent data in packet: " <<  dataPercentage << " %" << "\n";
		cout << left << setw(28) << "Percent data loss: " <<  dataLossPercent << " %" << "\n";
		cout << left << setw(28) << "Transmission speed: " << speed << " MB/s" << "\n";
		cout << left << setw(28) << "Average frame size: " << frameSizeAvg << " bytes" << "\n";
		
		if(protocol == "sctp"){
			cout << left << setw(28) << "Data chunk count: " << dataChunks << " chunks" << "\n";
			cout << left << setw(28) << "Average data per chunk: " << dataChunkAvg << " bytes" << "\n";
		}else{
			cout << left << setw(28) << "Data chunk count: " << "NaN" << "\n";
			cout << left << setw(28) << "Average data per chunk: " << "NaN" << "\n";
		}
		cout << left << setw(28) << "Number of clients: " << numClients << " clients" << "\n";
		cout << left << setw(28) << "Streams/sockets per client: " << numberOfStreamsPerClient << " streams" << "\n";
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
	cout << "Printing flags:" << endl;
	cout << left << setw(15) << "-noprint" << "prints no information" << endl;
	cout << left << setw(15) << "-print" << "prints only the information about the simulation" << endl;
	cout << left << setw(15) << "-printall" << "prints all the information (including info about packets)" << "\n" << endl;
	cout << "Supported protocols:" << endl;
	cout << left << setw(15) << "sctp" << "Stream Control Transmission Protocol" << "\n";
	cout << left << setw(15) << "tcp" << "Transmission Control Protocol" << "\n";
	cout << left << setw(15) << "udp" << "User Datagram Protocol" << "\n";
	cout << GUI_line() << "\n";
	exit(1);
}

void start_data_parser(string protocol, int numClients, int dataBytesPerClient, int numberOfStreamsPerClient, string sourceFile, string summaryTargetFile, string print) {
	string convert = "tshark -V -r " + sourceFile + ".pcap > " + sourceFile + ".txt";
	system(convert.c_str());

	string targetFile = sourceFile + "-parse.txt";
	summaryTargetFile += ".dat";
	sourceFile += ".txt";
	int expectedData = numClients * dataBytesPerClient;

	DataParser parser(sourceFile, protocol, targetFile, summaryTargetFile, print, expectedData, numClients, numberOfStreamsPerClient);
	parser.GUI();
	parser.packetExtractor();
}



