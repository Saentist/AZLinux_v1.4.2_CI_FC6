
#ifndef STB0899_COMMON_H
#define STB0899_COMMON_H


#define HOST_PC
#define NO_GUI

#define I2C_WRITE  0
#define I2C_READ   1

#ifndef NULL
#define NULL	(void *)0
#endif

/* BOOL type constant values */
#ifndef TRUE
    #define TRUE (1 == 1)
#endif
#ifndef FALSE
    #define FALSE (!TRUE)
#endif

#define INRANGE(X,Y,Z) (((X<=Y) && (Y<=Z))||((Z<=Y) && (Y<=X)) ? 1 : 0)
#define LSB(X) ((X & 0xFF))
#define MSB(Y) ((Y>>8)& 0xFF)
#define ABS(X) ((X)<0?(-X):(X))
#define MAKEWORD(X,Y) ((((X)&0xFF)<<8)+((Y)&0xFF))
#define MAX(X,Y) ((X)>=(Y)?(X):(Y))
#define MIN(X,Y) ((X)>=(Y)?(X):(Y))

#define WAIT_N_MS(X) msleep_interruptible(X)

#define dprintk(args...) \
	do { \
		if (debug) printk(KERN_DEBUG "stb0899: " args); \
	} while (0)

typedef int BOOL;
typedef char S8;
typedef int  S16;
typedef long S32;

typedef int FE_STB0899_Handle_t;




#endif

