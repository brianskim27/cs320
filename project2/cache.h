#ifndef CACHE_SIM_H
#define CACHE_SIM_H

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct trace {
    char type;
    unsigned long long address;
};

struct cache {
    bool isValid = false;
    unsigned long long tag = 0;
};

int log2(int base);

void directMapped(ofstream& fout, vector<trace> traces);
void setAssociative(ofstream& fout, vector<trace> traces);
void fullyAssociative(ofstream& fout, vector<trace> traces);
void fullyAssociativeLru(ofstream& fout, vector<trace> traces);
void fullyAssociativeHotCold(ofstream& fout, vector<trace> traces);
void setAssociativeNoAllocationWriteMiss(ofstream& fout, vector<trace> traces);
void setAssociativeNextLinePrefetching(ofstream& fout, vector<trace> traces);
void prefetchMiss(ofstream& fout, vector<trace> traces);

#endif // CACHE_SIM_H