#ifndef _HUFFMAN_H
#define	_HUFFMAN_H

#include <iostream>
#include <fstream>
#include <map>
#include <list>

#include "Defines.h"

using namespace std;

class Huffman {
private:
    fstream dictFile;
    map<Pel, list<char> > dict;

    void xParserDictFile();
    

public:
    Huffman(string fileName);
    
    list<char> encodeBlock(Pel** block);
    list<char> encodeSubBlock(Pel** block, int xx, int yy);

};

#endif	/* _HUFFMAN_H */

