#ifndef _INTRAENCODER_H
#define	_INTRAENCODER_H

#include <string>
#include <cmath>
#include "VideoHandler.h"



using namespace std;

class IntraEncoder {
private:
    VideoHandler* vh;

    int xCalcSAD(Pel** blk0, Pel** blk1);
    int xComputeIntraMode(int mode, Pel* neighbors, Pel** recon, Pel** residue);
    int xHorizonalMode(Pel* neighbors, Pel** recon, Pel** residue);
    int xVerticalMode(Pel* neighbors, Pel** recon, Pel** residue);
    int xDiagonalMode(Pel* neighbors, Pel** recon, Pel** residue);
    int xDCMode(Pel* neighbors, Pel** recon, Pel** residue);
    void xCopyBlock(Pel** blk0, Pel** blk1);

public:
    IntraEncoder(VideoHandler* vh);

    void encode();

};

#endif	/* _INTRAENCODER_H */

