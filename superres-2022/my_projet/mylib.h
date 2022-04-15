#ifndef MYLIB_H
#define MYLIB_H

#define SIZE_I 960
#define SIZE_J 1280
#define BLOCK_SIZE 16

#include "opencv2/opencv.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>

using namespace std;

using namespace cv;

//---------------------get_frame-------------------

Mat get_frame(Mat frame);

//---------------------noirBlanc-------------------

Mat noirBlanc(Mat frame);

//---------------------Plus Proche Voisin----------

Mat plus_proche_voisin(Mat image_BR, int res);

//---------------------Bilinéaire------------------

Mat bilineaire(Mat image_BR, int res);

//---------------------Bicubique ------------------

Mat bicubique(Mat image_BR,int res);

//---------------------Recentre l'image------------

Mat affichage_centre(Mat image_BR, int res);

Mat echantillonnage(Mat image_BR, int res);

#endif
