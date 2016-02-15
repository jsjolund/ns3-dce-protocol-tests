#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

//Declaration of the class
class GraphCreator {
	//Variables
	private : string filename;
	private : int height, width;
	private : int colA, colB;
	private : string labelA, labelB;
	private : double maxX, maxY;
	private : double minX, minY;
	private : string paramX, paramY;
	//Constructor
	public : GraphCreator(string);
	//Mehods
	public : const char* getLabelX();
	public : const char* getLabelY();
	public : void selectColumns();
	public : void selectData();
	public : int computeHeight();
	public : int computeWidth();
	public : int setSelectionVars(string, int);
	public : void setGnuplotParams();
	public : string GUI_line();
};

//Constructor
GraphCreator::GraphCreator(string s) {
	maxX = maxY = 0;
	minX = minY = 1000;
	colA = colB = -1;
	filename = s;
	height = computeHeight();
	width = computeWidth();
	selectColumns();
	colA = setSelectionVars(labelA, 1);
	colB = setSelectionVars(labelB, 2);
}

string GraphCreator::GUI_line() {
	string star_line;
	for (int i = 0; i < 80; i++) {
		star_line += "*";
	}
	return star_line;
}

//Column selecting method (based on user input)
void GraphCreator::selectColumns() {
	cout << "NS-PLOT (v2.0)" << "\n";
	cout << "Source file: " << filename << "\n";
	cout << GUI_line() << "\n";
	cout << left << setw(20) << "Available parameters:" << "\n";
	cout << left << setw(20) << "[1] Frame number";
	cout << left << setw(20) << "[2] Time";
	cout << left << setw(20) << "[3] Data with headers" << "\n";
	cout << left << setw(20) << "[4] Data no headers";
	cout << left << setw(20) << "[5] Data percentage";
	cout << left << setw(20) << "[6] Transmission speed" << "\n";
	cout << left << setw(20) << "[7] Frame size";
	cout << left << setw(20) << "[8] Data chunks";
	cout << left << setw(20) << "[9] Chunk size" << "\n";
	cout << GUI_line() << "\n";
	cout << "Insert the first column of data that will be the x-axis : ";
	getline(cin, labelA);
	cout << "Insert the second column of data that will be the y-axis : ";
	getline(cin, labelB);
}

/*Method for setting ColA and ColB variables
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

int GraphCreator::setSelectionVars(string label, int labelID) {
	int col;
	if(label == "Frame number" || label == "1") {
		if(labelID == 1) {
			labelA = "Number of frames";
		}
		else {
			labelB = "Number of frames";
		}
		col = 1;
	}
	else if(label == "Time" || label == "2") {
		if(labelID == 1) {
			labelA = "Time [s]";
		}
		else {
			labelB = "Time [s]";
		}
		col = 2;
	}
	else if(label == "Data with headers" || label == "3") {
		if(labelID == 1) {
			labelA = "Data with headers [bytes]";
		}
		else {
			labelB = "Data with headers [bytes]";
		}
		col = 3;
	}
	else if(label == "Data no headers" || label == "4") {
		if(labelID == 1) {
			labelA = "Data no headers [bytes]";
		}
		else {
			labelB = "Data no headers [bytes]";
		}
		col = 4;
	}
	else if(label == "Data percentage" || label == "5") {
		if(labelID == 1) {
			labelA = "Data percentage";
		}
		else {
			labelB = "Data percentage";
		}
		col = 5;
	}
	else if(label == "Transmission speed" || label == "6") {
		if(labelID == 1) {
			labelA = "Transmission speed [Mbytes/s]";
		}
		else {
			labelB = "Transmission speed [Mbytes/s]";
		}
		col = 6;
	}
	else if(label == "Frame size" || label == "7") {
		if(labelID == 1) {
			labelA = "Frame size [bytes]";
		}
		else {
			labelB = "Frame size [bytes]";
		}
		col = 7;
	}
	else if(label == "Data chunks" || label == "8") {
		if(labelID == 1) {
			labelA = "Data chunks";
		}
		else {
			labelB = "Data chunks";
		}
		col = 8;
	}
	else if (label == "Chunk size" || label == "9") {
		if(labelID == 1) {
			labelA = "Chunk size [bytes]";
		}
		else {
			labelB = "Chunk size [bytes]";
		}
		col = 9;
	}
	else {
		cout << "ERROR" << "\n";
		exit(1);
	}
	return col;
}


//Method for computation of number of rows in the data (used for the creation of temporary array)
int GraphCreator::computeHeight() {
	ifstream heightComp(filename);
	string line;
	int numberOfRows = 0;

	if(heightComp.is_open()) {
		while(getline(heightComp, line)) {
			numberOfRows++;
		}
		heightComp.close();
	}
	else {
		cout << "File could not be opened" << "\n";
		exit(1);
	}
	return numberOfRows;
}

//Method for computation of number of columns in the data (used for creation of temporary array)
int GraphCreator::computeWidth() {
	ifstream widthComp(filename);
	string line;
	int numberOfColumns = 1;

	if(widthComp.is_open()) {
		getline(widthComp, line);
		for(string::size_type i = 0; i < line.size(); i++) {
			if(line[i] == ' ') {
				numberOfColumns++;
			}
		}
	}
	else {
		cout << "File could not be opened" << "\n";
		exit(1);

	}
	return numberOfColumns;
}

//Method for the selection of data and writing it into a text file
void GraphCreator::selectData() {

	//Initialize temporary variables
	int paramCount, lineCount;
	string line;
	string dataA;
	string dataB;
	double valueA;
	double valueB;

	//Create file reading object
	ifstream readfile(filename);

	//Create temporary array
	string data[height][2];

	//Check if file is open
	if(readfile.is_open()) {
		cout << "Source file is open, extracting column data..." << "\n";
		cout << GUI_line() << "\n";
		cout << left << setw(18) << labelA;
		cout << left << setw(18) << labelB << "\n";
		cout << GUI_line() << "\n";
		lineCount = 0;
		//Read file line for line
		while(getline(readfile, line)) {
			paramCount = 1;
			//Go through each character in a line
			for(string::size_type i = 0; i < line.size(); i++) {
				//Check if the value is desired
				if(paramCount == colA) {
					if(line[i] != ' ') {
						dataA += line[i];
					}
				}
				if(paramCount == colB) {
					if(line[i] != ' ') {
						dataB += line[i];
					}
				}
				//Look for a space to determine the end of a parameter
				if(line[i] == ' ') {
					paramCount++;
				}
			}
			//Console output (for error checking)
			cout << left << setw(20) << dataA;
			cout << left << setw(20) << dataB << "\n";
			//Insert values into the temporary array
			data[lineCount][0] = dataA;
			data[lineCount][1] = dataB;
			//Update max and min values
			valueA = atof(dataA.c_str());
			valueB = atof(dataB.c_str());
			if(valueA > maxX) {
				maxX = valueA;
			}
			if(valueB > maxY) {
				maxY = valueB;
			}
			if(valueA < minX) {
				minX = valueA;
			}
			if(valueB < minY) {
				minY = valueB;
			}
			//Reset strings etc.
			lineCount++;
			dataA = "";
			dataB = "";
		}
		readfile.close();
	}
	else {
		cout << "File could not be opened";
	}
	cout << GUI_line() << "\n";

	//Open file for writing
	ofstream writefile("graph_data.dat");
	//Check if file is open0
	if(writefile.is_open()) {
		//Write to file
		writefile << "# X Y \n";
		for(int i = 0; i < height; i++) {
			for(int j = 0; j < 2; j++) {
				writefile << data[i][j];
				if ((2 - j) != 1) {
					writefile << ' ';
				}
			}
			writefile << "\n";
		}
		writefile.close();
		cout << "Source file is closed, data is selected successfully!" << "\n";
	}
	else {
		cout << "Printing error, file not found";
	}
}

//A get-function for x-label
const char* GraphCreator::getLabelX() {
	return labelA.c_str();
}
//A get-function for y-label
const char* GraphCreator::getLabelY() {
	return labelB.c_str();
}

//Function for setting Gnuplot parameters
void GraphCreator::setGnuplotParams() {
	cout << "Setting Gnuplot parameters..." << "\n";
	
	//Set the boundaries for the graph
	setenv("maxX", std::to_string(maxX*1.10).c_str(), true);
	setenv("minX", std::to_string(0).c_str(), true);
	setenv("maxY", std::to_string(maxY*1.10).c_str(), true);
	setenv("minY", std::to_string(0).c_str(), true);
	//Set the labels for the graph
	setenv("LabelX", getLabelX(), true);
	setenv("LabelY", getLabelY(), true);
}


//Main program (for testing)
int main(int argc, char* argv[]) {
	string file = argv[1];
	//Create the object
	GraphCreator graph(file);
	graph.selectData();
	graph.setGnuplotParams();
	//Execute Bash script for drawing the graph
	cout << "Creating plot..." << "\n";
	system("./plot.sh");
	cout << "Exiting NS-PLOT..." << "\n" << "\n";
}



