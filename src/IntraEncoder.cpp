#include <list>

#include "../inc/IntraEncoder.h"

int zzBlockOrder[16][2] =
{
  {0,0},  {1,0},  {0,1},  {1,1},
  {2,0},  {3,0},  {2,1},  {3,1},
  {0,2},  {1,2},  {0,3},  {1,3},
  {2,2},  {3,2},  {2,3},  {3,3}
};

int viewOrder[8] = {0, 2, 1, 4, 3, 6, 5, 7};

int frameGOPOrder[9] = {0, 8, 4, 2, 6, 1, 3, 5, 7};

int abs(int a) {
	return (a<=0) ? -a : a;
}

int sat(int a) {
	return (a < 0) ? 0 : (a > 255) ? 255 : a;
}

IntraEncoder::IntraEncoder(int mode, VideoHandler* vh, Huffman* huffRes, string name) {
	this->vh = vh;
	this->huffRes = huffRes;
	this->mode = mode;

	this->compressedBitCount = 0;
	this->uncompressedBitCount = 0;
	this->blockChoices = 0;
	this->subBlockChoices = 0;

	this->traceFile.open(name.c_str(), fstream::in);
	
}

void IntraEncoder::xHorizonalMode(Pel* neighbors, Pel** pred, int size) {
	bool isLeftRef = (neighbors[0] != -1);
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			if(isLeftRef) {
				Pel predSample = neighbors[(size-1) - y];
				pred[x][y] = predSample;
			}
		}
	}
}

void IntraEncoder::xVerticalMode(Pel* neighbors, Pel** pred, int size) {
	bool isUpRef = (neighbors[size+1] != -1);
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			if(isUpRef) {
				Pel predSample = neighbors[(size+1) + x];
				pred[x][y] = predSample;
			}
		}
	}
}

void IntraEncoder::xDCMode(Pel* neighbors, Pel** pred, int size) {
	unsigned int predSample = 0;
	int validNeigh = 0;

	for (int i = 0; i < 2*size + 1; i++) {
		if(neighbors[i] != -1) {
			predSample += (unsigned char) neighbors[i];
			validNeigh ++;
		}
	}
	
	if(validNeigh != 0) {
		predSample /= validNeigh;
	}
	else {
		predSample = 0x80;
	}

	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			pred[x][y] = predSample;
		}
	}

}

void IntraEncoder::xDRMode(Pel* neighbors, Pel** pred) {
	pred[3][0] = (neighbors[6] + neighbors[7]*2 + neighbors[8] + 2) >> 2;
	pred[2][0] = pred[3][1] = (neighbors[5] + neighbors[6]*2 + neighbors[7] + 2) >> 2;
	pred[1][0] = pred[2][1] = pred[3][2] = (neighbors[4] + neighbors[5]*2 + neighbors[6] + 2) >> 2;
	pred[0][0] = pred[1][1] = pred[2][2] = pred[3][3] = (neighbors[3] + neighbors[4]*2 + neighbors[5] + 2) >> 2;
				 pred[0][1] = pred[1][2] = pred[2][3] = (neighbors[2] + neighbors[3]*2 + neighbors[4] + 2) >> 2;
							  pred[0][2] = pred[1][3] = (neighbors[1] + neighbors[2]*2 + neighbors[3] + 2) >> 2;
										   pred[0][3] = (neighbors[0] + neighbors[1]*2 + neighbors[2] + 2) >> 2;

}

void IntraEncoder::xDLMode(Pel* neighbors, Pel** pred) {

	int n = SUB_BLOCK_SIZE+1;
	for (int y = 0; y < SUB_BLOCK_SIZE; y++) {
		int nn = n;
		for (int x = 0; x < SUB_BLOCK_SIZE; x++) {
			Pel predSample;
			if(x == 3 && y == 3) {
				predSample = (neighbors[nn] + 2*neighbors[nn+1] + neighbors[nn+1] + 2) >> 2;
			}
			else {
				predSample = (neighbors[nn] + 2*neighbors[nn+1] + neighbors[nn+2] + 2) >> 2;
			}
			pred[x][y] = predSample;
			nn++;
		}
		n++;
	}
}

void IntraEncoder::xVRMode(Pel* neighbors, Pel** pred) {
	int n = SUB_BLOCK_SIZE;
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		Pel predSample = (neighbors[n] + neighbors[n+1] + 1) >> 1;
		pred[i][0] = predSample;
		if(i >= 0 && i < 3) {
			pred[i+1][2] = predSample;
		}
		n++;
	}

	n = SUB_BLOCK_SIZE-1;
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		Pel predSample = (neighbors[n] + 2*neighbors[n+1] + neighbors[n+2] + 1) >> 2;
		pred[i][1] = predSample;
		if(i >= 0 && i < 3) {
			pred[i+1][3] = predSample;
		}
		n++;
	}

	n = SUB_BLOCK_SIZE;
	Pel predSample = (neighbors[n] + 2*neighbors[n-1] + neighbors[n-2] + 1);
	pred[0][2] = predSample;
	predSample = (neighbors[n-1] + 2*neighbors[n-2] + neighbors[n-3] + 1);
	pred[0][3] = predSample;
}

void IntraEncoder::xHDMode(Pel* neighbors, Pel** pred) {
	pred[0][0] = pred[2][1] = (neighbors[4] + neighbors[3] + 1) >> 1;
	pred[1][0] = pred[3][1] = (neighbors[3] + neighbors[4]*2 + neighbors[5] + 2) >> 2;
	pred[2][0] = (neighbors[4] + neighbors[5]*2 + neighbors[6] + 2) >> 2;
	pred[3][0] = (neighbors[5] + neighbors[6]*2 + neighbors[7] + 2) >> 2;

	pred[0][1] = pred[2][2] = (neighbors[3] + neighbors[2] + 1) >> 1;
	pred[1][1] = pred[3][2] = (neighbors[4] + neighbors[3]*2 + neighbors[2] + 2) >> 2;
	pred[0][2] = pred[2][3] = (neighbors[2] + neighbors[1] + 1) >> 1;
	pred[1][2] = pred[3][3] = (neighbors[3] + neighbors[2]*2 + neighbors[1] + 2) >> 2;

	pred[0][3] = (neighbors[0] + neighbors[1] + 1) >> 1;
	pred[1][3] = (neighbors[2] + neighbors[1]*2 + neighbors[0] + 2) >> 2;

}

void IntraEncoder::xVLMode(Pel* neighbors, Pel** pred) {
	pred[0][0] = (neighbors[5] + neighbors[6] + 1) >> 1;

	pred[1][0] = pred[0][2] = (neighbors[6] + neighbors[7] + 1) >> 1;
	pred[2][0] = pred[1][2] = (neighbors[7] + neighbors[8] + 1) >> 1;
	pred[3][0] = pred[2][2] = (neighbors[8] + neighbors[9] + 1) >> 1;
				 pred[3][2] = (neighbors[9] + neighbors[10] + 1) >> 1;

	pred[0][1] =              (neighbors[5] + neighbors[6]*2 + neighbors[7] + 2) >> 2;
	pred[1][1] = pred[0][3] = (neighbors[6] + neighbors[7]*2 + neighbors[8] + 2) >> 2;
	pred[2][1] = pred[1][3] = (neighbors[7] + neighbors[8]*2 + neighbors[9] + 2) >> 2;
	pred[3][1] = pred[2][3] = (neighbors[8] + neighbors[9]*2 + neighbors[10] + 2) >> 2;
				 pred[3][3] = (neighbors[9] + neighbors[10]*2 + neighbors[11] + 2) >> 2;

}

void IntraEncoder::xHUMode(Pel* neighbors, Pel** pred) {
	pred[0][0] = (neighbors[3] + neighbors[2] + 1) >> 1;
	pred[1][0] = (neighbors[3] + neighbors[2]*2 + neighbors[1] + 2) >> 2;
	pred[2][0] = pred[0][1] = (neighbors[2] + neighbors[1] + 1) >> 1;
	pred[2][0] = pred[0][1] = (neighbors[2] + neighbors[1]*2 + neighbors[0] + 2) >> 2;

	pred[2][2] = pred[0][2] = (neighbors[1] + neighbors[0] + 1) >> 1;
	pred[3][1] = pred[1][2] = (neighbors[1] + neighbors[0]*2 + neighbors[0] + 2) >> 2;
	
	pred[2][2] = pred[3][2] = pred[0][3] = pred[2][3] = pred[1][3] = pred[3][3] = neighbors[0];

}

void IntraEncoder::xPlaneMode(Pel* neighbors, Pel** pred) {
	int hLine = (neighbors[25] - neighbors[23]) * 1 +
			(neighbors[26] - neighbors[22]) * 2 +
			(neighbors[27] - neighbors[21]) * 3 +
			(neighbors[28] - neighbors[20]) * 4 +
			(neighbors[29] - neighbors[19]) * 5 +
			(neighbors[30] - neighbors[18]) * 6 +
			(neighbors[31] - neighbors[17]) * 7 +
			(neighbors[32] - neighbors[16]) * 8;
	
	int vLine = (neighbors[7] - neighbors[9]) * 1 +
			(neighbors[6] - neighbors[10]) * 2 +
			(neighbors[5] - neighbors[11]) * 3 +
			(neighbors[4] - neighbors[12]) * 4 +
			(neighbors[3] - neighbors[13]) * 5 +
			(neighbors[2] - neighbors[14]) * 6 +
			(neighbors[1] - neighbors[15]) * 7 +
			(neighbors[0] - neighbors[16]) * 8;

	int h = (5 * hLine + 32) >> 6;
	int v = (5 * vLine + 32) >> 6;

	int a = 16 * (neighbors[0] + neighbors[32] + 1) - 7 * (h + v);
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			int b = a + v * y + h * x;
			pred[x][y] = sat(b >> 5);
		}
	}
}

void IntraEncoder::xComputeIntraMode(int mode, Pel* neighbors, Pel** pred) {
	switch(mode) {
		case VER_MODE:
			xVerticalMode(neighbors, pred, BLOCK_SIZE);
			break;
		case HOR_MODE:
			xHorizonalMode(neighbors,  pred, BLOCK_SIZE);
			break;
		case DC_MODE:
			xDCMode(neighbors, pred, BLOCK_SIZE);
			break;
		case PLANE_MODE: //TODO
			xPlaneMode(neighbors, pred);
			break;
	}
}

void IntraEncoder::xComputeSubIntraMode(int mode, Pel* neighbors, Pel** pred) {

	switch(mode) {
		case VER_SMODE:
			xVerticalMode(neighbors, pred, SUB_BLOCK_SIZE);
			break;
		case HOR_SMODE:
			xHorizonalMode(neighbors, pred, SUB_BLOCK_SIZE);
			break;
		case DC_SMODE:
			xDCMode(neighbors, pred, SUB_BLOCK_SIZE);
			break;
		case DL_SMODE:
			xDLMode(neighbors, pred);
			break;
		case DR_SMODE:
			xDRMode(neighbors, pred);
			break;
		case VR_SMODE:
			xVRMode(neighbors, pred);
			break;
		case HD_SMODE:
			xHDMode(neighbors, pred);
			break;
		case VL_SMODE:
			xVLMode(neighbors, pred);
			break;
		case HU_SMODE:
			xHUMode(neighbors, pred);
			break;
	}
}

void IntraEncoder::xCopyBlock(Pel** blk0, Pel** blk1, int size) {
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			blk0[x][y] = blk1[x][y];
		}
	}
}

void IntraEncoder::xCalcResidue(Pel** block, Pel** blockPred, Pel** blockResidue, int size) {
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			blockResidue[x][y] = block[x][y] - blockPred[x][y];
		}
	}

}

void IntraEncoder::encode() {

	/*loop over the views*/
	for (int v = 0; v < this->vh->getNumOfViews(); v++) {
		/*loop over the GOPs*/
		for (int g = 0; g< this->vh->getNumOfGOP(); g++) {
			/*loop over the frames*/
			for (int f = (g==0) ? 0:1 ; f < GOP_SIZE+1; f++) {
				int vo = viewOrder[v];
				int fo = frameGOPOrder[f] + GOP_SIZE*g;

				cout << vo << " " << fo << endl;
				/*loop over the blocks*/
				for (int y = 0; y < this->vh->getHeight(); y+=BLOCK_SIZE) {
					for (int x = 0; x < this->vh->getWidth(); x+=BLOCK_SIZE) {
						Pel **blockResidue, **blockPred;
						blockResidue = new Pel*[BLOCK_SIZE];
						blockPred = new Pel*[BLOCK_SIZE];
						for (int i = 0; i < BLOCK_SIZE; i++) {
							blockResidue[i] = new Pel[BLOCK_SIZE];
							blockPred[i] = new Pel[BLOCK_SIZE];
						}

						/* tracing file reading */
						char blockType;

						/*I4*/
						vector<int> subModes;
						unsigned int subCost;

						this->traceFile >> blockType >> subCost;
						for(int i=0; i<16; i++)	{
							int subMode;
							this->traceFile >> subMode;
							subModes.push_back(subMode);
						}

						/*I16*/
						int mode;
						unsigned int cost;

						this->traceFile >> blockType >> cost >> mode;
						
						if( this->mode == I16_ONLY || (this->mode == I4_I16 && cost < subCost)) {
							Pel **block = vh->getBlock(vo, fo, x, y);
							Pel *neighbor = vh->getNeighboring(vo, fo, x ,y);
							xComputeIntraMode(mode, neighbor, blockPred);
							xCalcResidue(block, blockPred, blockResidue, BLOCK_SIZE);

							this->blockChoices ++;
							blockType = 'B';
							
						}
						else { /* (this->mode == I4_ONLY || (this->mode == I4_I16 && cost > subCost)) */
							Pel **subBlockResidue, **subBlockPred;
							subBlockResidue = new Pel*[SUB_BLOCK_SIZE];
							subBlockPred = new Pel*[SUB_BLOCK_SIZE];
							this->subBlockChoices ++;
							
							for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
								subBlockResidue[i] = new Pel[SUB_BLOCK_SIZE];
								subBlockPred[i] = new Pel[SUB_BLOCK_SIZE];
							}

							/* ZZ ORDER: TODO refactor it*/
							for(int i=0; i<16; i++)	{
								int xx = zzBlockOrder[i][0] * SUB_BLOCK_SIZE;
								int yy = zzBlockOrder[i][1] * SUB_BLOCK_SIZE;

								Pel **block = vh->getSubBlock(vo, fo, x+xx, y+yy);
								Pel *neighbor = vh->getSubNeighboring(vo, fo, x+xx, y+yy);

								xComputeSubIntraMode(mode, neighbor, subBlockPred);
								xCalcResidue(block,subBlockPred, subBlockResidue, SUB_BLOCK_SIZE);
								xCopySubBlock(blockResidue,subBlockResidue, SUB_BLOCK_SIZE, xx, yy);
							}

							blockType = 'S';
						}
						
						list<char> compressed = this->huffRes->encodeBlock(blockResidue);

						this->compressedBitCount += compressed.size() + ((blockType == 'B') ? MODE_BIT_WIDTH : 16*SMODE_BIT_WIDTH);
						this->uncompressedBitCount += BLOCK_SIZE * BLOCK_SIZE * SAMPLE_BIT_WIDTH;
						
						
						/*write back the residual information*/
						vh->insertResidualBlock(blockResidue, x, y, (blockType == 'B') ? BLOCK_MODE : SUB_BLOCK_MODE);
						subModes.clear();

					}
				}
				/*write the residual information of the frame in the file*/
				vh->writeResidualFrameInFile();
			}
		}
	}
	vh->closeFiles();
}

void IntraEncoder::xCopySubBlock(Pel** blk0, Pel** blk1, int size, int x, int y) {
	for (int yy = 0; yy < size; yy++) {
		for (int xx = 0; xx < size; xx++) {
			blk0[x + xx][y + yy] = blk1[xx][yy];
		}
	}
}

void IntraEncoder::report() {
	double blockPctg = this->blockChoices / (double) (this->blockChoices + this->subBlockChoices);
	double subBlockPctg = 1 - blockPctg;

	double losslessSavings = this->compressedBitCount / (double) (this->uncompressedBitCount);
	losslessSavings = 1 - losslessSavings;

	double bitsPerBlock = this->compressedBitCount / (double)(this->vh->getWidth() * this->vh->getHeight() * this->vh->getNumOfGOP() * this->vh->getNumOfViews() / (BLOCK_SIZE*BLOCK_SIZE) );
	

	cout << "BLOCK CHOICES:\t" << blockPctg << endl;
	cout << "SBLOCK CHOICES:\t" << subBlockPctg << endl;


	cout << "UNCOMPRESS BW:\t" << this->uncompressedBitCount / (BLOCK_SIZE*BLOCK_SIZE) << endl;
	cout << "COMPRESSED BW:\t" << this->compressedBitCount / (BLOCK_SIZE*BLOCK_SIZE) << endl;

	cout << "LOSSLESS SAV:\t" << losslessSavings << endl;
	cout << "AV. BIT/BLOCK:\t" << bitsPerBlock << endl;

}

void IntraEncoder::reportCSV() {
	double blockPctg = this->blockChoices / (double) (this->blockChoices + this->subBlockChoices);
	double subBlockPctg = 1 - blockPctg;

	double losslessSavings = this->compressedBitCount / (double) (this->uncompressedBitCount);
	losslessSavings = 1 - losslessSavings;

	double bitsPerBlock = this->compressedBitCount / (double)(this->vh->getWidth() * this->vh->getHeight() * this->vh->getNumOfGOP() * this->vh->getNumOfViews() / (BLOCK_SIZE*BLOCK_SIZE) );

	cout << vh->getVideoName() << ";";
	cout << ((this->mode == I4_ONLY) ? "I4_ONLY" : (this->mode == I16_ONLY) ? "I16_ONLY" : "I4_I16") << ";";
	cout << blockPctg << ";";
	cout << subBlockPctg << ";";
	cout << losslessSavings << ";";
	cout << bitsPerBlock << endl;;

}

void IntraEncoder::xReportStatus(int xx, int yy, int mode, Pel* neighbor, Pel** block, Pel** subBlockPred) {
	cout << xx << " " << yy << " " << mode << endl;
	for (int i = SUB_BLOCK_SIZE; i < SUB_BLOCK_SIZE*3 + 1; i++) {
		unsigned char n = neighbor[i];
		cout << (unsigned int)n << " ";
	}
	cout << endl;
	for (int i = SUB_BLOCK_SIZE-1; i >= 0 ; i--) {
		unsigned char n = neighbor[i];
		cout << (unsigned int)n<< endl;
	}
	/*
	for (int y = 0; y < SUB_BLOCK_SIZE; y++) {
		for (int x = 0; x < SUB_BLOCK_SIZE; x++) {
			cout << (int)block[x][y] << " ";
		}
		cout << endl;
	}*/
	cout << endl;

	for (int y = 0; y < SUB_BLOCK_SIZE; y++) {
		for (int x = 0; x < SUB_BLOCK_SIZE; x++) {
			unsigned char n = subBlockPred[x][y];
			cout << (unsigned int) n << " ";
		}
		cout << endl;
	}
	
}