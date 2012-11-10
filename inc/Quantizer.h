#ifndef _QUANTIZER_H
#define	_QUANTIZER_H

#include <string>
#include <fstream>
#include <map>
#include <iostream>

#include "Defines.h"

using namespace std;

class Quantizer {
private:
    fstream quantFile;
    vector<double> levels;
    map<Pel, Pel> fQuant, iQuant;
    int numOfLevels;

    void xParseQuantFile();
    void xGenerateQuantTable();
    
public:
    Quantizer(string fileName);
    void report();

    Pel** quantize(Pel** residue, int size);
    void invQuantize(Pel **residue, int size);

};

#endif	/* _QUANTIZER_H */

