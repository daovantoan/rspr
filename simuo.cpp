#include <iostream>
#include <cstdio>
#include <sstream>
#include <string>
#include <limits>
#include <cstdlib>
#include <omp.h>
#include <cmath>
#include <ctime>
#include <sys/time.h>

#include "TGenMT.hpp"
#include "simuPo.hpp"

using namespace std;

#define SIZE_SIMU 35000

int main(int argc, char **argv)
{
   double         par[SIZE_SIMU] = { 0. };
   double         total = 0., averagep, total_in = 0., average_in;
   double         lamda, , scart, cof_eff;
   int            ind, nbRep, nthreads, nbthread = 0;
   char           outFile[100];
   simuPo         *instanceSM[SIZE_SIMU] = { NULL };
   FILE           *fp; 
   struct timeval start_t, finish_t;

   if(argc < 2 || argc > 3)
   {
      cout << "Number of args invalid: ./simuo lambda nbRep" << endl;
      return -1;
   }
   
   lamda = atof(argv[1]);
   nbRep = atoi(argv[2]); 
 
   if(gettimeofday(&start_t,NULL) != 0){
      cout << "simuo.cpp: gettime error for starting " << endl;
      return -1;
   }

   #pragma omp parallel for reduction(+:total,total_in) private(ind)
   for( ind= 0; ind < nbRep; ind++)
   {
      if( omp_get_thread_num() == 0) 
         nbthread = omp_get_num_threads();

      instanceSM[ind] = new simuPo(lamda,ind);     
      par[ind]        = instanceSM[ind]->growPo();
  
      total    += par[ind]; 
      total_in += par[ind] * par[ind];
   }

   
   if(gettimeofday(&finish_t,NULL) != 0){
      cout << "simuo.cpp: gettime error for stoping " << endl;
      return -1;
   }  

   for(ind = 0; ind < nbRep; ind++)
   {
      delete instanceSM[ind];
   }

   /*Calculating*/
   averagep    = total/(double) nbRep ;
   average_in  = total_in/(double) nbRep;
   double imme = average_in - (averagep * averagep);
   scart       = sqrt(imme);
   cof_eff     = (2.5758 * scart)/sqrt(nbRep);

   sprintf(outFile, "out/resRep_om_%d", nbthread);

   fp  = fopen(outFile, "w");
   if(fp == NULL){
      cout << "simuo.cpp - e000: Can't open file\n" << endl;
      return -1;
   }
   
   fprintf(fp, "Parallel execution of %d threads:\n", nbthread); 
   for(ind = 0; ind < nbRep; ind++)
   {
      fprintf(fp, "%d : %14.10f\n", ind, par[ind]);
   }

   fprintf(fp, "Nombre de replications: %d\n", nbRep);
   fprintf(fp, "Moyenne: %14.10f\n", averagep);
   fprintf(fp, "Ecart-type: %14.10f\n", scart);
   fprintf(fp, "Total carree: %14.10f\n", total_in);
   fprintf(fp, "Moyenne carree: %14.10f\n", average_in);
   fprintf(fp, "Intervalle de confidence: %14.10f\n", cof_eff);
   fprintf(fp, "Temps d'exectuions: %lf\n",((finish_t.tv_sec + finish_t.tv_usec * 1e-6) - (start_t.tv_sec + start_t.tv_usec * 1e-6)));

   fclose(fp);
        
   return 0;
}
