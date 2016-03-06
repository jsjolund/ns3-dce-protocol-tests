#ifndef _DATAPARSER_H
#define _DATAPARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;


//Global methods
string GUI_line();
void ARG_ERROR_MSG();
void HELP_MSG();

//Declaration of the class
class DataParser {
	//Variables for file handling and printing
	private : string filename;
	private : string protocol;
	private : string file_out;
	private : string sum_file_out;
	private : bool PRINT_PACKET;
	private : bool PRINT_ALL;
	//Variables for extracted parameters
	private : double timeFix;
	private : double totalUsefulData;
	private : double totalData;
	private : int dataChunks;
	private : double totalTime;
	private : int numberOfStreamsPerClient;
	private : int packetCounter;
	private : double expectedData;
	private : int numClients;
	//Constructor
	public : DataParser(string, string, string, string, string, int, int, int);
	//Functions
	public : void GUI();
	public : void packetExtractor();
	public : void dataExtractor(ifstream&, int);
	//Private data extracting functions
	private : double getPacketLength(string[]);
	private : double getEpochTime(string[]);
	private : double getTTL(string[]);
	private : bool getPacketProtocol(string[], int);
	private : bool getHeartBeat(string[], int);
	private : string getSender(string[]);
	private : string getReciever(string[]);
	private : string getStream(string[]);
	private : int getPayload(string[]);
	private : void insertTotalData();
};

string GUI_line();
void ARG_ERROR_MSG();
void HELP_MSG();
void start_data_parser(string protocol, int numClients, int dataBytesPerClient, int numberOfStreamsPerClient, string sourceFile, string targetFile, string print);

#endif /* _DATAPARSER_H */


