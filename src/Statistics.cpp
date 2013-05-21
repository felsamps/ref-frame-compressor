#include "../inc/Statistics.h"

Statistics::Statistics() {
	this->occ.clear();
	this->total = 0;
	for (int i = -128; i < 128; i++) {
		this->occ[i] = 0;
	}

}

void Statistics::insertValue(int value) {
	this->occ[value] = (this->occ.find(value) == occ.end()) ? 1 : occ[value]+1;
	this->total += 1;
}

pair<double,double> Statistics::calcStats() {
	/* mean calculation */
	long long total = 0;
	double mean = 0.0;
	
	for (int i = -128; i < 128; i++) {
		total += this->occ[i];
		mean += this->occ[i] * i;
	}
	mean = mean / total;
	
	/* standard deviation calculation */
	double acum = 0.0;
	for (int i = -128; i < 128; i++) {
		acum += (pow(mean-i, 2) * this->occ[i]);
	}	
	
	double std = pow(acum / (total-1), 0.5);
	
	pair<double, double> returnable = make_pair(mean, std);
	return returnable;
	
}

void Statistics::report() {	
	
	pair<double, double> stats;
	
	stats = calcStats();
	double mean = stats.first;
	double std = stats.second;
	
	
	int TH_H = (int) mean + SIGMA_TIMES * std;
	int TH_L = (int) mean - SIGMA_TIMES * std;
	double pctg = SIGMA_PCTG;
		
	double acum = 0;
	for(map<int, long long>::iterator it = this->occ.begin(); it != this->occ.end(); it++) {
		int residue = (*it).first;
		int occ = (*it).second;
		
		if(residue > TH_L and residue <= TH_H) {
			cout << residue << " " << occ / (double)this->total << "\n";
			acum += occ / (double)this->total;
		}
		else {
			cout << residue << " 0.0" << endl;
		}
		
	}
	cout << "128 " << 1 - acum << endl;
	
}
