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

	int opMode = atoi(argv[1]);

	//for all opModes
	int w = atoi(argv[2]);
	int h = atoi(argv[3]);
	int nv = atoi(argv[4]);
	int gops = atoi(argv[5]);
	
	string reconFileName(argv[6]);
	string traceFileName(argv[7]);
	//endfor

	string huffDictFileName;
	string videoFileName;
	string quantFileName;

	string huff8DictFileName;
	string huff16DictFileName;
	string huff32DictFileName;
	string quant8FileName;
	string quant16FileName;
	string quant32FileName;
	string quantFile;

	Quantizer* q;
	Quantizer* q8;
	Quantizer* q16;
	Quantizer* q32;

	Huffman *huffRes;
	Huffman *huffRes8;
	Huffman *huffRes16;
	Huffman *huffRes32;

	VideoHandler* vh;
	IntraEncoder* ie;

	switch(opMode) {
		case 0:
			vh = new VideoHandler(w, h, nv, gops, reconFileName, "");
			ie = new IntraEncoder(opMode, vh, traceFileName); 
			
			ie->encode();
			//ie->reportOcc();
			ie->reportStats();
			
			break;
		
		case 1:
			huffDictFileName.assign(argv[8]);
			videoFileName.assign(argv[9]);
			
			huffRes = new Huffman(huffDictFileName);
			vh = new VideoHandler(w, h, nv, gops, reconFileName, videoFileName);
			ie = new IntraEncoder(opMode, vh, traceFileName, huffRes);
			
			ie->encode();
			ie->reportCSV();
						
			break;

		
		case 2:
			quantFileName.assign(argv[8]);

			q = new Quantizer(quantFileName);
			vh = new VideoHandler(w, h, nv, gops, reconFileName, " ");
			ie = new IntraEncoder(opMode, vh, traceFileName, q);
			
			ie->encode();
			ie->reportOcc();
			
			break;
		
		case 3:
			huffDictFileName.assign(argv[8]);
			huff8DictFileName.assign(argv[9]);
			huff16DictFileName.assign(argv[10]);
			huff32DictFileName.assign(argv[11]);
			
			quant8FileName.assign(argv[12]);
			quant16FileName.assign(argv[13]);
			quant32FileName.assign(argv[14]);
			videoFileName.assign(argv[15]);

			huffRes = new Huffman(huffDictFileName);
			huffRes8 = new Huffman(huff8DictFileName);
			huffRes16 = new Huffman(huff16DictFileName);
			huffRes32 = new Huffman(huff32DictFileName);
			q8 = new Quantizer(quant8FileName);
			q16 = new Quantizer(quant16FileName);
			q32 = new Quantizer(quant32FileName);

			
			vh = new VideoHandler(w, h, nv, gops, reconFileName, videoFileName);
			ie = new IntraEncoder(opMode, vh, traceFileName, huffRes, huffRes8, huffRes16, huffRes32, q8, q16, q32);
			ie->encode();
			ie->reportCSV();
			
			break;
/*
		case 4:
			huffDictFileName.assign(argv[9]);
			huff8DictFileName.assign(argv[10]);
			huff16DictFileName.assign(argv[11]);
			huff32DictFileName.assign(argv[12]);
			quant8FileName.assign(argv[13]);
			quant16FileName.assign(argv[14]);
			quant32FileName.assign(argv[15]);
			videoFileName.assign(argv[16]);
			quantFile.assign(argv[17]);

			huffRes = new Huffman(huffDictFileName);
			huffRes8 = new Huffman(huff8DictFileName);
			huffRes16 = new Huffman(huff16DictFileName);
			huffRes32 = new Huffman(huff32DictFileName);
			q8 = new Quantizer(quant8FileName);
			q16 = new Quantizer(quant16FileName);
			q32 = new Quantizer(quant32FileName);

			vh = new VideoHandler(w, h, nv, gops, reconFileName, videoFileName);
			ie = new IntraEncoder(opMode, mode, vh, traceFileName, huffRes, huffRes8, huffRes16, huffRes32, q8, q16, q32, quantFile);
			break;*/

	}

}

