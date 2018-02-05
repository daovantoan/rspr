/* ------------------------------------------------------------ */
/*   calnum.c: Determine the number of replications for a simu  */
/*   Input : None                                               */
/*   Output: Average of individus in parallel                   */
/*   Created by DAO Van Toan - August 2016                      */
/* ------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include "data.h"
#include "genMRIP.h"
#include "runSim.h"

/*-------------------------------------------------------*/
int main (int argc, char **argv)
{
   double  total=0., total_in=0., val, average, average_in;
   double  scart, cof,cof_eff,imme;
   int     nbRep, nBlocks, nbRes, nbNew, nbIm, nbTemp, nbPU, nbSock, nbCPU;
   int     nbFile = 0, count = 0, check = 0, ii, jj, nbCore, nbRep_int;
   char    prog[50], inFile[50], command[100];
   char  * path_work="./working";
   char  * path_out="./out";
   char  * path_data="./data";
   FILE  * fp;
   struct  timeval start_t, finish_t;

   if (argc != 5){
      printf("calnum.c - e001: invalid of args: ./calnum"
             " program (nbCPU) nbRep inputfile\n");
      return -1;
   }

   strcpy(prog,argv[1]);
   nbCPU  = atoi(argv[2]);
   nbRep  = atoi(argv[3]);
   strcpy(inFile,argv[4]);
   

   /*Get number of processor per core*/
   fp = popen ("hwloc-calc --number-of pu socket:0.core:0", "r");
   if (fp!=NULL)
   { 
      fscanf(fp, "%d", &nbPU);
      pclose(fp);
   }else {
      printf("rspr.c - e002: Can't open file!\n");
      exit(1);
   }

   /*Get number of sockets in machine*/
   fp = popen ("hwloc-calc --number-of socket all", "r");
   if (fp!=NULL)
   {
       fscanf(fp, "%d", &nbSock);
       pclose(fp);
   } else {
      printf("rspr.c - e003: Can't open file!\n");
      exit(1);
   }
  
   nbCore = nbCPU / nbPU;
   int call = nbCPU % nbPU;
   if(nbCore == 0 || call != 0){
      printf("calnum -e004: invalid of args: Nb of CPU should be Nx%d\n",nbPU);
      return -1;
   }   

   if(check_folder(path_data) == 0){
      create_folder(path_data);
      genMT_new(nbRep);
   }else if(check_folder(path_data) == 1
      && nbRep > count_file(path_data) 
      && nbRep < 100000){
      genMT_res(count_file(path_data), nbRep);
   }

   if(check_folder(path_work) == 0){
      create_folder(path_work);
   }

   if(check_folder(path_out) == 0){
      create_folder(path_out);
   }
   
   /*Delete old datas*/
   remove_file(path_work);

   /*begin of time calculating*/
   if(gettimeofday(&start_t,NULL) != 0){
      printf("calnum -e005: gettime error for starting\n");
      return -1;
   }

   nbRep_int = nbRep;

   /*Runs blocks of programs */
   runSimu(nbCore, nbPU, nbSock, nbRep, prog,inFile);
   
   /*Reduction of results*/
   for(ii = 0; ii < nbRep; ii++)
   {
     sprintf(command, "working/resRep_%06d", ii);
     val = 0.;

     fp  = fopen(command,"r");
     if(fp == NULL){
         printf("calnum.c - e006: Can't open file to read output\n");
         return -1;
      }

     fscanf(fp,"%lf",&val);

     total    += val;
     total_in += val * val;

     fclose(fp);
   }
   
   average    = total/(double) nbRep;
   average_in = total_in/(double) nbRep;
   imme       = average_in - (average * average);
   scart      = sqrt(imme);

   nbTemp     = (int)(((2.764 * 100 * scart)/average) * ((2.764 * 100 * scart)/average));

   nbRep      = nbTemp - nbRep;
  
   printf("Temporaire value : %d new Rep value: %d \n", nbTemp, nbRep);
 
   while(check == 0)
   {
      total = 0.; total_in = 0.; nbFile = 0;

      if(check_folder(path_data) == 1 && nbRep > count_file(path_data) && nbRep < 100000){
         genMT_res(count_file(path_data), nbRep);
      }

      if(nbRep > count_file(path_work)){ 
         runSimu_sup(nbCore, nbPU, nbSock, count_file(path_work), nbRep, prog, inFile);
      }

      /*Reduction of results*/
      for(ii = 0; ii < nbRep; ii++)
      {
         sprintf(command, "working/resRep_%06d", ii);
         val = 0.;

         fp = fopen(command,"r");
            if(fp == NULL){
            printf("calnum.c - e007: Can't open file to read output\n");
            return -1;
         }  

         fscanf(fp,"%lf",&val);

         total    += val;
         total_in += val * val;

         fclose(fp);
      }

      average    = total/(double) nbRep;
      average_in = total_in/(double) nbRep;
      imme       = average_in - (average * average);
      scart      = sqrt(imme);
      cof_eff    = (2.5758 * scart)/sqrt(nbTemp);
      cof        = average/100.;

      if(cof_eff > cof){
         nbTemp = (int)(((2.5758 * scart)/cof) * ((2.5758 * scart)/cof));
         nbRep  = nbTemp - nbRep;
         count++;
      }
      else{
         check = 1;
         if(count == 0)
         {
            nbTemp = (int)(((2.5758 * scart)/cof) * ((2.5758 * scart)/cof));
            nbRep  = nbRep - nbTemp;
         }
      }
   }

   /*Final calculation*/
   if(check_folder(path_data) == 1 && nbTemp > count_file(path_data) && nbTemp < 100000){
      genMT_res(count_file(path_data), nbTemp);
   }
   
   if(nbTemp > count_file(path_work)){
      runSimu_sup(nbCore, nbPU, nbSock, count_file(path_work), nbTemp, prog, inFile);
   }

   total = 0.; total_in = 0.;

   for(ii = 0; ii < nbTemp; ii++)
   {
      sprintf(command, "working/resRep_%06d", ii);
      val = 0.;

      fp = fopen(command,"r");
      if(fp == NULL){
         printf("calnum.c - e008: Can't open file to read output\n");
         return -1;
      }     

      fscanf(fp,"%lf",&val);

      total    += val;
      total_in += val * val;

      fclose(fp);
   }

   average    = total/(double) nbTemp;
   average_in = total_in/(double) nbTemp;
   imme       = average_in - (average * average);
   scart      = sqrt(imme);
   cof_eff    = (2.5758 * scart)/sqrt(nbTemp);
   cof        = average / 100.;
   int nbRec  = (int)(((2.5758 * scart)/cof)*((2.5758 * scart)/cof));
   
   time(&stop);

   sprintf(command, "out/resFinal_%d", nbTemp);
   fp  = fopen(command, "w");
   if(fp == NULL){
      printf("calnum.c - e009: Can't open file to write final result\n");
      return -1;
   }

  fprintf(fp, "Nombre de replication detecte: %d\n", nbTemp);
  fprintf(fp, "Moyenne: %14.10f\n", average);
  fprintf(fp, "Ecart type: %14.10f\n", scart);
  fprintf(fp, "Interval de confidence calcule: %14.10f\n", cof_eff);
  fprintf(fp, "Interval de confidence est 1/100 de moyenne: %14.10f\n",cof);
  fprintf(fp, "Nombre de replication recalcule: %d\n", nbRec);
  fprintf(fp, "Nombre de coeurs utilise: %d\n", nbCore);
  fprintf(fp, "Nombre de replication intial: %d\n", nbRep_int);
  fprintf(fp, "Temps execution: %.0f\n", difftime(stop,start));
  fclose(fp);

  /*Delete old datas*/
  remove_file(path_work);

  return 0;

}
