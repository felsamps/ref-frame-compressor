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
    Pel* frame;
    vector<string> fileNames;
    fstream file;

    void xInitFileNames(string name);
    void xHandleTargetFile(int view, int frame);
    int xCalcFilePos(int f);

public:
    VideoHandler(int w, int h, int nv, int nf, string name);

    Pel** getBlock(int v, int f, int x, int y);


};

#endif	/* _VIDEOHANDLER_H */

