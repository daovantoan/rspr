/* ------------------------------------------------------------ */
/*   genMRIP.c: Generate shell scripts for simu program         */
/*             to allow the programm to be run in parallel      */
/*   Input : None                                               */
/*   Output: Shell scripts for simu program                     */
/*   Created by DAO Van Toan - August 2016                      */
/* ------------------------------------------------------------ */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------ */
/*   genSimu: Create a shell script that has nbProc executions  */
/*             in parallel                                      */
/*   Input : number of cores/processors                         */
/*   Output: Shell scripts for simu program                     */
/* ------------------------------------------------------------ */
void genSimu(int nbCore, char * nameProg, char * inFile)
{
  int    ii = 0, jj, nbSock, nbCPSoc, nbPU, count=0, kk, tt;
  char   command[50];
  FILE * fp;

  /*Get number of sockets in machine*/
  fp = popen ("hwloc-calc --number-of socket all", "r");
  if (fp!=NULL)
  {
     fscanf(fp, "%d", &nbSock);
     pclose(fp);
  }else {
     printf("genMRIP.c - e000: Can't open file!\n");
     exit(1);
  }

  /*Get number of cores per socket*/
  fp = popen ("hwloc-calc --number-of core socket:0", "r");
  if (fp!=NULL)
  {
     fscanf(fp, "%d", &nbCPSoc);
     pclose(fp);
  }else {
     printf("genMRIP.c - e001: Can't open file!\n");
     exit(1);
  }


  /*Get number of processor per core*/
  fp = popen ("hwloc-calc --number-of pu socket:0.core:0", "r");
  if (fp!=NULL)
  {
     fscanf(fp, "%d", &nbPU);
     pclose(fp);
  }else {
     printf("genMRIP.c - e002: Can't open file!\n");
     exit(1);
  }


  /* Creat a shell script */
  FILE *file = fopen("simuMRIP", "w");

  if (file==NULL)
  {
     printf("genMRIP.c - e003: Can't open file!\n");
     exit(1);
  }

  const char * text = "#!/bin/bash";
  fprintf(file,"%s \n",text);
  fprintf(file,"\n");
  fprintf(file,"#number of physical cores from input:%d\n", nbCore);
  fprintf(file,"#number of sockets in machine:%d\n", nbSock);
  fprintf(file,"#number of cores per socket:%d\n", nbCPSoc);
  fprintf(file,"#number of logical processors per core:%d\n", nbPU);
  fprintf(file,"\n");
  fprintf(file,"listPids=\"\"\n");

  fprintf(file,"t=$(expr $1 \\* %d \\* %d)\n", nbPU, nbCore);
  fprintf(file,"\n");

  if(nbSock == 1) {
    for(tt = 0; tt < nbPU; tt++){
       for(kk = 0; kk < nbCore; kk++)
       {
          fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
          fprintf(file,"\n");
          fprintf(file,"hwloc-bind socket:0.core:%d.pu:%d ./%s %s $idRep%d & \n",kk,tt, nameProg, inFile, ii);
          fprintf(file,"listPids=\"$listPids $!\"\n");
          fprintf(file,"\n");
          ii++;
       }
    }
  } else if(nbCore == 1){
     int nbRep = nbPU;

     for(tt = 0; tt < nbPU; tt++){
        for(jj = 0; jj < nbSock; jj++)
        {
           if(ii < nbRep){
              fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
              fprintf(file,"\n");
              fprintf(file,"hwloc-bind socket:%d.core:1.pu:%d ./%s %s $idRep%d & \n", jj,tt, nameProg, inFile, ii);
              fprintf(file,"listPids=\"$listPids $!\"\n");
              fprintf(file,"\n");
              ii++;
           }
        }
     }
  }else {

    int nbRep = nbPU * nbCore;
    nbCore = nbCore/nbSock;

    for(tt = 0; tt < nbPU; tt++){
       for(kk = 0; kk < nbCore; kk++){
          for(jj = 0; jj < nbSock; jj++)
          {
             if(ii < nbRep){
                fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
                fprintf(file,"\n");
                fprintf(file,"hwloc-bind socket:%d.core:%d.pu:%d ./%s %s $idRep%d & \n",jj,kk, tt, nameProg, inFile, ii);
                fprintf(file,"listPids=\"$listPids $!\"\n");
                fprintf(file,"\n");
                ii++;
             }
          }
        }
     }
  }

  fprintf(file,"wait $listPids\n");
  fprintf(file,"\n");

  fclose(file);

  sprintf(command, "chmod +x ./simuMRIP");
  system(command);

}

/* ------------------------------------------------------------ */
/*   genRes: Create a additional shell script for a case of     */
/*          redundant in the division between nbRep and nbProc  */
/*   Input : number of cores, blocs, redundancy                 */
/*   Output: Additional shell script for simu program           */
/* ------------------------------------------------------------ */
void genRes(int nbCore, int nBlocks, int nbRes, char * nameProg, char * inFile)
{
  int    ii=0, jj, tt,kk, nbSock, nbPU, count=0;
  char   command[50];
  FILE * fp;

  /*Get number of sockets in machine*/
  fp = popen ("hwloc-calc --number-of socket all", "r");
  if (fp!=NULL)
  {
     fscanf(fp, "%d", &nbSock);
     pclose(fp);
  }else{
     printf("genMRIP.c - e004: Can't open file!\n");
     exit(1);
  }

  /*Get number of processor per core*/
  fp = popen ("hwloc-calc --number-of pu socket:0.core:0", "r");
  if (fp!=NULL)
  {
     fscanf(fp, "%d", &nbPU);
     pclose(fp);
  }else{
     printf("genMRIP.c - e005: Can't open file!\n");
     exit(1);
  }

  /* Creat a shell script */
  FILE *file = fopen("simuRes", "w");

  if (file==NULL)
  {
     printf("genMRIP.c - e006: Can't open file!\n");
     exit(1);
  }

  int aBloc = nbCore * nbPU;
  const char * text = "#!/bin/bash";
  fprintf(file,"%s \n",text);
  fprintf(file,"\n");
  fprintf(file,"listPids=\"\"\n");
  fprintf(file,"t=$(expr %d \\* %d)\n", aBloc, nBlocks);
  fprintf(file,"\n");

  if(nbRes == 1){
     fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
     fprintf(file,"\n");
     fprintf(file,"hwloc-bind socket:0.core:0.pu:0 ./%s %s $idRep%d & \n", nameProg, inFile, ii);
     fprintf(file,"listPids=\"$listPids $!\"\n");
  }

  if(nbSock == 1) {
     for(tt = 0; tt < nbPU; tt++)
        for(kk = 0; kk < nbCore; kk++)
        {
           if(ii < nbRes){
              fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
              fprintf(file,"\n");
              fprintf(file,"hwloc-bind socket:0.core:%d.pu:%d ./%s %s $idRep%d & \n", kk,tt, nameProg, inFile, ii);
              fprintf(file,"listPids=\"$listPids $!\"\n");
              fprintf(file,"\n");
              ii++;
           }
         }
  }else {
     nbCore = nbCore / nbSock;

     for(tt = 0; tt < nbPU; tt++)
        for(kk = 0; kk < nbCore; kk++)
           for(jj = 0; jj < nbSock; jj++)
           {
              if(ii < nbRes){
                 fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
                 fprintf(file,"\n");
                 fprintf(file,"hwloc-bind socket:%d.core:%d.pu:%d ./%s %s $idRep%d & \n", jj, kk, tt, nameProg, inFile, ii);
                 fprintf(file,"listPids=\"$listPids $!\"\n");
                 fprintf(file,"\n");
                 ii++;
             }
          }
     }

  fprintf(file,"wait $listPids\n");
  fprintf(file,"\n");
  fclose(file);

  sprintf(command, "chmod +x ./simuRes");
  system(command);
}

/* ------------------------------------------------------------ */
/* genRes_sup: Generate a supplemaintaire script                */
/* ------------------------------------------------------------ */

void genRes_sup(int nbCore, int nbRep_out, char * nameProg, char * inFile)
{
  int ii=0, jj, kk, tt, nbSock, count=0,  nbRes, nbSup, nbPU;
  char command[50];
  FILE *fp;

  /*Get number of sockets in machine*/
  fp = popen ("hwloc-calc --number-of socket all", "r");
  if (fp!=NULL)
  {
     fscanf(fp, "%d", &nbSock);
     pclose(fp);
  }else {
     printf("genMRIP.c - e007: Can't open file!\n");
     exit(1);
  }

  /*Get number of processor per core*/
  fp = popen ("hwloc-calc --number-of pu socket:0.core:0", "r");
  if (fp!=NULL)
  {
     fscanf(fp, "%d", &nbPU);
     pclose(fp);
  }else {
     printf("genMRIP.c - e008: Can't open file!\n");
     exit(1);
  }

  int aBloc = nbCore * nbPU;
  nbRes     = nbRep_out % aBloc;
  nbSup     = aBloc - nbRes;

  /* Creat a shell script */
  FILE *file = fopen("simuRes_sup", "w");

  if (file==NULL)
  {
     printf("genMRIP.c - e009: Can't open file!\n");
     exit(1);
  }

  const char * text = "#!/bin/bash";
  fprintf(file,"%s \n",text);
  fprintf(file,"\n");
  fprintf(file,"listPids=\"\"\n");
  fprintf(file,"t=%d\n", nbRep_out);
  fprintf(file,"\n");

  if(nbSup == 1){
     fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
     fprintf(file,"\n");
     fprintf(file,"hwloc-bind socket:0.core:0.pu:0 ./%s %s $idRep%d & \n", nameProg, inFile, ii);
     fprintf(file,"listPids=\"$listPids $!\"\n");
  }

  if(nbSock == 1) {
     for(tt = 0; tt < nbPU; tt++)
        for(kk = 0; kk < nbCore; kk++)
        {
           if(ii < nbSup){
              fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
              fprintf(file,"\n");
              fprintf(file,"hwloc-bind socket:0.core:%d.pu:%d ./%s %s $idRep%d & \n", kk,tt, nameProg, inFile, ii);
              fprintf(file,"listPids=\"$listPids $!\"\n");
              fprintf(file,"\n");
              ii++;
           }
         }
     } else {

     nbCore = nbCore / nbSock;

     for(tt = 0; tt < nbPU; tt++)
        for(kk = 0; kk < nbCore; kk++)
           for(jj = 0; jj < nbSock; jj++)
           {
              if(ii < nbSup){
                 fprintf(file,"idRep%d=$(expr $t + %d)", ii, ii);
                 fprintf(file,"\n");
                 fprintf(file,"hwloc-bind socket:%d.core:%d.pu:%d ./%s %s $idRep%d & \n", jj, kk, tt, nameProg, inFile, ii);
                 fprintf(file,"listPids=\"$listPids $!\"\n");
                 fprintf(file,"\n");
                 ii++;
             }
          }
     }

  fprintf(file,"wait $listPids\n");

  fprintf(file,"\n");
  fclose(file);

  sprintf(command, "chmod +x ./simuRes_sup");
  system(command);
}
