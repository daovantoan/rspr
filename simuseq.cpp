#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <limits>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <sys/time.h>

#include "TGenMT.hpp"
#include "simuPo.hpp"

using namespace std;

#define SIZE_SIMU 35000

/* -------------------------------------------------------------*/
int main(int argc, char **argv)
{
   double         seq[SIZE_SIMU] = {0.};
   double         total = 0., total_in = 0., averages, average_in;
   double         lamda, scart, cof_eff;
   int            ind, nbRep; 
   char           outFile[100];
   simuPo         *instanceSM[SIZE_SIMU] = { NULL };
   struct timeval start_t, finish_t;
   FILE           *fp;
   
   if(argc < 2 || argc > 3)
   {
      cout << "simuseq.cpp - e001: invalid: ./simseq lambda nbRep" << endl;
      return -1;
   }

   lamda = atof(argv[1]);
   nbRep = atoi(argv[2]);

   if(gettimeofday(&start_t,NULL) != 0){
      cout << "simuseq.cpp - e002: gettime error for starting " << endl;
      return -1;
   }

   for(ind = 0; ind < nbRep; ind++)
   {
      instanceSM[ind] = new simuPo(lamda,ind);
      seq[ind]        = instanceSM[ind]->growPo();

      total    += seq[ind];
      total_in += seq[ind] * seq[ind];
   }

   if(gettimeofday(&finish_t,NULL) != 0){
      cout << "simuseq.cpp - e003: gettime error for starting " << endl;
      return -1;
   }

   for(ind = 0; ind < nbRep; ind++)
   {
      delete instanceSM[ind];
   }
   
  
   /*Save intermediate results*/
   fp  = fopen("out/res_seq.im", "w");
   if(fp == NULL){
      printf("rspr.c - e004: Can't open file to write output\n");
      return -1;
   }

   for(ind = 0; ind < nbRep; ind++)
   {
      fprintf(fp, "%d : %14.10f\n", ind, seq[ind]);
   }
   fclose(fp);

   /*Calculating*/
   averages    = total/(double) nbRep;
   average_in  = total_in/(double)nbRep;
   double imme = average_in - (averages * averages);
   scart       = sqrt(imme);
   cof_eff     = (2.5758 * scart)/sqrt(nbRep); 

   /*Write all of results to file*/
   fp  = fopen("out/resRep_seq", "w");
   if(fp == NULL){
      cout << "simuseq.cpp - e005: Can't open file\n" << endl;
      return -1;
   }

   fprintf(fp, "Execution sequentielle:\n");

   for(ind = 0; ind < nbRep; ind++)
   {
      fprintf(fp, "%d : %14.10f\n", ind, seq[ind]);
   }
   
   fprintf(fp, "Nombre de replications: %d\n", nbRep);
   fprintf(fp, "Moyenne: %14.10f\n", averages);
   fprintf(fp, "Ecart-type: %14.10f\n", scart);
   fprintf(fp, "Total caree: %14.10f\n", total_in);
   fprintf(fp, "Moyenne carree: %14.10f\n",average_in);
   fprintf(fp, "Intervalle de confidence: %14.10f\n", cof_eff);
   fprintf(fp, "Temps d'executions: %lf\n", 
               ((finish_t.tv_sec + finish_t.tv_usec * 1e-6) 
               - (start_t.tv_sec + start_t.tv_usec * 1e-6)));
   fclose(fp);

   return 0;
}
/* -------------------------------------------------------------*/
