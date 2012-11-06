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
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			list<char> l = this->dict[block[x][y]];
			returnable.merge(l);
		}
		return returnable;
	}

}