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

using namespace std;

class VideoHandler {
private:

    int w, h, nv, nf;
    int targetView, targetFrame;
    Pel* reconFrame;
    Pel* residualFrame;
    vector<string> fileNames;
    fstream reconFile, residualFile;

    void xInitFileNames(string name);
    void xHandleTargetFile(int view, int frame);
    int xCalcFilePos(int f);

public:
    VideoHandler(int w, int h, int nv, int nf, string name);

    Pel** getBlock(int v, int f, int x, int y);
    Pel* getNeighboring(int v, int f, int x, int y);

    void insertResidualBlock(Pel** block, int x, int y);
    void writeResidualFrameInFile();
    void closeFiles();

    int getNumOfFrames();
    int getNumOfViews();
    int getWidth();
    int getHeight();
    
};

#endif	/* _VIDEOHANDLER_H */

