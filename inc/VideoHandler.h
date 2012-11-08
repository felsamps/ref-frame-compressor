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
    Pel* reconFrame;
    Pel* residualFrame;
    bool* modeFrame;
    vector<string> fileNames;
    fstream reconFile, residualFile, varFile;
    string videoName;

    void xInitFileNames(string name);
    void xHandleTargetFile(int view, int frame);
    int xCalcFilePos(int f);

public:
    VideoHandler(int w, int h, int nv, int nf, string name, string videoName);

    Pel** getBlock(int v, int f, int x, int y);
    Pel* getNeighboring(int v, int f, int x, int y);

    Pel** getSubBlock(int v, int f, int x, int y);
    Pel* getSubNeighboring(int v, int f, int x, int y);

    void insertResidualBlock(Pel** block, int x, int y, bool mode);
    void insertResidualSubBlock(Pel** block, int x, int y);
    void writeResidualFrameInFile();
    void calcVar();
    void closeFiles();

    int getNumOfGOP();
    int getNumOfViews();
    int getWidth();
    int getHeight();
    string getVideoName();
};

#endif	/* _VIDEOHANDLER_H */

