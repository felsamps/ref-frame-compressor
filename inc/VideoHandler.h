#ifndef _VIDEOHANDLER_H
#define	_VIDEOHANDLER_H

#include "../inc/Defines.h"

#include <string>
#include <vector>
#include <fstream>
#include <iosfwd>
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

class VideoHandler {
private:

    int w, h, nv, gops;
    int targetView, targetFrame;
    UPel* reconFrame;
    Pel* residualFrame;
    Pel* errorFrame;
    UPel*** lossyReconFrame;
    bool* modeFrame;
    vector<string> fileNames;
    fstream reconFile, residualFile, errorFile, lossyReconFile;
    string videoName;

    void xInitFileNames(string name);
    void xHandleTargetFile(int view, int frame);
    int xCalcFilePos(int f);

public:
    VideoHandler(int w, int h, int nv, int nf, string name, string videoName);

    UPel** getBlock(int v, int f, int x, int y);
    UPel* getNeighboring(int v, int f, int x, int y);

    UPel** getSubBlock(int v, int f, int x, int y);
    UPel* getSubNeighboring(int v, int f, int x, int y);

    void insertResidualBlock(Pel** block, int x, int y, bool mode);
    void insertResidualSubBlock(Pel** block, int x, int y, int xx, int yy);
    void writeResidualFrameInFile();

    void insertErrorBlock(Pel** block, int x, int y);
    void writeErrorFrameInFile();

    void insertLossyReconBlock(int** block, int view, int frame, int x, int y);
    void writeLossyReconInFile();

    void closeFiles();
    

    int getNumOfGOP();
    int getNumOfViews();
    int getWidth();
    int getHeight();
    string getVideoName();
};

#endif	/* _VIDEOHANDLER_H */

