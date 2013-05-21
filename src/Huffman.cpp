#include <list>

#include "../inc/Huffman.h"

Huffman::Huffman(string fileName) {
	this->dictFile.open(fileName.c_str(), fstream::in);
	this->dict.clear();

	this->xParserDictFile();
}

void Huffman::xParserDictFile() {
	while( !this->dictFile.eof() ) {
		int key;
		int size;
		list<char> l;
		this->dictFile >> key >> size;
		for (int i = 0; i < size; i++) {
			char c;
			this->dictFile >> c;
			l.push_back(c);
		}
		this->dict[(Pel)key] = l;
	}
}

list<char> Huffman::encodeBlock(Pel** block) {
	list<char> returnable;
	for (int y = 0; y < MACROBLOCK_SIZE; y++) {
		for (int x = 0; x < MACROBLOCK_SIZE; x++) {
			Pel sample = block[x][y];
			if(this->dict.find(sample) == this->dict.end()) {
				list<char> l = this->dict[128]; /* special code assignment */
				returnable.merge(l);
				list<char> l1(8, '0'); /* raw residue assignment */
				returnable.merge(l1);
			}
			else {
				list<char> l = this->dict[sample];
				returnable.merge(l);
			}
			
		}
	}
	return returnable;

}

list<char> Huffman::encodeSubBlock(Pel** block, int xx, int yy) {
	list<char> returnable;
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			Pel sample = block[x+xx][y+yy];
			if(this->dict.find(sample) == this->dict.end()) {
				list<char> l = this->dict[128]; /* special code assignment */
				returnable.merge(l);
				list<char> l1(8, '0'); /* raw residue assignment */
				returnable.merge(l1);
			}
			else {
				list<char> l = this->dict[sample];
				returnable.merge(l);
			}
		}
	}
	return returnable;

}