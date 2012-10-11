#ifndef _DEFINES_H
#define	_DEFINES_H

#define EN_SUB_BLOCK 1

#define BLOCK_SIZE 16
#define SUB_BLOCK_SIZE 4

#define BLOCK_MODE true
#define SUB_BLOCK_MODE false

typedef char Pel;

enum IntraMode {
    HOR_MODE,
    VER_MODE,
    DIAG_MODE,
    DC_MODE
};

#endif	/* _DEFINES_H */

