#ifndef PREDICTOR_H
#define PREDICTOR_H
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
struct entry {
	bool taken;
	unsigned long long address;
	unsigned long long target;
};

class Predictor {
	public:
		Predictor(string, string);
		void alwaysTaken();
		void alwaysNotTaken();
		void bimodalSingleBit(int table_size);
		void bimodalTwoBits(int table_size);
		void gShare(int ghr_size);
		void tournament();
		void branchTargetBuffer();
		//void getEntries();
		void output(string);
	
	private:
		vector<entry> entries;
		ofstream ofile;
		int num_taken;
		int num_not_taken;
		int num_branches;
};

#endif