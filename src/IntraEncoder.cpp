#include "../inc/IntraEncoder.h"

int abs(int a) {
	return (a<=0) ? -a : a;
}


IntraEncoder::IntraEncoder(VideoHandler* vh) {
	this->vh = vh;
	this->blockChoices = 0;
	this->subBlockChoices = 0;
}

int IntraEncoder::xHorizonalMode(Pel* neighbors, Pel** recon, Pel** residue, int size) {
	int acumSad = 0;
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			Pel predSample = neighbors[(size-1) - y];
			residue[x][y] = recon[x][y] - predSample;
			acumSad += abs(residue[x][y]);
		}
	}
	return acumSad;
}

int IntraEncoder::xVerticalMode(Pel* neighbors, Pel** recon, Pel** residue, int size) {
	int acumSad = 0;
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			Pel predSample = neighbors[(size+1) + x];
			residue[x][y] = recon[x][y] - predSample;
			acumSad += abs(residue[x][y]);
		}
	}
	return acumSad;
}

int IntraEncoder::xDCMode(Pel* neighbors, Pel** recon, Pel** residue, int size) {
	int predSample = 0;
	int validNeigh = 0;
	for (int i = 0; i < 2*size + 1; i++) {
		if(neighbors[i] != -1) {
			predSample += neighbors[i];
			validNeigh ++;
		}
	}
	predSample /= validNeigh;

	int acumSad = 0;
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			residue[x][y] = recon[x][y] - predSample;
			acumSad += abs(residue[x][y]);
		}
	}
	return acumSad;

}

int IntraEncoder::xComputeIntraMode(int mode, Pel* neighbors, Pel** recon, Pel** residue, int size) {
	int sad = 0x7FFFFFFF;
	switch(mode) {
		case HOR_MODE:
			sad = (neighbors[0] != -1) ? xHorizonalMode(neighbors, recon, residue, size) : sad;
			break;
		case VER_MODE:
			sad = (neighbors[size+1] != -1) ? xVerticalMode(neighbors, recon, residue, size) : sad;
			break;
		/*case DIAG_MODE: TODO
			sad = xDiagonalMode(neighbors, recon, residue);
			break;*/
		case DC_MODE:
			sad = (neighbors[0] != -1 && neighbors[size+1] != -1) ? xDCMode(neighbors, recon, residue, size) : sad;
			break;
	}
	return sad;
}

void IntraEncoder::xCopyBlock(Pel** blk0, Pel** blk1, int size) {
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			blk0[x][y] = blk1[x][y];
		}
	}
}

pair<IntraMode, int> IntraEncoder::xEncodeBlock(int v, int f, int x, int y, Pel** residue) {
	/* INTRA FOR BLOCKS */
	Pel* neighbors = vh->getNeighboring(v, f, x, y);
	Pel** block = vh->getBlock(v, f, x, y);

	Pel **tempRes;
	int tempSad, bestSad = 0x7FFFFFFF;
	IntraMode bestMode;
	
	tempRes = new Pel*[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++) {
		tempRes[i] = new Pel[BLOCK_SIZE];
	}
	
	for (int m = 0; m < 4; m++) {
		tempSad = xComputeIntraMode(m, neighbors, block, tempRes, BLOCK_SIZE);

		if( tempSad < bestSad ) {
			bestMode = (IntraMode)m;
			bestSad = tempSad;
			this->xCopyBlock(residue, tempRes, BLOCK_SIZE);
		}
	}
	pair<IntraMode, int> returnable(bestMode, bestSad);
	return returnable;
}

pair<vector<IntraMode>, int> IntraEncoder::xEncodeSubBlock(int v, int f, int x, int y, Pel** residue) {
	/* INTRA FOR SUB-BLOCKS */
	Pel **tempSubRes;
	int tempSubSad, bestSubSad = 0x7FFFFFFF, acumSubSad = 0;
	IntraMode bestSubMode;
	vector<IntraMode> modes;
	


	tempSubRes = new Pel*[SUB_BLOCK_SIZE];
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		tempSubRes[i] = new Pel[SUB_BLOCK_SIZE];
	}


	for (int yy = 0; yy < BLOCK_SIZE; yy+=SUB_BLOCK_SIZE) {
		for (int xx = 0; xx < BLOCK_SIZE; xx+=SUB_BLOCK_SIZE) {
			for (int m = 0; m < 4; m++) {
				Pel **block = vh->getSubBlock(v, f, x+xx, y+yy);
				Pel *subNeighbors = vh->getSubNeighboring(v, f, x+xx, y+yy);
				tempSubSad = xComputeIntraMode(m, subNeighbors, block, tempSubRes, SUB_BLOCK_SIZE);

				if( tempSubSad < bestSubSad ) {
					bestSubMode = (IntraMode)m;
					bestSubSad = tempSubSad;
					xCopySubBlock(residue, tempSubRes, SUB_BLOCK_SIZE, xx, yy);
				}
			}
			modes.push_back(bestSubMode);
			acumSubSad += bestSubSad;
		}
	}

	pair<vector<IntraMode>, int> returnable(modes,acumSubSad);
	return returnable;
}

void IntraEncoder::encode() {

	/*loop over the views*/
	for (int v = 0; v < this->vh->getNumOfViews(); v++) {
		/*loop over the frames*/
		for (int f = 0; f < this->vh->getNumOfFrames(); f++) {
			/*loop over the blocks*/
			for (int y = 0; y < this->vh->getHeight(); y+=BLOCK_SIZE) {
				for (int x = 0; x < this->vh->getWidth(); x+=BLOCK_SIZE) {
					Pel **blockResidue, **subBlockResidue;

					blockResidue = new Pel*[BLOCK_SIZE];
					subBlockResidue = new Pel*[BLOCK_SIZE];
					for (int i = 0; i < BLOCK_SIZE; i++) {
						blockResidue[i] = new Pel[BLOCK_SIZE];
						subBlockResidue[i] = new Pel[BLOCK_SIZE];
					}

					pair<IntraMode,int> pBlock;
					pBlock = this->xEncodeBlock(v, f, x, y, blockResidue);

					#if EN_SUB_BLOCK
					pair<vector<IntraMode>, int> pSubBlock;
					pSubBlock = this->xEncodeSubBlock(v, f, x, y, subBlockResidue);
					
					Pel **res;
					bool mode;
					int blockSad = pBlock.second;
					int subBlockSad = pSubBlock.second;
					
					if(abs(subBlockSad - blockSad) < 600) {
						res = blockResidue;
						this->blockChoices ++;
						mode = BLOCK_MODE;
					}
					else {
						res = subBlockResidue;
						this->subBlockChoices ++;
						mode = SUB_BLOCK_MODE;
					}
					#else

					Pel** res = blockResidue;
					bool mode = BLOCK_MODE;
					this->blockChoices ++;
					
					#endif
					/*write back the residual information*/
					if( x!=0 || y!=0 ) {
						vh->insertResidualBlock(res, x, y, mode);
					}
									
				}
			}
			/*write the residual information of the frame in the file*/
			vh->writeResidualFrameInFile();
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

	cout << "BLOCK CHOICES:\t" << blockPctg << endl;
	cout << "SBLOCK CHOICES:\t" << subBlockPctg << endl;

}