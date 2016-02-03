#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;




int main(int argc, char* argv[]) {
	int i = 2;
	string summaryFileName, sourceFile, targetFile, sourceFile1;
	int errorCheck;
	
	if(argc < 3) {
		cout << "Please add a parameter to the program" << endl;
		exit(1);
	}
	

	summaryFileName = argv[1];
	summaryFileName += ".dat";
	ofstream erase(summaryFileName) ;
	targetFile = argv[1];

	while(i < argc){
		sourceFile = argv[i];
		sourceFile1 = sourceFile + ".txt";

		errorCheck = setenv("targetFile", argv[1], true);
		if(errorCheck == -1){
			cout<<"Variable error in program";
			exit(1);
		}

		errorCheck = setenv("sourceFile", argv[i], true);
		if(errorCheck == -1){
			cout<<"Variable error in program";
			exit(1);
		}
		
		string testVar = "./DataParser sctp " + sourceFile + " to " + targetFile + " -printall";
		system(testVar.c_str());
		//system("./makefile.sh");
		remove(sourceFile1.c_str());
		i += 1;
	}
	


	
}
