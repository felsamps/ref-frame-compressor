#include <iostream>
#include <cstdlib>
#include <string>

#include "../inc/VideoHandler.h"
#include "../inc/Defines.h"
#include "../inc/IntraEncoder.h"
#include "../inc/Huffman.h"

using namespace std;

int main(int argc, char** argv) {

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	int nv = atoi(argv[3]);
	int gops = atoi(argv[4]);

	string reconFileName(argv[5]);
	string traceFileName(argv[6]);
	string huffDictFileName(argv[7]);

	VideoHandler* vh = new VideoHandler(w, h, nv, gops, reconFileName);
	Huffman *huffRes = new Huffman(huffDictFileName);
	IntraEncoder* ie = new IntraEncoder(vh, huffRes, traceFileName);

	ie->encode();
	ie->report();

	Huffman* hf = new Huffman("dict.mat");
}

