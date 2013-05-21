#include <list>

#include "../inc/IntraEncoder.h"
#include "../inc/Statistics.h"

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

IntraEncoder::IntraEncoder(int opMode, VideoHandler* vh, string name) {
	this->opMode = opMode;
	this->vh = vh;
	this->traceFile.open(name.c_str(), fstream::in);
	xInitCounters();	
}

IntraEncoder::IntraEncoder(int opMode, VideoHandler* vh, string name, Huffman* huffRes) {
	this->opMode = opMode;
	this->vh = vh;
	this->traceFile.open(name.c_str(), fstream::in);
	this->huffRes = huffRes;
	xInitCounters();
}

IntraEncoder::IntraEncoder(int opMode, VideoHandler* vh, string name, Quantizer* quant) {
	this->opMode = opMode;
	this->vh = vh;
	this->traceFile.open(name.c_str(), fstream::in);
	this->quant = quant;
	xInitCounters();
}

IntraEncoder::IntraEncoder(int opMode, VideoHandler* vh, string name, Huffman* huffRes, Quantizer* quant) {
	this->opMode = opMode;
	this->vh = vh;
	this->traceFile.open(name.c_str(), fstream::in);
	this->huffRes = huffRes;
	this->quant = quant;
	xInitCounters();
}

IntraEncoder::IntraEncoder(int opMode, VideoHandler* vh, string name, Huffman* huffRes, Huffman* huffRes16, Huffman* huffRes32, Huffman* huffRes64, Quantizer* quant16, Quantizer* quant32, Quantizer* quant64) {
	this->opMode = opMode;
	this->vh = vh;
	this->traceFile.open(name.c_str(), fstream::in);
	this->huffRes = huffRes;
	this->huffRes0 = huffRes16;
	this->huffRes1 = huffRes32;
	this->huffRes2 = huffRes64;

	this->quant0 = quant16;
	this->quant1 = quant32;
	this->quant2 = quant64;
	
	xInitCounters();
}


void IntraEncoder::xInitCounters() {
	this->compressedBitCount = 0;
	this->uncompressedBitCount = 0;
	this->blockChoices = 0;
	this->subBlockChoices = 0;
	this->stats = new Statistics();
}

void IntraEncoder::xHorizonalMode(UPel* neighbors, int** pred, int size) {
	bool isLeftRef = (neighbors[0] != 255); //TODO fix this
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			if(isLeftRef) {
				int predSample = neighbors[(size-1) - y];
				pred[x][y] = predSample;
			}
		}
	}
}

void IntraEncoder::xVerticalMode(UPel* neighbors, int** pred, int size) {
	bool isUpRef = (neighbors[size+1] != 255);
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			if(isUpRef) {
				int predSample = neighbors[(size+1) + x];
				pred[x][y] = predSample;
			}
		}
	}
}

void IntraEncoder::xDCMode(UPel* neighbors, int** pred, int size) {
	unsigned int predSample = 0;
	int validNeigh = 0;

	for (int i = 0; i < 2*size + 1; i++) {
		if(neighbors[i] != 255) {
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

void IntraEncoder::xDRMode(UPel* neighbors, int** pred) {
	pred[3][0] = (neighbors[6] + neighbors[7]*2 + neighbors[8] + 2) >> 2;
	pred[2][0] = pred[3][1] = (neighbors[5] + neighbors[6]*2 + neighbors[7] + 2) >> 2;
	pred[1][0] = pred[2][1] = pred[3][2] = (neighbors[4] + neighbors[5]*2 + neighbors[6] + 2) >> 2;
	pred[0][0] = pred[1][1] = pred[2][2] = pred[3][3] = (neighbors[3] + neighbors[4]*2 + neighbors[5] + 2) >> 2;
				 pred[0][1] = pred[1][2] = pred[2][3] = (neighbors[2] + neighbors[3]*2 + neighbors[4] + 2) >> 2;
							  pred[0][2] = pred[1][3] = (neighbors[1] + neighbors[2]*2 + neighbors[3] + 2) >> 2;
										   pred[0][3] = (neighbors[0] + neighbors[1]*2 + neighbors[2] + 2) >> 2;

}

void IntraEncoder::xDLMode(UPel* neighbors, int** pred) {

	if(neighbors[9] == 255 && neighbors[10] == 255 && neighbors[11] == 255 && neighbors[12] == 255 ) {
		neighbors[9] = neighbors[8];
		neighbors[10] = neighbors[8];
		neighbors[11] = neighbors[8];
		neighbors[12] = neighbors[8];		
	}

	pred[0][0]										  = (neighbors[5] + neighbors[6]*2 + neighbors[7] + 2) >> 2;
	pred[1][0] = pred[0][1]							  = (neighbors[6] + neighbors[7]*2 + neighbors[8] + 2) >> 2;
	pred[2][0] = pred[1][1] = pred[0][2]			  = (neighbors[7] + neighbors[8]*2 + neighbors[9] + 2) >> 2;
	pred[3][0] = pred[2][1] = pred[1][2] = pred[0][3] = (neighbors[8] + neighbors[9]*2 + neighbors[10] + 2) >> 2;
				 pred[3][1] = pred[2][2] = pred[1][3] = (neighbors[9] + neighbors[10]*2 + neighbors[11] + 2) >> 2;
							  pred[3][2] = pred[2][3] = (neighbors[10] + neighbors[11]*2 + neighbors[12] + 2) >> 2;
										   pred[3][3] = (neighbors[11] + neighbors[12]*2 + neighbors[12] + 2) >> 2;

	
	
}

void IntraEncoder::xVRMode(UPel* neighbors, int** pred) {
	pred[0][0] = pred[1][2] = (neighbors[5] + neighbors[4] + 1) >> 1;
	pred[1][0] = pred[2][2] = (neighbors[6] + neighbors[5] + 1) >> 1;
	pred[2][0] = pred[3][2] = (neighbors[7] + neighbors[6] + 1) >> 1;
	pred[3][0] = (neighbors[8] + neighbors[7] + 1) >> 1;
	
	pred[0][1] = pred[1][3] = (neighbors[3] + neighbors[4]*2 + neighbors[5] + 2) >> 2;
	pred[1][1] = pred[2][3] = (neighbors[4] + neighbors[5]*2 + neighbors[6] + 2) >> 2;
	pred[2][1] = pred[3][3] = (neighbors[5] + neighbors[6]*2 + neighbors[7] + 2) >> 2;
	pred[3][1] =              (neighbors[6] + neighbors[7]*2 + neighbors[8] + 2) >> 2;

	pred[0][2] = (neighbors[2] + neighbors[3]*2 + neighbors[4] + 2) >> 2;
	pred[0][3] = (neighbors[1] + neighbors[2]*2 + neighbors[3] + 2) >> 2;
}

void IntraEncoder::xHDMode(UPel* neighbors, int** pred) {
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

void IntraEncoder::xVLMode(UPel* neighbors, int** pred) {

	if(neighbors[9] == 255 && neighbors[10] == 255 && neighbors[11] == 255 && neighbors[12] == 255 ) {
		neighbors[9] = neighbors[8];
		neighbors[10] = neighbors[8];
		neighbors[11] = neighbors[8];
		neighbors[12] = neighbors[8];
	}
	
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

void IntraEncoder::xHUMode(UPel* neighbors, int** pred) {
	pred[0][0] = (neighbors[3] + neighbors[2] + 1) >> 1;
	pred[1][0] = (neighbors[3] + neighbors[2]*2 + neighbors[1] + 2) >> 2;
	pred[2][0] = pred[0][1] = (neighbors[2] + neighbors[1] + 1) >> 1;
	pred[3][0] = pred[1][1] = (neighbors[2] + neighbors[1]*2 + neighbors[0] + 2) >> 2;

	pred[2][1] = pred[0][2] = (neighbors[1] + neighbors[0] + 1) >> 1;
	pred[3][1] = pred[1][2] = (neighbors[1] + neighbors[0]*2 + neighbors[0] + 2) >> 2;
	
	pred[2][2] = pred[3][2] = pred[0][3] = pred[2][3] = pred[1][3] = pred[3][3] = neighbors[0];

}

void IntraEncoder::xPlaneMode(UPel* neighbors, int** pred) {
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

void IntraEncoder::xComputeIntraMode(int mode, UPel* neighbors, int** pred) {
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

void IntraEncoder::xComputeI4IntraMode(int mode, UPel* neighbors, int** pred) {

	switch(mode) {
		case VER_SMODE:
			xVerticalMode(neighbors, pred, 4);
			break;
		case HOR_SMODE:
			xHorizonalMode(neighbors, pred, 4);
			break;
		case DC_SMODE:
			xDCMode(neighbors, pred, 4);
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

void IntraEncoder::xCalcResidue(UPel** block, int** blockPred, Pel** blockResidue, int size) {
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			blockResidue[x][y] = block[x][y] - (Pel)blockPred[x][y];
		}
	}
}

void IntraEncoder::xFillZero(Pel** blk, int size, int xx, int yy) {
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			blk[xx + x][yy + y] = 0;
		}
	}
}

UPel** IntraEncoder::xReconstructBlock(int** pred, Pel** res) {
	UPel** returnable = new UPel*[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++) {
		returnable[i] = new UPel[BLOCK_SIZE];
	}
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			returnable[x][y] = UPel((UPel)(pred[x][y]) + res[x][y]);
		}
	}
	return returnable;
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
				
				/*loop over the blocks*/
				Pel **macroblockResidue;
				int **macroblockPred;
				macroblockResidue = new Pel*[MACROBLOCK_SIZE];
				macroblockPred = new int*[MACROBLOCK_SIZE];
				for (int i = 0; i < MACROBLOCK_SIZE; i++) {
					macroblockResidue[i] = new Pel[MACROBLOCK_SIZE];
					macroblockPred[i] = new int[MACROBLOCK_SIZE];
				}
				for (int y = 0; y < this->vh->getHeight(); y+=MACROBLOCK_SIZE) {
					for (int x = 0; x < this->vh->getWidth(); x+=MACROBLOCK_SIZE) {
						
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
						
						Pel **blockResidue;
						int **blockPred;
						blockResidue = new Pel*[BLOCK_SIZE];
						blockPred = new int*[BLOCK_SIZE];

						for (int i = 0; i < BLOCK_SIZE; i++) {
							blockResidue[i] = new Pel[BLOCK_SIZE];
							blockPred[i] = new int[BLOCK_SIZE];
						}


						/* ZZ ORDER: TODO refactor it*/
						for(int i=0; i<16; i++)	{
							int xx = zzBlockOrder[i][0] * BLOCK_SIZE;
							int yy = zzBlockOrder[i][1] * BLOCK_SIZE;

							UPel **block = vh->getBlock(vo, fo, x+xx, y+yy);
							UPel *neighbor = vh->getBlockNeighboring(vo, fo, x+xx, y+yy);

							xComputeI4IntraMode(subModes[i], neighbor, blockPred);
							xCalcResidue(block,blockPred, blockResidue, BLOCK_SIZE);
							xCopyPelSubBlock(macroblockResidue,blockResidue, BLOCK_SIZE, xx, yy);
							xCopyIntSubBlock(macroblockPred, blockPred, BLOCK_SIZE, xx, yy);
							
							//xReportStatus(xx, yy, subModes[i], neighbor, block, macroblockPred, macroblockResidue);
							
						}
						
						
						this->subBlockChoices ++;						

						list<char> compressed;
						Pel** error, res0, res1, res2;
						UPel** recBlock;

						switch(this->opMode) {
							case 0:
								vh->insertResidualBlock(macroblockResidue, x, y, this->stats);
								/* TODO implement Huffman dictionary statiscs analysis */

								break;
							case 1:
								/* Huffman */
								
								vh->insertResidualBlock(macroblockResidue, x, y, this->stats);
								
								compressed = this->huffRes->encodeBlock(macroblockResidue);
								this->compressedBitCount += compressed.size() + 16*SMODE_BIT_WIDTH;
								
								//cout << compressed.size() << " ";
								
								this->uncompressedBitCount += MACROBLOCK_SIZE * MACROBLOCK_SIZE * SAMPLE_BIT_WIDTH;
								
								break;
								
							case 2:

								/* Quantization */
								error = this->quant->quantize(macroblockResidue, MACROBLOCK_SIZE, 0, 0);
								vh->insertErrorBlock(error, x, y);

								vh->insertResidualBlock(macroblockResidue, x, y, this->stats);

								break;

							case 3:
								error = new Pel*[MACROBLOCK_SIZE];
								
								for (int i = 0; i < MACROBLOCK_SIZE; i++) {
									error[i] = new Pel[MACROBLOCK_SIZE];
								}
								
								int MSE_TH = 4;

								for(int i=0; i<16; i++)	{
									int xx = zzBlockOrder[i][0] * BLOCK_SIZE;
									int yy = zzBlockOrder[i][1] * BLOCK_SIZE;

									
									Pel** subError0 = quant0->getError(macroblockResidue, BLOCK_SIZE, xx, yy);
									Pel** subError1 = quant1->getError(macroblockResidue, BLOCK_SIZE, xx, yy);
									Pel** subError2 = quant2->getError(macroblockResidue, BLOCK_SIZE, xx, yy);
									
									double mse0 = xCalcMSE(subError0);
									double mse1 = xCalcMSE(subError1);
									double mse2 = xCalcMSE(subError2);
									
									/* In according with the THs, apply the rigth quantization level */
									Pel** subError = (mse0 <= MSE_TH) ? subError0 :
													 (mse1 <= MSE_TH) ? subError1 :
													 (mse2 <= MSE_TH) ? subError2 :
																		NULL; //lossless
									
									Quantizer *targetQuant = (mse0 <= MSE_TH) ? this->quant0 :
															 (mse1 <= MSE_TH) ? this->quant1 :
														     (mse2 <= MSE_TH) ? this->quant2 :
															 NULL; //lossless

									Huffman *targetHuff = (mse0 <= MSE_TH) ? this->huffRes0 :
														  (mse1 <= MSE_TH) ? this->huffRes1 :
														  (mse2 <= MSE_TH) ? this->huffRes2 :
														  this->huffRes; //lossless									

									if(targetQuant != NULL) {
										xCopyPelSubBlock(error, subError, BLOCK_SIZE, xx, yy);
										targetQuant->quantize(macroblockResidue, BLOCK_SIZE, xx, yy);
									}
									else {
										xFillZero(error, BLOCK_SIZE, xx, yy);
										
									}
									
									vh->insertResidualSubBlock(macroblockResidue, xx, yy, xx+x, yy+y, this->stats);

									/* Huffman */
									compressed = targetHuff->encodeSubBlock(macroblockResidue, xx, yy);
									this->compressedBitCount += (compressed.size() + (SMODE_BIT_WIDTH) + ADAPTIVE_QUANT_ID_BIT_WIDTH);

									if(targetQuant != NULL) {
										targetQuant->invQuantize(macroblockResidue, BLOCK_SIZE, xx, yy);
									}

								}
								this->uncompressedBitCount += MACROBLOCK_SIZE * MACROBLOCK_SIZE * SAMPLE_BIT_WIDTH;
								//cout << x << " " << y << " " << blockLen << endl;
								this->vh->insertErrorBlock(error, x, y);
								recBlock = this->xReconstructBlock(macroblockPred, macroblockResidue);
								this->vh->insertLossyReconBlock(recBlock, vo, fo, x, y);
								break;
						}

						subModes.clear();
					}
				}
				/*write the residual information of the frame in the file*/
				vh->writeResidualFrameInFile();
				if(opMode >= 2) {
					vh->writeErrorFrameInFile();
				}
			}
		}
	}
	/*if(opMode >= 2) {
		vh->writeLossyReconInFile();
	}*/
	vh->closeFiles();
}

double IntraEncoder::xCalcMSE(Pel** subError) {
	double returnable = 0;
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			returnable += subError[x][y];
		}
	}
	
	return returnable / (BLOCK_SIZE*BLOCK_SIZE);
}

void IntraEncoder::xCopyPelSubBlock(Pel** blk0, Pel** blk1, int size, int x, int y) {
	for (int yy = 0; yy < size; yy++) {
		for (int xx = 0; xx < size; xx++) {
			blk0[x + xx][y + yy] = blk1[xx][yy];
		}
	}
}

void IntraEncoder::xCopyIntSubBlock(int** blk0, int** blk1, int size, int x, int y) {
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


	cout << "UNCOMPRESS BW:\t" << this->uncompressedBitCount / (MACROBLOCK_SIZE*MACROBLOCK_SIZE) << endl;
	cout << "COMPRESSED BW:\t" << this->compressedBitCount / (MACROBLOCK_SIZE*MACROBLOCK_SIZE) << endl;

	cout << "LOSSLESS SAV:\t" << losslessSavings << endl;
	cout << "AV. BIT/BLOCK:\t" << bitsPerBlock << endl;

}

void IntraEncoder::reportCSV() {

	double losslessSavings = this->compressedBitCount / (double) (this->uncompressedBitCount);
	losslessSavings = 1 - losslessSavings;

	double bitsPerBlock = this->compressedBitCount / (double)(this->vh->getWidth() * this->vh->getHeight() * this->vh->getNumOfGOP() * this->vh->getNumOfViews() / (BLOCK_SIZE*BLOCK_SIZE) );

	if(opMode == 1 || opMode == 3) {
		cout << vh->getVideoName() << ";";
		
		cout << losslessSavings << ";";
		cout << bitsPerBlock << endl;
	}

}

void IntraEncoder::xReportStatus(int xx, int yy, int mode, UPel* neighbor, UPel** block, int** macrolockPred, Pel** macrolockResidue) {
	cout << xx << " " << yy << " " << mode << endl;
	for (int i = BLOCK_SIZE; i < BLOCK_SIZE*3 + 1; i++) {
		unsigned char n = neighbor[i];
		cout << (int)n << " ";
	}
	cout << endl;
	for (int i = BLOCK_SIZE-1; i >= 0 ; i--) {
		unsigned char n = neighbor[i];
		cout << (int)n << endl;
	}
	
	for (int y = 0; y < BLOCK_SIZE; y++) {
		for (int x = 0; x < BLOCK_SIZE; x++) {
			cout << (int)block[x][y] << " ";
		}
		cout << endl;
	}
	cout << endl;
	

	for (int y = 0; y < MACROBLOCK_SIZE; y++) {
		for (int x = 0; x < MACROBLOCK_SIZE; x++) {
			UPel n = macrolockPred[x][y];
			cout << (int)n << " ";
		}
		cout << endl;
	}
	
	for (int y = 0; y < MACROBLOCK_SIZE; y++) {
		for (int x = 0; x < MACROBLOCK_SIZE; x++) {
			Pel n = macrolockResidue[x][y];
			cout << (int)n << " ";
		}
		cout << endl;
	}
	getchar();
	
}

void IntraEncoder::reportOcc() {
	this->stats->report();
}

void IntraEncoder::reportStats() {
	pair<double,double> result = this->stats->calcStats();
	cout << result.first << " " << result.second << endl;
}