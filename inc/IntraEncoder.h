#ifndef _INTRAENCODER_H
#define	_INTRAENCODER_H

#include <string>
#include <cmath>
#include <fstream>

#include "VideoHandler.h"
#include "Huffman.h"
#include "Quantizer.h"
#include "Statistics.h"

using namespace std;

class IntraEncoder {
private:
    VideoHandler* vh;
    Huffman *huffRes, *huffRes0, *huffRes1, *huffRes2;
    Quantizer *quant, *quant0, *quant1, *quant2;
    fstream traceFile;
    fstream costFile;
    int mode;
    int opMode;
    Statistics* stats;
    
    long long int blockChoices, subBlockChoices;
    long long int compressedBitCount, uncompressedBitCount;

    void xInitCounters();

    int xCalcSAD(Pel** blk0, Pel** blk1);

    void xComputeIntraMode(int mode, UPel* neighbors, int** pred);
    void xComputeI4IntraMode(int mode, UPel* neighbors,  int** pred);
    void xHorizonalMode(UPel* neighbors, int** pred, int size);
    void xVerticalMode(UPel* neighbors, int** pred, int size);
    void xDiagonalMode(UPel* neighbors, int** pred, int size);
    void xDCMode(UPel* neighbors, int** pred, int size);
    void xPlaneMode(UPel* neighbors, int** pred);
    void xDLMode(UPel* neighbors, int** pred);
    void xDRMode(UPel* neighbors, int** pred);
    void xVRMode(UPel* neighbors, int** pred);
    void xHDMode(UPel* neighbors, int** pred);
    void xVLMode(UPel* neighbors, int** pred);
    void xHUMode(UPel* neighbors, int** pred);

    void xCopyBlock(Pel** blk0, Pel** blk1, int size);
    void xCopyPelSubBlock(Pel** blk0, Pel** blk1, int size, int x, int y);
    void xCopyIntSubBlock(int** blk0, int** blk1, int size, int x, int y);
    void xFillZero(Pel** blk, int size,  int xx, int yy);

    UPel** xReconstructBlock(int** pred, Pel** res);

    void xCalcResidue(UPel **block, int **blockPred, Pel **blockResidue, int size);

    pair<IntraMode, int> xEncodeBlock(int v, int f, int x, int y, Pel** residue);
    pair<vector<IntraMode>, int> xEncodeSubBlock(int v, int f, int x, int y, Pel** residue);

    void xReportStatus(int xx, int yy, int mode, UPel* neighbor, UPel** block, int** macrolockPred, Pel** macrolockResidue);
   

public:
    
    IntraEncoder(int opMode, VideoHandler* vh, string name); //MODE 0
    IntraEncoder(int opMode, VideoHandler* vh, string name, Huffman* huffRes); //MODE 1
    IntraEncoder(int opMode, int mode, VideoHandler* vh, string name, Quantizer* quant); //MODE 2
    IntraEncoder(int opMode, int mode, VideoHandler* vh, string name, Huffman* huffRes, Quantizer* quant); //MODE 3
    IntraEncoder(int opMode,
                 int mode,
                 VideoHandler* vh,
                 string name,
                 Huffman* huffRes,
                 Huffman* huffRes16,
                 Huffman* huffRes32,
                 Huffman* huffRes64,
                 Quantizer* quant16,
                 Quantizer* quant32,
                 Quantizer* quant64,
                 string costFileName); //MODE 4
    

    void encode();

    void report();
    void reportCSV(); 
    void reportOcc();

};

#endif	/* _INTRAENCODER_H */

