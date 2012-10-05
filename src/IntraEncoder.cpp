#include "../inc/IntraEncoder.h"

int abs(int a) {
	return (a<=0) ? -a : a;
}


IntraEncoder::IntraEncoder(VideoHandler* vh) {
	this->vh = vh;
}

int IntraEncoder::xHorizonalMode(Pel* neighbors, Pel** recon, Pel** residue) {
	int acumSad = 0;
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			Pel predSample = neighbors[(BLOCK_SIZE-1) - y];
			residue[x][y] = recon[x][y] - predSample;
			acumSad += abs(residue[x][y]);
		}
	}
	return acumSad;
}

int IntraEncoder::xVerticalMode(Pel* neighbors, Pel** recon, Pel** residue) {
	int acumSad = 0;
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			Pel predSample = neighbors[(BLOCK_SIZE+1) + x];
			residue[x][y] = recon[x][y] - predSample;
			acumSad += abs(residue[x][y]);
		}
	}
	return acumSad;
}

int IntraEncoder::xDCMode(Pel* neighbors, Pel** recon, Pel** residue) {
	int predSample = 0;
	int validNeigh = 0;
	for (int i = 0; i < 2*BLOCK_SIZE + 1; i++) {
		if(neighbors[i] != -1) {
			predSample += neighbors[i];
			validNeigh ++;
		}
	}
	predSample /= validNeigh;

	int acumSad = 0;
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			residue[x][y] = recon[x][y] - predSample;
			acumSad += abs(residue[x][y]);
		}
	}
	return acumSad;

}

int IntraEncoder::xComputeIntraMode(int mode, Pel* neighbors, Pel** recon, Pel** residue) {
	int sad = 0x7FFFFFFF;
	switch(mode) {
		case 0:
			sad = (neighbors[0] != -1) ? xHorizonalMode(neighbors, recon, residue) : sad;
			break;
		case 1:
			sad = (neighbors[BLOCK_SIZE+1] != -1) ? xVerticalMode(neighbors, recon, residue) : sad;
			break;
		/*case 2: TODO
			sad = xDiagonalMode(neighbors, recon, residue);
			break;*/
		case 3:
			sad = (neighbors[0] != -1 && neighbors[BLOCK_SIZE+1] != -1) ? xDCMode(neighbors, recon, residue) : sad;
			break;
	}
	return sad;
}

void IntraEncoder::xCopyBlock(Pel** blk0, Pel** blk1) {
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			blk0[x][y] = blk1[x][y];
		}
	}
}

void IntraEncoder::encode() {

	/*loop over the views*/
	for (int v = 0; v < this->vh->getNumOfViews(); v++) {
		/*loop over the frames*/
		for (int f = 0; f < this->vh->getNumOfFrames(); f++) {
			/*loop over the blocks*/
			for (int y = 0; y < this->vh->getHeight(); y+=BLOCK_SIZE) {
				for (int x = 0; x < this->vh->getWidth(); x+=BLOCK_SIZE) {
					Pel* neighbors = vh->getNeighboring(v, f, x, y);
					Pel** block = vh->getBlock(v, f, x, y);

					Pel **tempRes, **bestRes;
					int tempSad, bestSad = 0x7FFFFFFF;
					int bestMode;

					tempRes = new Pel*[BLOCK_SIZE];
					bestRes = new Pel*[BLOCK_SIZE];
					for (int i = 0; i < BLOCK_SIZE; i++) {
						tempRes[i] = new Pel[BLOCK_SIZE];
						bestRes[i] = new Pel[BLOCK_SIZE];
					}
					
					/*compute SAD modes*/
					/*choose the best*/
					/*residue calculation*/
					
					for (int m = 0; m < 4; m++) {
						tempSad = xComputeIntraMode(m, neighbors, block, tempRes);

						if( tempSad < bestSad ) {
							bestMode = m;
							bestSad = tempSad;
							xCopyBlock(bestRes, tempRes);
						}
					}
					
					/*write back the residual information*/

					if( x==0 && y==0 ) {
						vh->insertResidualBlock(block, x, y);
					}
					else {
						vh->insertResidualBlock(bestRes, x, y);
					}
					
										
				}
			}
			/*write the residual information of the frame in the file*/
			vh->writeResidualFrameInFile();
		}
	}
	vh->closeFiles();

}