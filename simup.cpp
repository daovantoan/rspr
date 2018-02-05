#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <cmath>
#include <limits>
#include <pthread.h>
#include <unistd.h>

#include "TGenMT.hpp"
#include "simuPo.hpp"

using namespace std;

#define NBTHREADS 4
#define SIZE_SIMU 35000

/* -------------------------------------------------------------*/
/* A structure for a thread of replications                     */
/* -------------------------------------------------------------*/
typedef struct {
   int idRep;
   int nbRep;
   int nbRes;
   int lamda;
}s_data;

double result_imme[SIZE_SIMU] = { 0. }; /*Save intermediates*/

/* -------------------------------------------------------------*/
/* Run a simulation with N replications                         */
/* -------------------------------------------------------------*/
void *runSimu(void * data)
{
   int      tid, ind, nbExp, ires, lambda, numRes;
   int      res_begin = 0, res_end = 0;
   FILE   * fp[SIZE_SIMU];
   s_data * data2 = (s_data *)data;

   tid    = data2->idRep;
   nbExp  = data2->nbRep;
   lambda = data2->lamda;
   numRes = data2->nbRes;
   
   simuPo * instanceSM[SIZE_SIMU] = { NULL };
   int nbegin = tid * nbExp, nbEnd = nbExp * (tid + 1);

   for(ind = nbegin; ind < nbEnd; ind++)
   {
      instanceSM[ind] = new simuPo(lambda,ind);
      result_imme[ind] = instanceSM[ind]->growPo();
   }

   if(numRes != 0 && tid == 0)
   {
      res_begin = nbExp * NBTHREADS;
      res_end   = res_begin + numRes;

      if(res_end <= SIZE_SIMU){
         for(ires = res_begin; ires < res_end; ires++)
         {
            instanceSM[ires] = new simuPo(lambda,ires);
            result_imme[ires] = instanceSM[ires]->growPo();}
      }
      else cout << "Size of a simulation is not enough" << endl;
   }

   for(ind = nbegin; ind < nbEnd + res_end; ind++)
   {
      delete instanceSM[ind];
   }
}

/* -----------------------Main function------------------------*/

int main(int argc, char **argv)
{
   double         total = 0., total_in = 0., average, average_in;
   double         scart, cof_eff;
   int            nbRepT, ind, nbRep, lam, rc, nbres; 
   pthread_t      threadID[NBTHREADS];
   s_data         data[NBTHREADS];
   FILE         * fp;
   char           command[100], outFile[100];
   struct timeval start_t, finish_t;
   
   /*Affinity thread*/
   pthread_attr_t attr;
   cpu_set_t cpus;
   pthread_attr_init(&attr);

   if(argc < 2 || argc > 3)
   {
      cout << "Number of args invalid: ./simup lambda nbRep" << endl;
      return -1;
   }

   lam    = atoi(argv[1]);
   nbRep  = atoi(argv[2]);
   nbRepT = nbRep / NBTHREADS;
   nbres  = nbRep % NBTHREADS; 
   
   if(gettimeofday(&start_t,NULL) != 0){
      cout << "simuo.cpp: gettime error for starting " << endl;
      return -1;
   }
    
   for(ind = 0; ind < NBTHREADS; ind++)
   {
      data[ind].idRep = ind;
      data[ind].nbRep = nbRepT;
      data[ind].lamda = lam;
      data[ind].nbRes = nbres;
      
      CPU_ZERO(&cpus);  
      CPU_SET(ind,&cpus);
      pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
      rc = pthread_create(&threadID[ind], NULL, runSimu, &data[ind]);

      if(rc){
        cout << "simup.cpp - e001 : Unable to create thread," 
             << rc << endl;
        return -1;
      }
   }
   
   /*waiting thread is terminated*/
   for(ind = 0; ind < NBTHREADS; ind++){
      pthread_join(threadID[ind], NULL);
   }
   
   if(gettimeofday(&finish_t,NULL) != 0){
      cout << "simuo.cpp: gettime error for stoping " << endl;
      return -1;
   }  

   /*Reduction*/
   for(ind = 0; ind < nbRep; ind++)
   {
      total    += result_imme[ind];
      total_in += result_imme[ind] * result_imme[ind];
   }

   average     = total/(double) nbRep;
   average_in  = total_in/(double) nbRep;
   double imme = average_in - (average * average);
   scart       = sqrt(imme);
   cof_eff     = (2.5758 * scart)/sqrt(nbRep);

   sprintf(outFile, "out/resRep_pth_%d", NBTHREADS);
   fp  = fopen(outFile, "w");
   if(fp == NULL){
      cout << "simup.cpp - e002 : Can't open file\n" << endl;
      return -1;
   }

   fprintf(fp, "Parallel execution of %d threads\n", NBTHREADS);
   for(ind = 0; ind < nbRep; ind++)
   {
      fprintf(fp, "%d : %14.10f\n", ind, result_imme[ind]);
   }

   fprintf(fp, "Nombre de replications: %d\n", nbRep);
   fprintf(fp, "Moyenne: %14.10f\n", average);
   fprintf(fp, "Total carree: %14.10f\n", total_in);
   fprintf(fp, "Moyenne carree: %14.10f\n", average_in);
   fprintf(fp, "Ecart-type: %14.10f\n", scart);
   fprintf(fp, "Intervalle de confidence: %14.10f\n", cof_eff);
   fprintf(fp, "Temps d'exectuions: %lf\n",((finish_t.tv_sec + finish_t.tv_usec * 1e-6) - (start_t.tv_sec + start_t.tv_usec * 1e-6)));
   fclose(fp);

   return 0;
}
