#include "Predictor.h"
#include <stdlib.h>
#include <iostream>
#include<fstream>
#include<sstream>
#include <vector>
#include <string>
#include <math.h>

using namespace std;

Predictor::Predictor(string ifilename, string ofilename) {
	this->num_branches = 0; // Initialize the number of branches
	
	// Temporary variables
	unsigned long long addr;
	string behavior, line;
	unsigned long long target;
  	
  	ifstream infile(ifilename); // Open input file
  	this->ofile.open(ofilename); // Open output file
  	
	// Read lines from the input file
  	while(getline(infile, line)) {
  		this->num_branches++;
  		stringstream s(line);
  		s >> hex >> addr >> behavior >> hex >> target;
  		
  		entry e;
  		e.address = addr;
  		e.target = target;
  		
  		if(behavior == "T") {
  			e.taken = true;
  			this->num_taken++;
  		}
  		else {
  			e.taken = false;
  			this->num_not_taken++;
  		}
  		
  		this->entries.push_back(e);
  	}
}

void Predictor::alwaysTaken() {
	int num_correct = 0;
	
	for(entry e : entries) {
		if(e.taken) num_correct++; // Increment if branch was correctly predicted as taken
	}

	this->ofile << num_correct << "," << this->num_branches << ";" << endl;
}

void Predictor::alwaysNotTaken() {
	int num_correct = 0;

	for(entry e : entries) {
		if(!e.taken) num_correct++; // Increment if branch was correctly predicted as not taken
	}

	this->ofile << num_correct << "," << this->num_branches << ";" << endl;
}

void Predictor::bimodalSingleBit(int table_size) {
	int num_correct = 0;
	vector<bool> table(table_size, true);
	
	// Iterate through all entries
	for(entry e : entries) {
		int key = e.address % table_size;
		
		// Check if the prediction matches the actual outcome
		if(e.taken == table[key]) num_correct++;
		else {
			if(table[key]) table[key] = false;
			else table[key] = true;
		}
	}
	
	this->ofile << num_correct << "," << this->num_branches << "; ";
}

void Predictor::bimodalTwoBits(int table_size) {
	int num_correct = 0;
	vector<int> table(table_size, 3);
	
	// Iterate through all entries
	for(entry e : entries) {
		int key = e.address % table_size; // Calculate the index into the table based on the address modulo the table size
		
		if(e.taken && table[key] > 1) { // If the branch is taken and the counter is strongly taken or weakly taken, increment the number of correct predictions and saturate counter to 3 (strongly taken)
			num_correct++;
			if(table[key] < 3) table[key]++;
		}
		else if (e.taken && table[key] < 2) { // If the branch is taken and the counter is weakly not taken or stronly not taken, saturate the counter to 3 (strongly taken)
			if(table[key] < 3) table[key]++;
		}
		else if(!e.taken && table[key] > 1) { // If the branch is not taken and the counter is strongly taken or weakly taken, decrement the counter unless it is already at the minimum value of 0
			if(table[key] > 0) table[key]--;
		}
		else if(!e.taken && table[key] < 2) { // If the branch is not taken and the counter is weakly not taken or strongly not taken, increment the number of correct predictions and decrement the counter unless it is already at 0
			num_correct++;
			if(table[key] > 0) table[key]--;
		}
	}
	
	this->ofile << num_correct << "," << this->num_branches << "; ";
}

void Predictor::gShare(int ghr_size) {
	int num_correct = 0;
	int ghr = 0 << ghr_size;
	int size = pow(2, ghr_size) - 1;
	vector<int> table(2048, 3);

	// Iterate through all entries
	for(entry e : entries) {
		int key = (e.address ^ (ghr&size)) % 2048;
		
		// Check the conditions based on the prediction outcome and the current state of the counter
		if(e.taken && table[key] > 1) { // If the branch is taken and the counter is strongly taken or weakly taken, increment the number of correct predictions and saturate counter to 3 (strongly taken)
			num_correct++;
			if(table[key] < 3) table[key]++;
			ghr = ghr<<1;
			ghr += 1;
		}
		else if (e.taken && table[key] < 2) { // If the branch is taken and the counter is weakly not taken or stronly not taken, saturate the counter to 3 (strongly taken)
			if(table[key] < 3) table[key]++;
			ghr = ghr<<1;
			ghr += 1;
		}
		else if(!e.taken && table[key] > 1) { // If the branch is not taken and the counter is strongly taken or weakly taken, decrement the counter unless it is already at the minimum value of 0
			if(table[key] > 0) table[key]--;
			ghr = ghr<<1;
		}
		else if(!e.taken && table[key] < 2) { // If the branch is not taken and the counter is weakly not taken or strongly not taken, increment the number of correct predictions and decrement the counter unless it is already at 0
			num_correct++;
			if(table[key] > 0) table[key]--;
			ghr = ghr<<1;
		}
	}
	
	this->ofile << num_correct << "," << this->num_branches << "; ";
}

void Predictor::tournament() {
	int num_correct = 0;
	int ghr = 0;
	int size = pow(2, 11) - 1;

	vector<int> selector_table(2048, 0);
	vector<int> gshare_table(2048, 3);
	vector<int> bimodal_table(2048, 3);
	
	// Tournament logic
	for(entry e : entries) {
		int key = e.address % 2048;
		int gkey = (e.address ^ (ghr&size)) % 2048;
		
		if(selector_table[key] == 0 || selector_table[key] == 1) { // Predict using gshare
			// Conditions based on prediction outcomes for both predictors
			if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3 ) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && e.taken) { // Both correct
				num_correct++;
			}
			else if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && e.taken) { // Gshare correct, bimodal not correct
				num_correct++;
				if(selector_table[key] == 1 || selector_table[key] == 2 || selector_table[key] == 3) selector_table[key]--;
			}
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && e.taken) { // Bimodal correct, gshare not correct
				if(selector_table[key] == 0 || selector_table[key] == 1 || selector_table[key] == 2) selector_table[key]++;
			}
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && !e.taken) { // Both correct
				num_correct++;
			}

			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && !e.taken) { // Gshare correct, bimodal not correct
				num_correct++;
				if(selector_table[key] == 1 || selector_table[key] == 2 || selector_table[key] == 3) selector_table[key]--;
			}
			else if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && !e.taken) { // Bimodal correct, gshare not correct
				if(selector_table[key] == 0 || selector_table[key] == 1 || selector_table[key] == 2) selector_table[key]++;
			}
		}
		else if(selector_table[key] == 2 || selector_table[key] == 3) { // Predict using bimodal
			// Conditions based on prediction outcomes for both predictors
			if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && e.taken) { // Both correct
				num_correct++;
			}
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && e.taken) { // Bimodal correct, ghsare not correct
				num_correct++;
				if(selector_table[key] == 0 || selector_table[key] == 1 || selector_table[key] == 2) selector_table[key]++;
			}
			else if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && e.taken) { // Gshare correct, bimodal not correct
				if(selector_table[key] == 1 || selector_table[key] == 2 || selector_table[key] == 3)  selector_table[key]--;
			}
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && !e.taken) { // Both correct
				num_correct++;
			}
			else if((gshare_table[gkey] == 1 || gshare_table[gkey] == 0) && (bimodal_table[key] == 2 || bimodal_table[key] == 3) && !e.taken) { // Gshare correct, bimodal not correct
				if(selector_table[key] == 1 || selector_table[key] == 2 || selector_table[key] == 3) selector_table[key]--;
			}
			else if((gshare_table[gkey] == 2 || gshare_table[gkey] == 3) && (bimodal_table[key] == 1 || bimodal_table[key] == 0) && !e.taken) { // Bimodal correct, gshare not correct
				num_correct++;
				if(selector_table[key] == 0 || selector_table[key] == 1 || selector_table[key] == 2) selector_table[key]++;
			}
		}
		else {
			cout << "Neither gshare nor bimodal" << endl; // Neither gshare nor bimodal are preffered
		}

		// Update the counters based on the actual outcome of the branch
		if(e.taken) {
			if(gshare_table[gkey] < 3) gshare_table[gkey]++; // Increment the counter in the gshare table if it is less than 3
			ghr = ghr << 1;
			ghr += 1;
			if(bimodal_table[key] < 3) bimodal_table[key]++; // Increment the counter in the bimodal table if it is less than 3
		}
		else {
			if(gshare_table[gkey] > 0) gshare_table[gkey]--; // Decrement the counter in the gshare table if it is greater than 0
			ghr = ghr << 1;
			if(bimodal_table[key] > 0) bimodal_table[key]--; // Decrement the counter in the gshare table if it is greater than 0
		}
	}
	
	this->ofile << num_correct << "," << this->num_branches << "; " << endl;
}

void Predictor::branchTargetBuffer() {
	int num_correct = 0;
	int count = 0;

	vector<bool> predictions(512, true);
	vector<pair<unsigned long long, unsigned long long>> btb(128);
	
	// Iterate through all entries
	for(entry e : entries) {
		int key = e.address % 512;
		int btb_index = e.address % 128;
		
		if(predictions[key] == true) { // Check if the prediction in the predictions vector is true (indicating a predicted branch)
			count++;

			// Check if the entry exists in the branch target buffer
			if(btb[btb_index].first == e.address) {
				if(btb[btb_index].second == e.target) num_correct++;
				else btb[btb_index].second = e.target;
			}
			else {
				btb[btb_index].first = e.address;
				btb[btb_index].second = e.target;
			}
		}
		
		predictions[key] = e.taken; // Update the prediction in the predictions vector based on the actual outcome of the branch
	}

	this->ofile << count << "," << num_correct << ";" << endl;
	this->ofile.close();
}

// void Predictor::getEntries() {
// 	for(entry e : entries) cout << "taken?: " << e.taken << " " << "address: " << e.address << endl; // Output whether the branch was taken and its address
// }

void Predictor::output(string s) {
	this->ofile << s;
}