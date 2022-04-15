#include "mylib.h"
#include "mylib.cuh"
#include <iostream>
#include <chrono>

__global__ void kernel_PPV_GPU(unsigned char *d_image_in, unsigned char *d_image_out,int size_j,int res)
{
	int i,j,vi,vj;
	

	int height = blockIdx.x*BLOCK_SIZE; // num de block dans la grille de block
	int width = blockIdx.y*BLOCK_SIZE;

	i = height + threadIdx.x;// recuperer l'identifiant d'un thread dans les blocs
	j = width + threadIdx.y;

	for(vi=0;vi<res;vi++) // Parcourir les nouveaux pixels sur la même ligne
	{
		for(vj=0;vj<res;vj++)  // Parcourir les nouveaux pixels sur la même colonne
		{
			d_image_out[0+3*(j*res+vj)+(i*res+vi)*3*size_j*res] = d_image_in[0+j*3+i*3*size_j];
			d_image_out[1+3*(j*res+vj)+(i*res+vi)*3*size_j*res] = d_image_in[1+j*3+i*3*size_j];
			d_image_out[2+3*(j*res+vj)+(i*res+vi)*3*size_j*res] = d_image_in[2+j*3+i*3*size_j];
		}	
	}
}

Mat PPV_GPU( Mat in,int res)
{
	cudaError_t error;
	Mat out;
	out.create(in.rows*res,in.cols*res,CV_8UC3);
	auto start= std::chrono::high_resolution_clock::now();

	// allocate host memory
	unsigned char *h_image_in_GPU ;
	h_image_in_GPU=in.data;
	
	/*cudaEvent_t start,stop,start_mem,stop_mem;
	error = cudaEventCreate(&start_mem);
	error = cudaEventCreate(&stop_mem);
	
	error = cudaEventRecord(start, NULL);
	error = cudaEventSynchronize(start);*/
	
	// images on device memoryÍÍÍ
	unsigned char *d_image_in_GPU;
	unsigned char *d_image_out_GPU;
	
	const unsigned long int mem_size=in.cols*in.rows*3*sizeof(unsigned char);
	
	// Alocation mémoire de d_image_in et d_image_out sur la carte GPU
	cudaMalloc((void**) &d_image_in_GPU,mem_size );
	cudaMalloc((void**) &d_image_out_GPU, mem_size*res*res);
	
	// copy host memory to device
	cudaMemcpy(d_image_in_GPU, h_image_in_GPU,mem_size ,cudaMemcpyHostToDevice);
	
	//error = cudaEventRecord(stop_mem, NULL);
	
	// Wait for the stop event to complete
	//error = cudaEventSynchronize(stop_mem);
	//float msecMem = 0.0f;
	//error = cudaEventElapsedTime(&msecMem, start, stop_mem);
	
	// setup execution parameters -> découpage en threads
	dim3 threads(BLOCK_SIZE,BLOCK_SIZE);
	dim3 grid(in.rows/BLOCK_SIZE,in.cols/BLOCK_SIZE);
	
	// lancement des threads executé sur la carte GPU
	kernel_PPV_GPU<<< grid, threads >>>(d_image_in_GPU, d_image_out_GPU,in.cols,res);
	
	// Record the start event
	//error = cudaEventRecord(start_mem, NULL);
	//error = cudaEventSynchronize(start_mem);
	
	// copy result from device to host
	cudaMemcpy(out.data, d_image_out_GPU, mem_size*res*res,cudaMemcpyDeviceToHost);
	cudaFree(d_image_in_GPU);
	cudaFree(d_image_out_GPU);
	/*
	float msecTotal,msecMem2;
	error = cudaEventRecord(stop, NULL);
	error = cudaEventSynchronize(stop);
	error = cudaEventElapsedTime(&msecTotal, start, stop);
	error = cudaEventElapsedTime(&msecMem2, start_mem, stop);
	*/
	auto end= std::chrono::high_resolution_clock::now();
   auto resultat=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
   std::cout<< "Temps Plus Proche Voisin avec GPU ="<<resultat.count()<<"millisecondes "<<endl;
	return out;
}

__global__ void kernel_bili_GPU(unsigned char *d_image_in, unsigned char *d_image_out,int size_j,int res)
{
	int i,j,vi,vj,P,X=0,N=0, coef;
	int a ,b ;
	int tab_ref[5][2];
	int diff_rows,diff_cols;
	unsigned char *d_image_out_bis;
	
	int height = blockIdx.x*BLOCK_SIZE; // num de block dans la grille de block
	int width = blockIdx.y*BLOCK_SIZE;

	i = height + threadIdx.x;// recuperer l'identifiant d'un thread dans les blocs
	j = width + threadIdx.y;

		//echantillonage
		d_image_out[1+(j*res)*3+(i*res*res)*3*size_j] = d_image_in[1+j*3+i*3*size_j];
		d_image_out[0+(j*res)*3+(i*res*res)*3*size_j] = d_image_in[0+j*3+i*3*size_j];
		d_image_out[2+(j*res)*3+(i*res*res)*3*size_j] = d_image_in[2+j*3+i*3*size_j];
	
	for(vi=0;vi<res;vi++) // Parcourir les nouveaux pixels sur la même ligne
	{
		for(vj=0;vj<res;vj++) // Parcourir les nouveaux pixels sur la même colonne
		{
			if((vi>0) || (vj>0)) // Si pixel inconnu
			{
				a=1;
			}		
			else if((vi>0) && (vj>0)) // Si pixel connu
			{
				a=0;
			}
			d_image_out[0+3*(j*res+vj)+(i*res+vi)*3*size_j*res] = (d_image_in[0+j*3+i*3*size_j]+d_image_in[0+(j+1)*3+(i+1)*3*size_j]+d_image_in[0+(j+1)*3+i*3*size_j]+d_image_in[0+j*3+(i+1)*3*size_j])/(4);
			d_image_out[1+3*(j*res+vj)+(i*res+vi)*3*size_j*res] = (d_image_in[1+j*3+i*3*size_j]+d_image_in[1+(j+1)*3+(i+1)*3*size_j]+d_image_in[1+(j+1)*3+i*3*size_j]+d_image_in[1+j*3+(i+1)*3*size_j])/(4);
			d_image_out[2+3*(j*res+vj)+(i*res+vi)*3*size_j*res] = (d_image_in[2+j*3+i*3*size_j]+d_image_in[2+(j+1)*3+(i+1)*3*size_j]+d_image_in[2+(j+1)*3+i*3*size_j]+d_image_in[2+j*3+(i+1)*3*size_j])/(4);
		}
	}		
}

Mat bili_GPU( Mat in,int res)
{
	cudaError_t error;
	Mat out;
	out.create(in.rows*res,in.cols*res,CV_8UC3);
	
	auto start= std::chrono::high_resolution_clock::now();
	// allocate host memory
	unsigned char *h_image_in_GPU ;
	h_image_in_GPU=in.data;
	
	/*cudaEvent_t start,stop,start_mem,stop_mem;
	error = cudaEventCreate(&start_mem);
	error = cudaEventCreate(&stop_mem);
	
	error = cudaEventRecord(start, NULL);
	error = cudaEventSynchronize(start);*/
	
	// images on device memoryÍÍÍ
	unsigned char *d_image_in_GPU;
	unsigned char *d_image_out_GPU;
	
	const unsigned long int mem_size=in.cols*in.rows*3*sizeof(unsigned char);
	
	// Alocation mémoire de d_image_in et d_image_out sur la carte GPU
	cudaMalloc((void**) &d_image_in_GPU,mem_size );
	cudaMalloc((void**) &d_image_out_GPU, mem_size*res*res);
	
	// copy host memory to device
	cudaMemcpy(d_image_in_GPU, h_image_in_GPU,mem_size ,cudaMemcpyHostToDevice);
	
	//error = cudaEventRecord(stop_mem, NULL);
	
	// Wait for the stop event to complete
	//error = cudaEventSynchronize(stop_mem);
	//float msecMem = 0.0f;
	//error = cudaEventElapsedTime(&msecMem, start, stop_mem);
	
	// setup execution parameters -> découpage en threads
	dim3 threads(BLOCK_SIZE,BLOCK_SIZE);
	dim3 grid(in.rows/BLOCK_SIZE,in.cols/BLOCK_SIZE);
	
	// lancement des threads executé sur la carte GPU
	kernel_bili_GPU<<< grid, threads >>>(d_image_in_GPU, d_image_out_GPU,in.cols,res);
	
	// Record the start event
	//error = cudaEventRecord(start_mem, NULL);
	//error = cudaEventSynchronize(start_mem);
	
	// copy result from device to host
	cudaMemcpy(out.data, d_image_out_GPU, mem_size*res*res,cudaMemcpyDeviceToHost);
	cudaFree(d_image_in_GPU);
	cudaFree(d_image_out_GPU);
	/*
	float msecTotal,msecMem2;
	error = cudaEventRecord(stop, NULL);
	error = cudaEventSynchronize(stop);
	error = cudaEventElapsedTime(&msecTotal, start, stop);
	error = cudaEventElapsedTime(&msecMem2, start_mem, stop);
	*/
	auto end= std::chrono::high_resolution_clock::now();
   auto resultat=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
   std::cout<< "Temps Bilinéaire avec GPU ="<<resultat.count()<<"millisecondes "<<endl;
   
	return out;
}

__global__ void kernel_bicubic_GPU(unsigned char *d_image_in, unsigned char *d_image_out,int size_j,int res)
{
	int i,j,vi,vj,P,X=0,N=0, coef;
	int a ,b ,M,Q,tab_ref1[4];
	int tab_ref[5][2], rseu[4][3];
	int diff_rows,diff_cols;
	unsigned char *d_image_out_bis;
	
	int height = blockIdx.x*BLOCK_SIZE; // num de block dans la grille de block
	int width = blockIdx.y*BLOCK_SIZE;

	i = height + threadIdx.x;// recuperer l'identifiant d'un thread dans les blocs
	j = width + threadIdx.y;

		//echantillonage
		d_image_out[1+(j*res)*3+(i*res*res)*3*size_j] = d_image_in[1+j*3+i*3*size_j];
		d_image_out[0+(j*res)*3+(i*res*res)*3*size_j] = d_image_in[0+j*3+i*3*size_j];
		d_image_out[2+(j*res)*3+(i*res*res)*3*size_j] = d_image_in[2+j*3+i*3*size_j];
	
	for(vi=0;vi<res;vi++) // Parcourir les nouveaux pixels sur la même ligne
	{
		for(vj=0;vj<res;vj++) // Parcourir les nouveaux pixels sur la même colonne
		{
			if ((vi<=0) && (vj<=0))
			{
				
			}
			else if(vi==0) // Si pixel inconnu colonne
			{
				for(int M=0;M<4;M++)
				{
					tab_ref1[M]=i-2*(1-M);
				}
				for (int R=0;R<4;R++) 
				{
					float p=1.0;
					for(int T=0;T<4;T++)
					{
						if(R!=T)
						{
							p = p* (i - tab_ref1[T])/(tab_ref1[R] - tab_ref1[T]);
						}
					}
					for ( int A=0;A<3;A++)
					{
						Q=round(p * d_image_in[A+j*3+tab_ref1[M]*3*size_j]);
						d_image_out[A+3*(j*res+vj)+(i*res+vi)*3*size_j*res]= d_image_out[A+3*(j*res+vj)+(i*res+vi)*3*size_j*res] + Q ;
					} 
				}
			}		
			else if(vj==0) // Si pixel connu
			{
				for(int M=0;M<4;M++)
				{
					tab_ref1[M]=j-2*(1-M);
				}
				for (int R=0;R<4;R++) 
				{
					float p=1.0;
					for(int T=0;T<4;T++)
					{
						if(R!=T)
						{
							p = p* (j - tab_ref1[T])/(tab_ref1[R] - tab_ref1[T]);
						}
					}
					for ( int A=0;A<3;A++)
					{
						Q=round(p * d_image_in[A+j*3+tab_ref1[M]*3*size_j]);
						d_image_out[A+3*(j*res+vj)+(i*res+vi)*3*size_j*res]= d_image_out[A+3*(j*res+vj)+(i*res+vi)*3*size_j*res] + Q ;
					} 
				}
			}
			
			else 
			{
				for (int Y=0;Y<4;Y++)
				{
					for(int M=0;M<4;M++)
					{
						tab_ref1[M]=j-2*(1-M);
					}
					for (int R=0;R<4;R++) 
					{
						float p=1.0;
						for(int T=0;T<4;T++)
						{
							if(R!=T)
							{
								p = p* (j - tab_ref1[T])/(tab_ref1[R] - tab_ref1[T]);
							}
						}
						for ( int A=0;A<3;A++)
						{
							Q=round(p * d_image_in[A+j-(1-Y)*3+tab_ref1[M]*3*size_j]);
							rseu[Y][A]= rseu[Y][A] + Q ;
						} 
					}
				}
				for(int M=0;M<4;M++)
				{
					tab_ref1[M]=i-2*(1-M);
				}
				for (int R=0;R<4;R++) 
				{
					float p=1.0;
					for(int T=0;T<4;T++)
					{
						if(R!=T)
						{
							p = p* (i - tab_ref1[T])/(tab_ref1[R] - tab_ref1[T]);
						}
					}
					for ( int A=0;A<3;A++)
					{
						Q=round(p * rseu[M][A]);
						d_image_out[A+3*(j*res+vj)+(i*res+vi)*3*size_j*res]= d_image_out[A+3*(j*res+vj)+(i*res+vi)*3*size_j*res] + Q ;
					} 
				}
			}
		}
	}		
}

Mat bicubic_GPU( Mat in,int res)
{
	cudaError_t error;
	Mat out;
	float msecTotal,msecMem2,msecMem = 0.0f;;
	out.create(in.rows*res,in.cols*res,CV_8UC3);
	
	//auto start= std::chrono::high_resolution_clock::now();
	// allocate host memory
	unsigned char *h_image_in_GPU ;
	h_image_in_GPU=in.data;
	
	cudaEvent_t start,stop,start_mem,stop_mem;
	error = cudaEventCreate(&start_mem);
	error = cudaEventCreate(&stop_mem);
	
	error = cudaEventRecord(start, NULL);
	error = cudaEventSynchronize(start);
	

	// images on device memoryÍÍÍ
	unsigned char *d_image_in_GPU;
	unsigned char *d_image_out_GPU;
	
	const unsigned long int mem_size=in.cols*in.rows*3*sizeof(unsigned char);
	
	// Alocation mémoire de d_image_in et d_image_out sur la carte GPU
	cudaMalloc((void**) &d_image_in_GPU,mem_size );
	cudaMalloc((void**) &d_image_out_GPU, mem_size*res*res);
	
	// copy host memory to device
	cudaMemcpy(d_image_in_GPU, h_image_in_GPU,mem_size ,cudaMemcpyHostToDevice);
	
	error = cudaEventRecord(stop_mem, NULL);
	
	// Wait for the stop event to complete
	error = cudaEventSynchronize(stop_mem);
	error = cudaEventElapsedTime(&msecMem, start, stop_mem);
	
	// setup execution parameters -> découpage en threads
	dim3 threads(BLOCK_SIZE,BLOCK_SIZE);
	dim3 grid(in.rows/BLOCK_SIZE,in.cols/BLOCK_SIZE);
	
	// lancement des threads executé sur la carte GPU
	kernel_bicubic_GPU<<< grid, threads >>>(d_image_in_GPU, d_image_out_GPU,in.cols,res);
	
	// Record the start event
	error = cudaEventRecord(start_mem, NULL);
	error = cudaEventSynchronize(start_mem);
	
	// copy result from device to host
	cudaMemcpy(out.data, d_image_out_GPU, mem_size*res*res,cudaMemcpyDeviceToHost);
	cudaFree(d_image_in_GPU);
	cudaFree(d_image_out_GPU);
	
	
	/*error = cudaEventRecord(stop, NULL);
	error = cudaEventSynchronize(stop);*/
	/*error = cudaEventElapsedTime(&msecTotal, start, stop);
	error = cudaEventElapsedTime(&msecMem2, start_mem, stop);*/
	
	/*auto end= std::chrono::high_resolution_clock::now();
   auto resultat=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
   std::cout<< "Temps Bicubique avec GPU ="<<resultat.count()<<"millisecondes "<<endl;*/
  //  std::cout<< "Temps Bicubique avec GPU ="<<msecTotal<<"millisecondes "<<"calcul "<<msecTotal-(msecMem+msecMem2)<<"("<<(1-(msecMem+msecMem2)/msecTotal)*100<<")"<<endl;
	return out;
}