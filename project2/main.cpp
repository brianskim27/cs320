#include <iostream>
#include <stdlib.h>
#include "cache.h"

int main(int argc, char *argv[])
{
	trace temp;
	vector<trace> traces;
	ifstream fin(argv[1]);
	
	while (fin >> temp.type >> std::hex >> temp.address)
	{
		traces.push_back(temp);
	}
	
	fin.close();
	
	ofstream fout(argv[2]);
	
	directMapped(fout, traces);
	setAssociative(fout, traces);
	fullyAssociative(fout, traces);
	setAssociativeNoAllocationWriteMiss(fout, traces);
	setAssociativeNextLinePrefetching(fout, traces);
	prefetchMiss(fout, traces);
	
	fout.close();
	return 0;
}