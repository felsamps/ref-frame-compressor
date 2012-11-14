#include "../inc/VideoHandler.h"

VideoHandler::VideoHandler(int w, int h, int nv, int gops, string name, string videoName) {
	this->w = w;
	this->h = h;
	this->nv = nv;
	this->gops = gops;
	this->videoName = videoName;

	this->targetFrame = -1;
	this->targetView = -1;

	this->reconFrame = new UPel[w * h];
	this->residualFrame = new Pel[w * h];
	this->errorFrame = new Pel[w * h];

	this->lossyReconFrame = new UPel**[nv];
	for (int v = 0; v < nv; v++) {
		this->lossyReconFrame[v] = new UPel*[1 + gops*GOP_SIZE];
		for (int f = 0; f < (1 + gops*GOP_SIZE); f++) {
			this->lossyReconFrame[v][f] = new UPel[(int)(w * h)];
		}
	}


	this->modeFrame = new bool[(w/BLOCK_SIZE) * (h/BLOCK_SIZE)];

	this->residualFile.open("residue.mat", fstream::out);
	this->errorFile.open("errors.mat", fstream::out);
	

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
	if(this->targetView != v || this->targetFrame != f) {
		if(this->targetView != -1 && this->targetView != v) {
			this->reconFile.close();
		}
		if(this->targetView != v) {
			this->reconFile.open(this->fileNames[v].c_str(), fstream::in);
		}

		this->targetView = v;
		this->targetFrame = f;
		
		int filePos = xCalcFilePos(f);
		this->reconFile.seekg(filePos ,ios::beg);
		this->reconFile.read((char*)this->reconFrame, this->w * this->h);


	}
}

UPel** VideoHandler::getBlock(int v, int f, int x, int y) {
	xHandleTargetFile(v, f);

	UPel **returnable = new UPel*[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++) {
		returnable[i] = new UPel[BLOCK_SIZE];
	}

	for (int j = 0; j < BLOCK_SIZE; j++) {
		for (int i = 0; i < BLOCK_SIZE; i++) {
			returnable[i][j] = this->reconFrame[(i+x) + (j+y) * this->w];
		}
	}

	return returnable;

}

UPel **VideoHandler::getSubBlock(int v, int f, int x, int y) {
	xHandleTargetFile(v, f);

	UPel **returnable = new UPel*[SUB_BLOCK_SIZE];
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		returnable[i] = new UPel[SUB_BLOCK_SIZE];
	}

	for (int j = 0; j < SUB_BLOCK_SIZE; j++) {
		for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
			returnable[i][j] = this->reconFrame[(i+x) + (j+y) * this->w];
		}
	}

	return returnable;
}



UPel* VideoHandler::getNeighboring(int v, int f, int x, int y) {
	xHandleTargetFile(v, f);

	UPel *returnable = new UPel[2*BLOCK_SIZE + 1];
	
	/* upper-right neighbor sample */
	returnable[BLOCK_SIZE] = (x!=0 && y!=0) ? this->reconFrame[(x-1) + (y-1)*this->w] : -1;

	/* left samples */
	for (int j = 0; j < BLOCK_SIZE; j++) {
		returnable[(BLOCK_SIZE-1) - j] = (x != 0) ? this->reconFrame[(x-1) + (j+y)*this->w] : -1;
	}

	/* upper samples */
	for (int i = 0; i < BLOCK_SIZE; i++) {
		returnable[(BLOCK_SIZE+1) + i] = (y != 0) ? this->reconFrame[(i+x) + (y-1)*this->w] : -1;
	}

	return returnable;
	
}

UPel* VideoHandler::getSubNeighboring(int v, int f, int x, int y) {
	xHandleTargetFile(v, f);

	UPel *returnable = new UPel[3*SUB_BLOCK_SIZE + 1];

	/* upper-right neighbor sample */
	returnable[SUB_BLOCK_SIZE] = (x!=0 && y!=0) ? this->reconFrame[(x-1) + (y-1)*this->w] : -1;

	/* left samples */
	for (int j = 0; j < SUB_BLOCK_SIZE; j++) {
		returnable[(SUB_BLOCK_SIZE-1) - j] = (x != 0) ? this->reconFrame[(x-1) + (j+y)*this->w] : -1;
	}

	/* upper samples */
	for (int i = 0; i < 2*SUB_BLOCK_SIZE; i++) {
		returnable[(SUB_BLOCK_SIZE+1) + i] = 
				(y == 0) ? -1 :
					((x+i) >= this->w) ? -1 :
						this->reconFrame[(i+x) + (y-1)*this->w];
	}

	return returnable;

}

void VideoHandler::insertResidualBlock(Pel** block, int x, int y, bool mode) {
	for (int j = 0; j < BLOCK_SIZE; j++) {
		for (int i = 0; i < BLOCK_SIZE; i++) {
			this->residualFrame[(i+x) + (j+y) * this->w] = block[i][j];
		}
	}
	this->modeFrame[x/BLOCK_SIZE + (y/BLOCK_SIZE)+this->w] = mode;
}

void VideoHandler::insertResidualSubBlock(Pel** block, int x, int y, int xx, int yy) {
	for (int j = 0; j < SUB_BLOCK_SIZE; j++) {
		for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
			this->residualFrame[(i+xx) + (j+yy) * this->w] = block[i+x][j+y];
		}
	}
}

void VideoHandler::writeResidualFrameInFile() {
	for (int y = 0; y < this->h; y++) {
		for (int x = 0; x < this->w; x++) {
			this->residualFile << (int)this->residualFrame[x + y*this->w] << " ";
		}
		this->residualFile << endl;
	}
}

void VideoHandler::insertErrorBlock(Pel** block, int x, int y) {
	for (int j = 0; j < BLOCK_SIZE; j++) {
		for (int i = 0; i < BLOCK_SIZE; i++) {
			this->errorFrame[(i+x) + (j+y) * this->w] = block[i][j];
		}
	}
}

void VideoHandler::writeErrorFrameInFile() {
	for (int y = 0; y < this->h; y++) {
		for (int x = 0; x < this->w; x++) {
			this->errorFile << (int)this->errorFrame[x + y*this->w] << " ";
		}
		this->errorFile << endl;
	}
}

void VideoHandler::insertLossyReconBlock(int** block, int view, int frame, int x, int y) {

	for (int j = 0; j < BLOCK_SIZE; j++) {
		for (int i = 0; i < BLOCK_SIZE; i++) {
			this->lossyReconFrame[view][frame][(i+x) + (j+y) * this->w] = (UPel) block[i][j];
		}
	}
	
}

void VideoHandler::writeLossyReconInFile() {
	vector<string> intToStr;
	intToStr.push_back("0");
	intToStr.push_back("1");
	intToStr.push_back("2");
	this->reconFile.close();
	
	for (int v = 0; v < this->nv; v++) {
		string fileName;
		fileName = "lossy_recon_" + intToStr[v] + ".yuv";
		this->lossyReconFile.open(fileName.c_str(), fstream::out);
		this->reconFile.open(this->fileNames[v].c_str(), fstream::in);
		for (int f = 0; f < this->gops*GOP_SIZE + 1; f++) {			
			int framePos = xCalcFilePos(f) + (this->w*this->h);
			this->reconFile.seekg(framePos, ios::beg);
			
			UPel* chroma = new UPel[ (this->w*this->h)/2];
			this->reconFile.read((char*)chroma, (this->w*this->h)/2 );

			this->lossyReconFile.write((char*)this->lossyReconFrame[v][f], this->w * this->h);
			this->lossyReconFile.write((char*)chroma, (this->w*this->h)/2 );
		}
		this->reconFile.close();
		this->lossyReconFile.close();
	}
}

void VideoHandler::closeFiles() {
	this->residualFile.close();
	this->errorFile.close();
	this->lossyReconFile.close();
}

int VideoHandler::getHeight() {
	return this->h;
}

int VideoHandler::getWidth() {
	return this->w;
}

int VideoHandler::getNumOfGOP() {
	return this->gops;
}

int VideoHandler::getNumOfViews() {
	return this->nv;
}

string VideoHandler::getVideoName() {
	return this->videoName;
}

