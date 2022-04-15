#include "mylib.h"
#include "opencv2/opencv.hpp"
#include <bits/stdc++.h>
#include <chrono>
using namespace std;

int main(int, char**)
{ 		
	VideoCapture cap(0); // open the default camera
	if(!cap.isOpened())  // check if we succeeded
		return -1;
	
	while(1){
		Mat frame;

auto start = chrono::high_resolution_clock::now();
	    	cap >> frame;
	    	char c=(char)waitKey(25);
			printf("x : %u y : %u\n",frame.cols,frame.rows);
		
		/*if(c == '1'){				// if '1' est appuye
		
			Mat NB = noirBlanc(frame);
	    		imshow("NoirEtBlanc", NB);
			
  
    
		}
		else if(c == '2'){			// if '2' est appuye
		
			Mat seuil = seuillage(frame);
	    		imshow("seuillage", seuil);
				auto end = chrono::high_resolution_clock::now();
  
  
    
		}
		else if (c == '3'){			// if '3' est appuye

			
  
			Mat cont = contour(frame);
			
	    		imshow("contour", cont);
		}*/
		else if(c == '0') destroyAllWindows();	// if '0' est appuye

		else imshow("frame", frame);
			

auto end = chrono::high_resolution_clock::now();
  
    // Calculating total time taken by the program.
    double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  
    time_taken *= 1e-6;
  
   cout << "Frame processing: " << fixed 
         << time_taken << setprecision(9);
    cout << " msec" << endl;
			
	    	if(c==27)				// if 'esc' est appuye
	      		break;


		
	}
  	// When everything done, release the video capture object
	cap.release();

	// Closes all the frames
	destroyAllWindows();

	return 0;
}
