#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>

using namespace std;

vector<string>coutStream;

void clearString(string &str) {
	for (int i = 0; i < str.size(); i++) {
		if (!(str[i] >= 32 && str[i] <= 126)) {
			str.erase(i--);
		}
		if(str[i]=='\n' || str[i] == '\r')
			str.erase(i--);
	}
}
void loadCoutStream(string input) {
	clearString(input);
	coutStream.push_back(input);
}
void printCoutStream() {
	for (int i = 0; i < coutStream.size(); i++) {
		fflush(stdin);
		fflush(stdout);
		printf("%s \n", coutStream[i].c_str());
	}

	coutStream.clear();
}
void readFile(string file, vector<string> &fileData) {
	std::ifstream ifs(file, std::ios::in);
	if (!ifs.is_open()) {
		loadCoutStream("Failed to open file :" + file);
		return;
	}

	string saveStr = "";
	while (std::getline(ifs, saveStr)) {
		fileData.push_back(saveStr);
	}
	ifs.close();
}
void fixFile(string target, string name, vector<string> &fileData) {
	for (int i = 0; i < fileData.size(); i++)
		if (fileData[i].find(target) != string::npos)
			fileData[i] = target + "   " + name;
}
string addMark(string name) {
	return "\"" + name + "\"";
}
void assignFile(string file, vector<string> &fileData) {
	ofstream myfile;
	myfile.open(file);
	for (int i = 0; i < fileData.size(); i++)
		myfile << fileData[i] << endl;
	myfile.close();
}

void createDir(string dir) {
	string s = "mkdir " + dir;
	system(s.c_str());
	s = "rm -r -f " + dir + "/*";
	system(s.c_str());
}
void fixLVSFile(string LVSrul, string RUNSET, string GDS, string CDL, string cell) {
	vector<string> fileData;
	readFile(RUNSET, fileData);
	string markGDS = addMark(GDS);
	fixFile("*lvsRulesFile:", LVSrul, fileData);
	fixFile("*lvsLayoutPaths:", markGDS, fileData);
	fixFile("*lvsLayoutPrimary:", cell, fileData);
	fixFile("*lvsSourcePath:", CDL, fileData);
	fixFile("*lvsSourcePrimary:", cell, fileData);
	fixFile("*lvsReportFile:", "LVS_report", fileData);
	assignFile(RUNSET, fileData);
}
void runLVS(string RUNSET) {
	system("rm LVS_report");
	string s = "calibre -gui -lvs -runset " + RUNSET + " -batch";
	system(s.c_str());
}
void printErrorMessage(string cellName) {
	vector<string> fileData;
	readFile("LVS_report", fileData);
	for (int i = 1; i < fileData.size(); i++) {
		if (fileData[i].find("#     INCORRECT     #") != string::npos) {
			loadCoutStream(cellName + " error : LVS INCORRECT");
			return;
		}
	}
}
void fixStarCmd(string cell) {
	vector<string> fileData;
	readFile("star_cmd", fileData);
	fixFile("BLOCK:", cell,fileData);
	string s = cell + ".spf";
	fixFile("NETLIST_FILE:", s, fileData);
	assignFile("star_cmd", fileData);
}
void runRC(string cell) {
	system("source ./cciFlow.sh ");
	string s = "mv " + cell + ".spf RC/";
	system(s.c_str());
}
void deleteExtraFile(string cell) {

}
void autoLVSandRC(string LVSrul, string RUNSET, string GDS, string CDL, vector<string> &cellList) {
	system("rm -r -f svdb/*");
	createDir("RC");
	for (int i = 0; i < cellList.size(); i++) {
		string cell = cellList[i];
		fixLVSFile(LVSrul, RUNSET, GDS, CDL, cell);
		runLVS(RUNSET);
		printErrorMessage(cell);
		fixStarCmd(cell);
		runRC(cell);
		deleteExtraFile(cell);
	}
}

void readCellList(vector<string> &cellList) {
	vector<string> fileData;
	readFile("cellList", fileData);
	for (int i = 0; i < fileData.size(); i++) {
		clearString(fileData[i]);
		cellList.push_back(fileData[i]);
	}
		
}
int main(int argc, char *argv[]) {
	if (argc < 4) {
		cout << "please add ARGV of LVS.rul , CDL , RUNSET , GDS" << endl;
		return -1;
	}
	string LVSrul = argv[1];
	string RUNSET = argv[2];
	string GDS = argv[3];
	string CDL = argv[4];

	/*
	//test in VS
	string LVSrul = "finFlex_LVS.rul";
	string CDL = "finFlex.cdl";
	string RUNSET = "finFlex_runset";
	string GDS = "AO_fin21.gds";
	*/

	vector<string> cellList;
	readCellList(cellList);
	if (cellList.size() == 0) {
		cout << "please add cellList file" << endl;
		return -1;
	}
	autoLVSandRC(LVSrul, RUNSET, GDS, CDL, cellList);
	printCoutStream();
}