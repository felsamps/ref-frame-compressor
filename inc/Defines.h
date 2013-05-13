#ifndef _DEFINES_H
#define	_DEFINES_H

#define MACROBLOCK_SIZE 16
#define BLOCK_SIZE 4

#define SAMPLE_BIT_WIDTH 8
#define SMODE_BIT_WIDTH 4
#define MODE_BIT_WIDTH 2
#define ADAPTIVE_QUANT_ID_BIT_WIDTH 2

#define GOP_SIZE 8

#define TH0 1000
#define TH1 2000
#define TH2 3000

#define SIGMA_TIMES 2
#define SIGMA_PCTG 0.042

typedef char Pel;

typedef unsigned char UPel;

enum IntraMode {
    VER_MODE,
    HOR_MODE,
    DC_MODE,
    PLANE_MODE
};

enum SubIntraMode {
    VER_SMODE,
    HOR_SMODE,
    DC_SMODE,
    DL_SMODE,
    DR_SMODE,
    VR_SMODE,
    HD_SMODE,
    VL_SMODE,
    HU_SMODE
};

#define A_S (BLOCK_SIZE+1)
#define B_S (BLOCK_SIZE+2)
#define C_S (BLOCK_SIZE+3)
#define D_S (BLOCK_SIZE+4)
#define I_S (BLOCK_SIZE-1)
#define J_S (BLOCK_SIZE-2)
#define K_S (BLOCK_SIZE-3)
#define L_S (BLOCK_SIZE-4)
#define M_S (BLOCK_SIZE)




#endif	/* _DEFINES_H */

