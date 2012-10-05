#include <iostream>
#include <cstdlib>
#include <string>

#include "../inc/VideoHandler.h"
#include "../inc/Defines.h"

using namespace std;

int main(int argc, char** argv) {

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	int nv = atoi(argv[3]);
	int nf = atoi(argv[4]);

	string reconFileName(argv[5]);

	VideoHandler vh(w, h, nv, nf, reconFileName);
	
}

