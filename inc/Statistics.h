#ifndef STATISTICS_H
#define	STATISTICS_H

#include <map>
#include <iostream>
#include <cmath>

#include "Defines.h"

using namespace std;

class Statistics {
private:
    map<int, long long> occ;
    long long total;
    
    pair<double,double> xCalcMeanStd();
    
public:
    Statistics();
    
    void insertValue(int value);
    void generateProbabilities();
    void report();
};

#endif	/* STATISTICS_H */

