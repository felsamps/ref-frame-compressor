#include <iostream>
#include <cstdlib>
#include <string>

#include "../inc/VideoHandler.h"
#include "../inc/Defines.h"
#include "../inc/IntraEncoder.h"
#include "../inc/Huffman.h"
#include "../inc/Quantizer.h"

using namespace std;

int main(int argc, char** argv) {

	int mode = atoi(argv[1]);
	int w = atoi(argv[2]);
	int h = atoi(argv[3]);
	int nv = atoi(argv[4]);
	int gops = atoi(argv[5]);

	string reconFileName(argv[6]);
	string traceFileName(argv[7]);
	string huffDictFileName(argv[8]);
	string videoFileName(argv[9]);
	string quantFileName(argv[10]);

	VideoHandler* vh = new VideoHandler(w, h, nv, gops, reconFileName, videoFileName);
	Huffman *huffRes = new Huffman(huffDictFileName);
	Quantizer* q = new Quantizer(quantFileName);
	IntraEncoder* ie = new IntraEncoder(mode, vh, huffRes, q, traceFileName);

	ie->encode();
	ie->reportCSV();

}

