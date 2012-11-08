#include "../inc/VideoHandler.h"

VideoHandler::VideoHandler(int w, int h, int nv, int gops, string name, string videoName) {
	this->w = w;
	this->h = h;
	this->nv = nv;
	this->gops = gops;
	this->videoName = videoName;

	this->targetFrame = -1;
	this->targetView = -1;

	this->reconFrame = new Pel[w * h];
	this->residualFrame = new Pel[w * h];
	this->modeFrame = new bool[(w/BLOCK_SIZE) * (h/BLOCK_SIZE)];

	this->residualFile.open("residue.mat", fstream::out);
	this->varFile.open("variance.mat", fstream::out);

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
		if(this->targetView != -1) {
			this->reconFile.close();
		}
		if(this->targetView != v) {
			this->reconFile.open(this->fileNames[v].c_str(), fstream::in);
		}

		this->targetView = v;
		this->targetFrame = f;

		int filePos = xCalcFilePos(f);
		this->reconFile.seekg(filePos ,ios::beg);
		this->reconFile.read(this->reconFrame, this->w * this->h);
	}
}

Pel** VideoHandler::getBlock(int v, int f, int x, int y) {
	xHandleTargetFile(v, f);

	Pel **returnable = new Pel*[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++) {
		returnable[i] = new Pel[BLOCK_SIZE];
	}

	for (int j = 0; j < BLOCK_SIZE; j++) {
		for (int i = 0; i < BLOCK_SIZE; i++) {
			returnable[i][j] = this->reconFrame[(i+x) + (j+y) * this->w];
		}
	}

	return returnable;

}

Pel **VideoHandler::getSubBlock(int v, int f, int x, int y) {
	xHandleTargetFile(v, f);

	Pel **returnable = new Pel*[SUB_BLOCK_SIZE];
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		returnable[i] = new Pel[SUB_BLOCK_SIZE];
	}

	for (int j = 0; j < SUB_BLOCK_SIZE; j++) {
		for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
			returnable[i][j] = this->reconFrame[(i+x) + (j+y) * this->w];
		}
	}

	return returnable;
}



Pel* VideoHandler::getNeighboring(int v, int f, int x, int y) {
	xHandleTargetFile(v, f);

	Pel *returnable = new Pel[2*BLOCK_SIZE + 1];
	
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

Pel* VideoHandler::getSubNeighboring(int v, int f, int x, int y) {
	xHandleTargetFile(v, f);

	Pel *returnable = new Pel[3*SUB_BLOCK_SIZE + 1];

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

void VideoHandler::insertResidualSubBlock(Pel** block, int x, int y) {
	for (int j = 0; j < SUB_BLOCK_SIZE; j++) {
		for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
			this->residualFrame[(i+x) + (j+y) * this->w] = block[i][j];
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

void VideoHandler::calcVar() {
	for (int y = 0; y < this->h; y+=BLOCK_SIZE) {
		for (int x = 0; x < this->w; x+=BLOCK_SIZE) {
			int acum = 0;
			for (int yy = 0; yy < BLOCK_SIZE; yy++) {
				for (int xx = 0; xx < BLOCK_SIZE; xx++) {
					acum += this->reconFrame[x+xx + (y+yy)*this->w];
				}
			}
			double average = acum / (double)(BLOCK_SIZE*BLOCK_SIZE);
			double dAcum = 0.0;
			for (int yy = 0; yy < BLOCK_SIZE; yy++) {
				for (int xx = 0; xx < BLOCK_SIZE; xx++) {
					dAcum += pow(average - this->reconFrame[x+xx + (y+yy)*this->w], 2);
				}
			}
			double variance = dAcum / (double)(BLOCK_SIZE*BLOCK_SIZE);
			this->varFile << variance << " ";
		}
		this->varFile << endl;
	}

}

void VideoHandler::closeFiles() {
	this->residualFile.close();
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
