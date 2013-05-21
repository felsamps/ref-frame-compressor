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

    Pel** quantize(Pel** residue, int size,  int x, int y);
    void invQuantize(Pel **residue, int size,  int x, int y);
    
    Pel** getError(Pel** residue, int size,  int x, int y);

    int getNumOfLevels();

};

#endif	/* _QUANTIZER_H */

