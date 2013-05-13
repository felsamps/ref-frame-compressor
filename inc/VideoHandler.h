#ifndef _VIDEOHANDLER_H
#define	_VIDEOHANDLER_H

#include "../inc/Defines.h"
#include "../inc/Statistics.h"

#include <string>
#include <vector>
#include <fstream>
#include <iosfwd>
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>
#include <map>

using namespace std;

class VideoHandler {
private:

    int w, h, nv, gops;
    int targetView, targetFrame;
    UPel* reconFrame;
    Pel* residualFrame;
    Pel* errorFrame;
    UPel*** lossyReconFrame;
    vector<string> fileNames;
    fstream reconFile, residualFile, errorFile, lossyReconFile;
    string videoName;

    void xInitFileNames(string name);
    void xHandleTargetFile(int view, int frame);
    int xCalcFilePos(int f);

public:
    VideoHandler(int w, int h, int nv, int nf, string name, string videoName);

    UPel** getMacroblock(int v, int f, int x, int y);
    UPel* getMBNeighboring(int v, int f, int x, int y);

    UPel** getBlock(int v, int f, int x, int y);
    UPel* getBlockNeighboring(int v, int f, int x, int y);

    void insertResidualBlock(Pel** block, int x, int y, Statistics* stats);
    void insertResidualSubBlock(Pel** block, int x, int y, int xx, int yy, Statistics* stats);
    void writeResidualFrameInFile();

    void insertErrorBlock(Pel** block, int x, int y);
    void writeErrorFrameInFile();

    void insertLossyReconBlock(UPel** block, int view, int frame, int x, int y);
    void writeLossyReconInFile();

    void closeFiles();
    

    int getNumOfGOP();
    int getNumOfViews();
    int getWidth();
    int getHeight();
    string getVideoName();
};

#endif	/* _VIDEOHANDLER_H */

