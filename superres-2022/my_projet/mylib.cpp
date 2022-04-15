#include "mylib.h"
#include <iostream>
#include <chrono>

//---------------------noirBlanc-----------------------
Mat noirBlanc(Mat frame)
{
   Mat im_gray_out;
  
   if (frame.empty())
   exit(0);
  
   cvtColor(frame,im_gray_out,COLOR_RGB2GRAY);
   return im_gray_out;
}

//---------------------Centrer l'image-----------------------
Mat affichage_centre(Mat image_BR, int res)
{
   int height=image_BR.cols ;
   int width=image_BR.rows;
   Mat image_HR;
   image_HR.create(height,width,CV_8UC3);
 
   for(int i=(res-1)*height/(2*res);i<(res+1)*height/(2*res)+1;i++)
   {
       for(int j=(res-1)*width/(2*res);j<(res+1)*width/(2*res)+1;j++)
       {
           for(int k=0;k<4;k++)
           {
               image_HR.at<Vec3b>(i-(res-1)*height/(2*res),j-(res-1)*width/(2*res))[k]= image_BR.at<Vec3b>(i,j)[k];
           }
       }
   }
   return image_HR;
} 

//---------------------Interpolation plus proche voisin-----------------------
Mat plus_proche_voisin(Mat image_BR,int res)
{
   int width = image_BR.cols*res ;
   int height = image_BR.rows*res ;
   Mat image_HR;
   image_HR.create(height,width,CV_8UC3);

  //(i,j) coordonnées basse resolution
  //k indice de couleur
  auto start= std::chrono::high_resolution_clock::now();
   for (int i=0;i<(height/res);i++)
   {
       for (int j=0;j<(width/res);j++)
       {
           for (int k=0;k<3;k++)
           {
               for (int vi=0;vi<res;vi++)
               {
                   for (int wj=0;wj<res;wj++)
                   {
                       image_HR.at<Vec3b>(i*res+vi,j*res+wj)[k] = image_BR.at<Vec3b>(i,j)[k];
                   } 
               }
           }   
       }
   } 
   //image_HR=noirBlanc(image_HR);
   auto end= std::chrono::high_resolution_clock::now();
   auto resultat=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
   std::cout<< "Temps Plus Proche Voisin ="<<resultat.count()<<"millisecondes "<<endl;
   return image_HR;
}

//---------------------Interpolation bicubique-----------------------

Mat bicubique(Mat image_BR,int res)
{
   
   int width= image_BR.cols*res;
   int height =  image_BR.rows*res;
   int N=0,D=0,X=0,A=0;
   signed int a,b;
   int Q, ligne=0, colonne=0;
   int P1[4][3];
   float q;
   //int tab_ref0[4][2],tab_ref1[4][2],tab_ref2[4][2],tab_ref3[8][2];
   int tab_ref[4][4][2];
   int tab_ref1[4];
   Mat image_HR;
   image_HR.create(height,width,CV_8UC3);
   Mat image_HR_bis;
   image_HR_bis.create(height,width,CV_8UC3);
 
    auto start= std::chrono::high_resolution_clock::now();
   // échantillonnage de l'image
   for (int i=0;i<(height/res);i++)
   {
       for (int j=0;j<(width/res);j++)
       {
            for (int k=0;k<3;k++)
           {
              image_HR_bis.at<Vec3b>(i*res,j*res)[k] = image_BR.at<Vec3b>(i,j)[k]; // Placer dans une image HR BIS l'image BR multipliée par la résolution 
              image_HR.at<Vec3b>(i*res,j*res)[k] = image_BR.at<Vec3b>(i,j)[k];
           }   
       }
   }
    //image_HR=image_HR_bis; //Prendre une image échantillonnée de référence sans modifier l'image finale
    
    for (int I=3;I<height-4;I++) // Parcourir les pixels de l'image HR BIS
    {
       for (int J=4;J<width-4;J++)
       {
            int P=image_HR_bis.at<Vec3b>(I,J)[0];N=0;
            if(P<=0) // Utiliser les pixels qui n'ont pas de valeur
            {
                for(a=I-(2*res);a<I+(2*res+1);a++)
                {
                    if((a<height) && (a>-1)) //Si on ne sort pas de l'image
                    {
                        for ( b=J-(2*res);b<J+(2*res);b++) //Recherche des 16 pixels connus sur la colonne 
                        {
                            if ((b>-1) && (b<width))  //Si on ne sort pas de l'image  
                            { 
                                int G=image_HR_bis.at<Vec3b>(a,b)[0];
                                if((G>=1) && (N<16)) // ranger les pixels connus dans un tableau
                                {
                                    if(N<4)
                                    {
                                        tab_ref[0][N][0] = a; 
                                        tab_ref[0][N][1] = b;
                                    }
                                    else if ((N>3) && (N<8))
                                    {
                                        tab_ref[1][N-4][0] = a; 
                                        tab_ref[1][N-4][1] = b;
                                    }
                                    else if ((N>7) && (N<12))
                                    {
                                        tab_ref[2][N-8][0] = a; 
                                        tab_ref[2][N-8][1] = b;
                                    }
                                    else
                                    {
                                        tab_ref[3][N-12][0] = a; 
                                        tab_ref[3][N-12][1] = b;
                                    }

                                    N++; // N représente le nombre de pixels connus autour du pixel inconnu
                                }
                            }
                        }
                    }
                }

                colonne =0; ligne=0;
               for (int U=0;U<4;U++)
                {
                    if(tab_ref[U][0][0] == I)
                    {
                        ligne= U+1;
                    }
                    else if (tab_ref[0][U][1] == J)
                    {
                        colonne = U+1;
                    }
                } 
                    //----------cubique---------- 
                if((ligne <= 0) && (colonne <= 0))  
                {

                    for (int R=0;R<4;R++)
                    {
                        float p=1.0;
                        for(int T=0;T<4;T++)
                        {
                            if(R!=T)
                            {
                                p = p* (J - tab_ref[0][T][1])/(tab_ref[0][R][1] - tab_ref[0][T][1]);
                            }
                        }
                        for ( A=0;A<3;A++)
                        {
                            Q=round(p * image_HR_bis.at<Vec3b>(tab_ref[0][R][0],tab_ref[0][R][1])[A]);
                            image_HR_bis.at<Vec3b>(tab_ref[0][0][0],J)[A] = image_HR_bis.at<Vec3b>(tab_ref[0][0][0],J)[A] + Q;
                        }
                    }
                    tab_ref1[0]=tab_ref[0][0][0];
                    for (int R=0;R<4;R++) 
                    {
                        float p=1.0;
                        for(int T=0;T<4;T++)
                        {
                            if(R!=T)
                            {
                                p = p* (J - tab_ref[1][T][1])/(tab_ref[1][R][1] - tab_ref[1][T][1]);
                            }
                        }
                        for ( A=0;A<3;A++)
                        {
                            Q=round(p * image_HR_bis.at<Vec3b>(tab_ref[1][R][0],tab_ref[1][R][1])[A]);
                            image_HR_bis.at<Vec3b>(tab_ref[1][0][0],J)[A] = image_HR_bis.at<Vec3b>(tab_ref[1][0][0],J)[A] + Q ;
                        } 
                    } 
                    tab_ref1[1]=tab_ref[1][0][0];
                    for (int R=0;R<4;R++) 
                    {
                        float p=1.0;
                        for(int T=0;T<4;T++)
                        {
                            if(R!=T)
                            {
                                p = p* (J - tab_ref[2][T][1])/(tab_ref[2][R][1] - tab_ref[2][T][1]);
                            }
                        }
                        for ( A=0;A<3;A++)
                        {
                            Q=round(p * image_HR_bis.at<Vec3b>(tab_ref[2][R][0],tab_ref[2][R][1])[A]);
                            image_HR_bis.at<Vec3b>(tab_ref[2][0][0],J)[A] = image_HR_bis.at<Vec3b>(tab_ref[2][0][0],J)[A] + Q ;
                        } 
                    }   
                    tab_ref1[2]=tab_ref[2][0][0];
                    for (int R=0;R<4;R++) 
                    {
                        float p=1.0;
                        for(int T=0;T<4;T++)
                        {
                            if(R!=T)
                            {
                                p = p* (J - tab_ref[3][T][1])/(tab_ref[3][R][1] - tab_ref[3][T][1]);
                            }
                        }
                        for ( A=0;A<3;A++)
                        {
                            Q=round(p * image_HR_bis.at<Vec3b>(tab_ref[3][R][0],tab_ref[3][R][1])[A]);
                            image_HR_bis.at<Vec3b>(tab_ref[3][0][0],J)[A] = image_HR_bis.at<Vec3b>(tab_ref[3][0][0],J)[A] + Q ;
                        } 
                    } 
                    tab_ref1[3]=tab_ref[3][0][0];
                    //----------------------------------------------------
                    for (int R=0;R<4;R++) 
                    {
                        float p=1.0;
                        for(int T=0;T<4;T++)
                        {
                            if(R!=T)
                            {
                                p = p* (I - tab_ref1[T])/(tab_ref1[R] - tab_ref1[T]);
                            }
                        }
                        for ( A=0;A<3;A++)
                        {
                            Q=round(p * image_HR_bis.at<Vec3b>(tab_ref1[R],J)[A]);
                            image_HR.at<Vec3b>(I,J)[A] = image_HR.at<Vec3b>(I,J)[A] + Q ;
                        } 
                    } 
                    for(int O=0;O<4;O++)
                    {   
                        for(int A=0;A<3;A++)
                        { 
                            image_HR_bis.at<Vec3b>(tab_ref[O][0][0],J)[A]=0;
                        }
                    }
                }
                else if(ligne >= 1)    
                {
                    for(int M=0;M<4;M++)
                    {
                        tab_ref1[M]=tab_ref[ligne-1][M][1];
                    }
                    for (int R=0;R<4;R++) 
                    {
                        float p=1.0;
                        for(int T=0;T<4;T++)
                        {
                            if(R!=T)
                            {
                                p = p* (J - tab_ref1[T])/(tab_ref1[R] - tab_ref1[T]);
                            }
                        }
                        for ( A=0;A<3;A++)
                        {
                            Q=round(p * image_HR_bis.at<Vec3b>(I,tab_ref1[R])[A]);
                            image_HR.at<Vec3b>(I,J)[A] = image_HR.at<Vec3b>(I,J)[A] + Q ;
                        } 
                    }
                }
                else if(colonne >= 1)    
                {
                    for(int M=0;M<4;M++)
                    {
                        tab_ref1[M]=tab_ref[M][colonne-1][0];
                    }
                    for (int R=0;R<4;R++) 
                    {
                        float p=1.0;
                        for(int T=0;T<4;T++)
                        {
                            if(R!=T)
                            {
                                p = p* (I - tab_ref1[T])/(tab_ref1[R] - tab_ref1[T]);
                            }
                        }
                        for ( A=0;A<3;A++)
                        {
                            Q=round(p * image_HR_bis.at<Vec3b>(tab_ref1[R],J)[A]);
                            image_HR.at<Vec3b>(I,J)[A] = image_HR.at<Vec3b>(I,J)[A] + Q ;
                        } 
                    }
                }
                
                
                
            }
        }   
    }
  //image_HR=noirBlanc(image_HR); // mettre l'image finale en noir et blanc pour mieux voir la différence de précision
    auto end= std::chrono::high_resolution_clock::now();
   auto resultat=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
   std::cout<< "Temps Bicubique ="<<resultat.count()<<"millisecondes "<<endl;
    return image_HR;
}     

//---------------------Interpolation bilinéaraire-----------------------
Mat bilineaire(Mat image_BR,int res)
{
  int width= image_BR.cols*res;
  int height =  image_BR.rows*res;
  int coef = 0,N=0,P;
  int diff_rows,diff_cols; // Pour identifier la distance qui sépare les pixels connus du pixel inconnu
  int tab_ref[5][2]; // Ranger les pixels connus autour du pixel inconnu
  Mat image_HR; // image finale
  image_HR.create(height,width,CV_8UC3);
  Mat image_HR_bis; // image sur-échantillonnée
  image_HR_bis.create(height,width,CV_8UC3);

  auto start= std::chrono::high_resolution_clock::now();
  // échantillonnage de l'image
  for (int i=0;i<(height/res);i++)
  {
      for (int j=0;j<(width/res);j++)
      {
          for (int k=0;k<3;k++)
          {
             image_HR_bis.at<Vec3b>(i*res,j*res)[k] = image_BR.at<Vec3b>(i,j)[k]; // Placer dans une image HR BIS l'image BR multipliée par la résolution
          }  
      }
  }

    image_HR=image_HR_bis; //Prendre une image échantillonnée de référence sans modifier l'image Haute Résolution

  // Recherche des nouveaux pixels qui n'ont pas encore de valeur 
   
   for (int I=0;I<height;I++) // Parcourir les pixels de l'image HR BIS
   {
      for (int J=0;J<width;J++)
      {
           N=0;
           for (int W=0;W<5;W++) //trouver les pixels en diagonales, lignes et colonnes
                    {
                       tab_ref[W][0]=0;
                       tab_ref[W][1]=0;
                    }    

           
            P=image_HR.at<Vec3b>(I,J)[0];
           if(P<=0) // Utiliser les pixels qui n'ont pas de valeur
           {
               for (signed int a=I-(res-1);a<I+(res-1);a++) //Recherche des 4 pixels connus sur la ligne 
               {
                   if((a<height) && (a>-1)) //Si on ne sort pas de l'image
                   {
                       for (signed int b=J-(res-1);b<J+(res-1);b++) //Recherche des 4 pixels connus sur la colonne 
                       {
                           if ((b>-1) && (b<width)) //Si on ne sort pas de l'image
                           {
                               int G=image_HR_bis.at<Vec3b>(a,b)[0]; 
                               if(G>0) // ranger les pixels connus 
                               {
                                   tab_ref[N][0] = a;
                                   tab_ref[N][1] = b;
                                   N++; //N représente le nombre de pixels connus autour du pixel inconnu
                                  
                               }
                           }
                       }
                   }
               }
                if(N==2) //S'il n'y a que deux pixels connus
                {
                    for (int w=0;w<2;w++) //trouver les pixels en diagonales, lignes et colonnes
                    {
                        diff_cols=abs((tab_ref[w][0])-I); //calcul distance entre les pixels sur les lignes
                        diff_rows=abs((tab_ref[w][1])-J); //calcul distance entre les pixels sur les colonnes 
                        for (int v=0;v<3;v++) //trouver les pixels en diagonales, lignes et colonnes
                        {
                            image_HR.at<Vec3b>(I,J)[v]=image_HR.at<Vec3b>(I,J)[v]+image_HR_bis.at<Vec3b>( (tab_ref[w][0]) , (tab_ref[w][1]) )[v]*round((1-(diff_cols+diff_rows)/(2*res-1)));
                        }
                    }    
                } 

                else
                {
                    
                    for (int h=0;h<N;h++)
                    {
                        diff_cols=abs(tab_ref[h][0]-I); //calcul distance entre les pixels sur les lignes
                        diff_rows=abs(tab_ref[h][1]-J); //calcul distance entre les pixels sur les colonnes
                        coef=coef+diff_rows+diff_cols;  //calcul distance totale entre les pixels
                    }
                    for (int C=0;C<N;C++) //trouver les pixels en diagonales, lignes et colonnes
                    {      
                        for (int x=0;x<3;x++) //trouver les pixels en diagonales, lignes et colonnes
                        {
                            diff_cols=abs(tab_ref[C][0]-I);
                            diff_rows=abs(tab_ref[C][1]-J);
                            image_HR.at<Vec3b>(I,J)[x]=image_HR.at<Vec3b>(I,J)[x]+image_HR_bis.at<Vec3b>( (tab_ref[C][0]) , (tab_ref[C][1]) )[x]/N;
                        }
                    }
                }  
             }
        }    
    }         
     
   //image_HR=noirBlanc(image_HR);// mettre l'image finale en noir et blanc pour mieux voir la différence de précision
   auto end= std::chrono::high_resolution_clock::now();
   auto resultat=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
   std::cout<< "Temps Bilinéaire ="<<resultat.count()<<"millisecondes "<<endl;
   return image_HR;
}

Mat echantillonnage(Mat image_BR,int res)
{
   
   int width= image_BR.cols*res;
   int height =  image_BR.rows*res;
   int N=0,D=0,X=0,A=0;
   signed int a,b;
   int Q, ligne=0, colonne=0;
   int P1[4][3];
   float q;
   //int tab_ref0[4][2],tab_ref1[4][2],tab_ref2[4][2],tab_ref3[8][2];
   int tab_ref[4][4][2];
   int tab_ref1[4];
   Mat image_HR;
   image_HR.create(height,width,CV_8UC3);
   Mat image_HR_bis;
   image_HR_bis.create(height,width,CV_8UC3);
 
    auto start= std::chrono::high_resolution_clock::now();
   // échantillonnage de l'image
   for (int i=0;i<(height/res);i++)
   {
       for (int j=0;j<(width/res);j++)
       {
            for (int k=0;k<3;k++)
           {
              image_HR_bis.at<Vec3b>(i*res,j*res)[k] = image_BR.at<Vec3b>(i,j)[k]; // Placer dans une image HR BIS l'image BR multipliée par la résolution 
              image_HR.at<Vec3b>(i*res,j*res)[k] = image_BR.at<Vec3b>(i,j)[k];
           }   
       }
   }

   return image_HR;
}   








