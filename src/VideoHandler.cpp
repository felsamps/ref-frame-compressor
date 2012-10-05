#include "../inc/VideoHandler.h"

VideoHandler::VideoHandler(int w, int h, int nv, int nf, string name) {
	this->w = w;
	this->h = h;
	this->nv = nv;
	this->nf = nf;

	this->targetFrame = -1;
	this->targetView = -1;

	this->frame = new Pel[w * h];

	xInitFileNames(name);
}

void VideoHandler::xInitFileNames(string name) {
	for (int v = 0; v < nv; v++) {
		stringstream ss;
		ss << v;
		this->fileNames.push_back(name + "_" + ss.str() + ".yuv");
	}
}

int VideoHandler::xCalcFilePos(int f) {
	return (this->w * this->h * 1.5) * f;
}

void VideoHandler::xHandleTargetFile(int v, int f) {
	if(this->targetView != -1) {
		this->file.close();
	}
	if(this->targetView != v) {
		this->file.open(this->fileNames[v].c_str(), fstream::in);
	}

	this->targetView = v;
	this->targetFrame = f;

	int filePos = xCalcFilePos(f);
	this->file.seekg(filePos ,ios::beg);
	this->file.read(this->frame, this->w * this->h);

}

Pel **VideoHandler::getBlock(int v, int f, int x, int y) {
	if(this->targetView != v || this->targetFrame != f) {
		xHandleTargetFile(v, f);
	}

	Pel **returnable = new Pel*[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++) {
		returnable[i] = new Pel[BLOCK_SIZE];
	}

	for (int j = 0; j < BLOCK_SIZE; j++) {
		for (int i = 0; i < BLOCK_SIZE; i++) {
			returnable[i][j] = this->frame[(i+x) + (j+y) * this->w];
		}
	}

	return returnable;

}