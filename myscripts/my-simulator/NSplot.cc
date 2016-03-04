#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

//Define the error message function
void DIM_ERR_MSG();
void FNF_ERR_MSG();
void HLP_MSG();
void ARG_ERR_MSG();


//Define the parameters and units
string parameters[13][2] = {{"Frames", ""}, {"Transmission time", "[s]"}, {"Data with headers", "[MB]"},
					   {"Data no headers", "[MB]"}, {"Data expected", "[MB]"}, {"Data percentage", "[%]"}, 
					   {"Data loss", "[%]"}, {"Transmission speed", "[Mbytes/s]"}, {"Avg. frame size", "[bytes]"}, 
					   {"Data chunks", ""}, {"Avg chunk size", "[bytes]"}, {"Clients", ""}, {"Streams/sockets per client", ""}};

//Declaration of the class
class NSplot {
	//Variables
	private : string colA, colB, colC;
	private : string labelA, labelB, labelC;
	private : string action;
	private : string output;
	private : int dimensions;
	//Constructor
	public : NSplot(string, string);
	//Mehods
	public : void selectColumns();
	public : string setLabel(string);
	public : void setGnuplotParams();
	public : void setAction(string);
	public : void checkFile(string);
};

//Constructor
NSplot::NSplot(string out, string dim) {
	//Set internal parameters
	colA = colB = colC = -1;
	action = "plot ";
	output = out;

	//Set the dimension
	if(dim == "-2d") {
		dimensions = 2;
	}
	else if (dim == "-3d") {
		dimensions = 3;
	}
	else {
		DIM_ERR_MSG();
	}
	selectColumns();
}

//GUI function for printing a line
string GUI_line() {
	string star_line;
	for (int i = 0; i < 80; i++) {
		star_line += "*";
	}
	return star_line;
}

//Column selecting method (based on user input)
void NSplot::selectColumns() {
	cout << "NS-PLOT (v3.0)" << endl;
	cout << GUI_line() << endl;
	cout << left << setw(30) << "Available parameters:" << endl;
	cout << left << setw(30) << "[1] Frames";
	cout << left << setw(30) << "[2] Transmission time";
	cout << left << setw(30) << "[3] Data with headers" << endl;
	cout << left << setw(30) << "[4] Data no headers";
	cout << left << setw(30) << "[5] Data expected";
	cout << left << setw(30) << "[6] Data percentage" << endl;
	cout << left << setw(30) << "[7] Data loss percentage";
	cout << left << setw(30) << "[8] Transmission speed";
	cout << left << setw(30) << "[9] Avg. frame size" << endl;
	cout << left << setw(30) << "[10] Data chunks";
	cout << left << setw(30) << "[11] Avg. chunk size";
	cout << left << setw(30) << "[12] Clients" << endl;
	cout << left << setw(30) << "[13] Streams/sockets per client" << endl;
	cout << GUI_line() << endl;
	cout << "Insert the first column of data that will be the x-axis : ";
	getline(cin, colA);
	labelA = setLabel(colA);
	cout << "[" << parameters[atoi(colA.c_str())-1][0] << "] is selected for x-axis" << endl;
	cout << "Insert the second column of data that will be the y-axis : ";
	getline(cin, colB);
	labelB = setLabel(colB);
	cout << "[" << parameters[atoi(colB.c_str())-1][0] << "] is selected for y-axis" << endl;
	if(dimensions == 3) {
		cout << "Insert the second column of data that will be the z-axis : ";
		getline(cin, colC);
		labelC = setLabel(colC);
		cout << "[" << parameters[atoi(colC.c_str())-1][0] << "] is selected for y-axis" << endl;
	}
	cout << GUI_line() << endl;
	cout << "Files for processing: " << endl;

}

//Method for setting X-Y labels in the graph
string NSplot::setLabel(string column) {
	return parameters[atoi(column.c_str())-1][0] + " " + parameters[atoi(column.c_str())-1][1];
}

//Method for file checking (checking if the file can be opened)
void NSplot::checkFile(string file) {
	ifstream reader(file);
	if(!reader.is_open()) {
		cout << ".......not valid" << endl;
		FNF_ERR_MSG();
	}
	else {
		cout << "..........valid" << endl;
	}
}


//Function for setting Gnuplot parameters (via bash variables)
void NSplot::setGnuplotParams() {
	cout << GUI_line() << endl;
	cout << "Setting Gnuplot parameters..." << endl;
	cout << "Number of dimensions: " << dimensions << endl;
	cout << "Output PNG file: " << output << endl;
	cout << GUI_line() << endl;

	//Set the labels for the graph
	setenv("LabelX", labelA.c_str(), true);
	setenv("LabelY", labelB.c_str(), true);
	if(dimensions == 3) {
		setenv("LabelZ", labelC.c_str(), true);
	}
	//Set what shall be displayed on the graph
	setenv("action", action.c_str(), true);
	//Set the output file
	setenv("output", output.c_str(), true);

}

//Method for setting the columns and source files which will be passed to gnuplot
void NSplot::setAction(string nextFile) {
	cout << nextFile;
	checkFile(nextFile);
	if(dimensions == 2) {
		string addition = " '" + nextFile + "' using " + colA + ":" + colB + " title '" + nextFile + "',";
		action += addition;
	}
	else if (dimensions == 3) {
		action = "splot '" + nextFile + "' using " + colA + ":" + colB + ":" + colC + " title '" + nextFile + "'";
	}
	else {
		DIM_ERR_MSG();
	}
}

//Dimension error message
void DIM_ERR_MSG() {
	cout << GUI_line() << endl;
	cout << "Dimension error, the supported dimensions are -2d and -3d" << endl;
	cout << "Only one graph at a time is accepted for a 3D plot" << endl;
	cout << GUI_line() << endl;
	exit(1);
}

//Argument error message
void ARG_ERR_MSG() {
	cout << GUI_line() << endl;
	cout << "Argument error, the correct argument format is:" << endl;
	cout << "./NSplot output-file data-file-1 data-file-2 ... dimension-flag" << endl;
	cout << GUI_line() << endl;
	exit(1);
}

//File not found message
void FNF_ERR_MSG() {
	cout << GUI_line() << endl;
	cout << "File not found! Check your filename argument!"<< endl;
	cout << GUI_line() << endl;
	exit(1);
}

//Help message
void HLP_MSG() {
	cout << GUI_line() << endl;
	cout << "This is a C++ program for viewing graphs from NS-3 simulations"<< endl;
	cout << "The program supports both 2D (by adding -2d flag in the end) and 3D graphs (by adding -3d flag in the end)" << endl;
	cout << "Multiple data lines can be compared in 2D mode by passing them from the bash as arguments" << endl << endl;
	cout << "The format of arguments looks as follows:" << endl;
	cout << "./NSplot output-file data-file-1 data-file-2 ... dimension-flag" << endl;
	cout << GUI_line() << endl;
	exit(1);
}

#define PLOT2D "gnuplot <<- EOF\n set term png\n set output '$output'\n set xlabel '$LabelX'\n set ylabel '$LabelY'\n set grid\n set key above\n $action\nEOF\ndisplay $output\n"
#define PLOT3D "gnuplot <<- EOF\n set term png\n set output '$output'\n set xlabel '$LabelX'\n set ylabel '$LabelY'\n set zlabel '$LabelZ' rotate parallel\n set grid\n set key above\n $action\nEOF\ndisplay $output\n"

//Main program
int main(int argc, char* argv[]) {
	//Some argument handling
	string HELP_FLAG = argv[1];
	if(HELP_FLAG == "-help") {
		HLP_MSG();
	}
	if(argc == 1) {
		ARG_ERR_MSG();
	}

	string dimension = argv[argc-1];

	NSplot graph(argv[1], dimension);
	for (int var = 2; var < argc-1; var++) {
		graph.setAction(argv[var]);
	}
	graph.setGnuplotParams();

	//Starting bash scripts, depending on dimension
	if(dimension == "-2d") {
		system(PLOT2D);
	}
	else if (dimension == "-3d") {
		if (argc != 4) {
			DIM_ERR_MSG();
		}
		system(PLOT3D);
	}
	else {
		DIM_ERR_MSG();
	}
}
