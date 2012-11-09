#ifndef _INTRAENCODER_H
#define	_INTRAENCODER_H

#include <string>
#include <cmath>
#include <fstream>

#include "VideoHandler.h"
#include "Huffman.h"
#include "Quantizer.h"

using namespace std;

class IntraEncoder {
private:
    VideoHandler* vh;
    Huffman* huffRes;
    Quantizer* quant;
    fstream traceFile;
    int mode;
    
    long long int blockChoices, subBlockChoices;
    long long int compressedBitCount, uncompressedBitCount;


    int xCalcSAD(Pel** blk0, Pel** blk1);

    void xComputeIntraMode(int mode, Pel* neighbors, Pel** pred);
    void xComputeSubIntraMode(int mode, Pel* neighbors,  Pel** pred);
    void xHorizonalMode(Pel* neighbors, Pel** pred, int size);
    void xVerticalMode(Pel* neighbors, Pel** pred, int size);
    void xDiagonalMode(Pel* neighbors, Pel** pred, int size);
    void xDCMode(Pel* neighbors, Pel** pred, int size);
    void xPlaneMode(Pel* neighbors, Pel** pred);
    void xDLMode(Pel* neighbors, Pel** pred);
    void xDRMode(Pel* neighbors, Pel** pred);
    void xVRMode(Pel* neighbors, Pel** pred);
    void xHDMode(Pel* neighbors, Pel** pred);
    void xVLMode(Pel* neighbors, Pel** pred);
    void xHUMode(Pel* neighbors, Pel** pred);

    void xCopyBlock(Pel** blk0, Pel** blk1, int size);
    void xCopySubBlock(Pel** blk0, Pel** blk1, int size, int x, int y);

    void xCalcResidue(Pel **block, Pel **blockPred, Pel **blockResidue, int size);

    pair<IntraMode, int> xEncodeBlock(int v, int f, int x, int y, Pel** residue);
    pair<vector<IntraMode>, int> xEncodeSubBlock(int v, int f, int x, int y, Pel** residue);

    void xReportStatus(int xx, int yy, int mode, Pel* neighbor, Pel** block, Pel** subBlockPred);

public:
    IntraEncoder(int mode, VideoHandler* vh, Huffman* huffRes, Quantizer* quant, string name);

    void encode();

    void report();
    void reportCSV();

};

#endif	/* _INTRAENCODER_H */

