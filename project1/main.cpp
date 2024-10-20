#include <iostream>
#include <stdlib.h>
#include "Predictor.h"

int main(int argc, char* argv[]) {
	Predictor p = Predictor(argv[1], argv[2]);

	p.alwaysTaken();
	
	p.alwaysNotTaken();
	
	p.bimodalSingleBit(16);
	p.bimodalSingleBit(32);
	p.bimodalSingleBit(128);
	p.bimodalSingleBit(256);
	p.bimodalSingleBit(512);
	p.bimodalSingleBit(1024);
	p.bimodalSingleBit(2048);
	p.output("\n");

	p.bimodalTwoBits(16);
	p.bimodalTwoBits(32);
	p.bimodalTwoBits(128);
	p.bimodalTwoBits(256);
	p.bimodalTwoBits(512);
	p.bimodalTwoBits(1024);
	p.bimodalTwoBits(2048);
	p.output("\n");
	
	p.gShare(3);
	p.gShare(4);
	p.gShare(5);
	p.gShare(6);
	p.gShare(7);
	p.gShare(8);
	p.gShare(9);
	p.gShare(10);
	p.gShare(11);
	p.output("\n");
	
	p.tournament();
	
	p.branchTargetBuffer();
}