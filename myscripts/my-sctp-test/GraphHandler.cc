#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include "GraphHandler.h"
#include "DataParser.h"


using namespace std;

void start_graph_handler(string target, string source) {
	string summaryFileName, sourceFile, targetFile, sourceFile1;

	

	summaryFileName = target;
	summaryFileName += ".dat";
	ofstream erase(summaryFileName.c_str());
	targetFile = target;

	sourceFile = source;
	sourceFile1 = sourceFile + ".txt";
	
	start_data_parser("sctp", sourceFile, targetFile, "-print");
	remove(sourceFile1.c_str());
}
