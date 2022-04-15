#ifndef MYLIB_CUH
#define MYLIB_CUH

#include "mylib.h"


#include <cuda_runtime.h>


Mat seuillageGPU( Mat in);
Mat PPV_GPU( Mat in,int res);
Mat bili_GPU( Mat in,int res);
Mat bili_BIS_GPU( Mat in,int res);
Mat bicubic_GPU( Mat in,int res);
#endif
