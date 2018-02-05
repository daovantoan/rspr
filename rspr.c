/* ------------------------------------------------------------ */
/*   rspr.c: A tool runs a stochastic simulation in parallel    */
/*           and guarantee numerical reproducibility            */
/*   Input : stochastic simulation, inputs, parameters          */
/*   Output: Average or total calculated                        */
/*   Created by DAO Van Toan - v1.0  2016                       */
/* ------------------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "data.h"
#include "genMRIP.h"

#define SIZE_SIMU 35000

int main (int argc, char **argv)
{
   double  total = 0., average, average_in, total_in = 0.;
   double  cof_eff, scart, imme;
   double  res_imm[SIZE_SIMU] = { 0. };
   int     nbCore, nbRep, jj, ii, nBlocks, nbRes, nbFile = 0;
   int     nbCPU_in,typeCal, nbPU, nbSock, chFork, nbLimit = 15000;
   char    prog[50], indata[50], command[200], buffer[80];
   char  * path_work="./working";
   char  * path_data="./data";
   char  * path_out="./out";
   FILE  * fp;
   struct  timeval start_t, finish_t;

   if(argc < 6 || argc > 6){
      printf("rspr.c - e000: args invalid: ./rspr program nbCPU nbRep inputfile type\n");
      printf("rspr.c: type: 0 - total, 1 - average,deviation standard, confidence interval\n");
      return -1;
   }

   strcpy(prog,argv[1]);
   nbCPU_in = atoi(argv[2]);
   nbRep = atoi(argv[3]);
   strcpy(indata,argv[4]);
   typeCal = atoi(argv[5]);

   if(typeCal != 0 && typeCal != 1)
   {
      printf("rspr.c - e001: args invalid: 0 - total," 
                     "1 - average,deviation std,interval\n");
      return -1;
   }

   /*Get number of processor per core*/
   fp = popen ("hwloc-calc --number-of pu socket:0.core:0", "r");
   if (fp!=NULL)
   {
      fscanf(fp, "%d", &nbPU);
      pclose(fp);
   }else {
      printf("rspr.c - e00p: Can't open file!\n");
      exit(1);
   }

   /*Get number of sockets in machine*/
   fp = popen ("hwloc-calc --number-of socket all", "r");
   if (fp!=NULL)
   {
      fscanf(fp, "%d", &nbSock);
      pclose(fp);
   } else {
      printf("rspr.c - e002: Can't open file!\n");
      exit(1);
   }

   if(check_folder(path_data) == 0){
      create_folder(path_data);
      genMT_new(nbRep);
   }else if(check_folder(path_data) == 1 
            && nbRep > count_file(path_data)
            && nbRep < 100000)
   {
      genMT_res(count_file(path_data), nbRep);
   }

   if(check_folder(path_work) == 0){
      create_folder(path_work);
   }

   /*Number of blocks with nbProc*/
   nbCore    = nbCPU_in/nbPU;
   int call  = nbCPU_in%nbPU;
   if(nbCore  == 0 || call != 0)
   {
      printf("rspr.c - e003: Number of CPU should be even or Nx%d \n",nbPU);
      return -1;
   }

   nBlocks = nbRep / nbCPU_in;
   nbRes = nbRep % nbCPU_in;

   /*Delete old datas*/
   if(count_file(path_work)!= 0)
      remove_file(path_work);

   /*Create shell scripts for MRIP technic*/
   genSimu(nbCore, prog, indata);
   if(nBlocks == 0 || nbRes != 0)
   {
      genRes(nbCore,nBlocks,nbRes,prog,indata);
   }

   /*begin of time calculating*/
   if(gettimeofday(&start_t,NULL) != 0){
      printf("rspr.c - e004: gettime error for starting\n");
      return -1;
   }

   /*Runs blocks of processus in parallel */
   jj = nBlocks - 1;
   do {
      sprintf(command, "./simuMRIP %d", jj);
      system(command);
      jj--;
   }while (jj >= 0);

   if(nBlocks == 0 || nbRes != 0){
      sprintf(command, "./simuRes %d %d", nBlocks, nbRes);
      system(command);
   }

   /*End of time calculating*/
   if(gettimeofday(&finish_t,NULL) != 0){
      printf("rspr.c - e005: gettime error for stoping\n");
      return -1;
   }

   /*Save intermediate results*/
   sprintf(command, "out/res_%d.im", nbCPU_in);

   fp  = fopen(command, "w");
   if(fp == NULL){
      printf("rspr.c - e006: Can't open file to write output\n");
      return -1;
   }

   for(jj = 0; jj < nbRep; jj++)
   {
      fprintf(fp, "%d : %14.10f\n", jj, res_imm[jj]);
   }
   fclose(fp);

   /*Verify intermediate results between seq vs parallel*/
   int idenCom, fileStt;
   fileStt  = file_exist("out/res_seq.im");
   if(fileStt == 1) 
      idenCom = compare_intermediate("out/res_seq.im",command);

   /*Reduction of results*/
   ii = 0;
   while(ii < nbRep)
   {
      sprintf(command, "working/resRep_%06d", ii);
      fp = fopen(command,"r");
      if(fp == NULL){
         printf("rsprc.c - e007: Can't open file to read output\n");
         return -1;
      }
      fread(&buffer, sizeof(double), 1, fp);
      res_imm[ii] = atof(buffer);
      fclose(fp);

      total    += res_imm[ii];
      total_in += res_imm[ii] * res_imm[ii];

      ii++;
   }

   average    = total/(double) nbRep;
   average_in = total_in/(double) nbRep;
   imme       = average_in - (average * average);
   scart      = sqrt(imme);
   cof_eff    = (2.5758 * scart)/sqrt(nbRep);

   /*Write final results to a file*/
   sprintf(command, "out/resFinal_%d", nbCPU_in);

   fp  = fopen(command, "w");
   if(fp == NULL){
      printf("rspr.c - e008: Can't open file to write output\n");
      return -1;
   }

   fprintf(fp, "Execution parallèle avec %d cores:\n", nbCPU_in);
   for(jj = 0; jj < nbRep; jj++)
   {
      fprintf(fp, "%d : %14.10f\n", jj, res_imm[jj]);
   }

   fprintf(fp,"Numbre of replications: %d\n", nbRep);
   if(typeCal == 0){
   fprintf(fp,"Total calcule: %14.10f\n", total);
   }else{
      fprintf(fp, "Moyenne: %14.10f\n", average);
      fprintf(fp, "Ecart-type: %14.10f\n", scart);
      fprintf(fp, "Total carree: %14.10f\n", total_in);
      fprintf(fp, "Moyenne carree: %14.10f\n",average_in);
      fprintf(fp, "Interval de confidence: %14.10f\n", cof_eff);
   }
   fprintf(fp, "Temps execution: %lf\n", 
           ((finish_t.tv_sec + finish_t.tv_usec * 1e-6) 
            - 
           (start_t.tv_sec + start_t.tv_usec * 1e-6)));
   if(fileStt != 1) fprintf(fp,"Intermediate results of sequence does not exist\n");
   else{
      if(idenCom == -1) fprintf(fp,"Cannot open files to compare sequence vs parallel\n");
      else if(idenCom == 0) fprintf(fp, "Intermediate results are different\n");
      else fprintf(fp,"Intermediate results are identical\n");
   }
   fclose(fp);

   /*Delete temporary file*/
   if(count_file(path_work) != 0)
     remove_file(path_work);

   return 0;
}
