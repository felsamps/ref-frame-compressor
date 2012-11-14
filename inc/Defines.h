#ifndef _DEFINES_H
#define	_DEFINES_H

#define I4_ONLY 0
#define I16_ONLY 1
#define I4_I16 2

#define BLOCK_SIZE 16
#define SUB_BLOCK_SIZE 4

#define BLOCK_MODE true
#define SUB_BLOCK_MODE false

#define SAMPLE_BIT_WIDTH 8
#define SMODE_BIT_WIDTH 4
#define MODE_BIT_WIDTH 2
#define ADAPTIVE_QUANT_ID_BIT_WIDTH 2

#define GOP_SIZE 8

#define TH0 1000
#define TH1 2000
#define TH2 3000



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

#define A_S (SUB_BLOCK_SIZE+1)
#define B_S (SUB_BLOCK_SIZE+2)
#define C_S (SUB_BLOCK_SIZE+3)
#define D_S (SUB_BLOCK_SIZE+4)
#define I_S (SUB_BLOCK_SIZE-1)
#define J_S (SUB_BLOCK_SIZE-2)
#define K_S (SUB_BLOCK_SIZE-3)
#define L_S (SUB_BLOCK_SIZE-4)
#define M_S (SUB_BLOCK_SIZE)




#endif	/* _DEFINES_H */

