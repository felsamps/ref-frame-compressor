#ifndef _INTRAENCODER_H
#define	_INTRAENCODER_H

#include <string>
#include <cmath>
#include "VideoHandler.h"



using namespace std;

class IntraEncoder {
private:
    VideoHandler* vh;
    long long int blockChoices, subBlockChoices;

    int xCalcSAD(Pel** blk0, Pel** blk1);

    int xComputeIntraMode(int mode, Pel* neighbors, Pel** recon, Pel** residue, int size);
    int xHorizonalMode(Pel* neighbors, Pel** recon, Pel** residue, int size);
    int xVerticalMode(Pel* neighbors, Pel** recon, Pel** residue, int size);
    int xDiagonalMode(Pel* neighbors, Pel** recon, Pel** residue, int size);
    int xDCMode(Pel* neighbors, Pel** recon, Pel** residue, int size);

    void xCopyBlock(Pel** blk0, Pel** blk1, int size);
    void xCopySubBlock(Pel** blk0, Pel** blk1, int size, int x, int y);

    pair<IntraMode, int> xEncodeBlock(int v, int f, int x, int y, Pel** residue);
    pair<vector<IntraMode>, int> xEncodeSubBlock(int v, int f, int x, int y, Pel** residue);

public:
    IntraEncoder(VideoHandler* vh);

    void encode();

    void report();

};

#endif	/* _INTRAENCODER_H */

