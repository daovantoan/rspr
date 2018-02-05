/* ------------------------------------------------------------ */
/*   runSim.c: Runs a simulation in parallel,a supporting for   */
/*             determine number of replication of simulation    */
/* ------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "genMRIP.h"
/* ------------------------------------------------------------ */
/*   runSim: Runs a simulation in parallel                      */
/*   Input: number of cores, number of replications, inputfile  */
/*          name of program                                     */
/*   Output: None                                               */
/* ------------------------------------------------------------ */
void runSimu(int    nbCore, int    nbPU, int nbSock, int nbRep,
             char * prog,   char * inFile)
{
   int jj, chFork, aBloc, nBlocks, nbRes, nbFile = 0;
   char command[200];
   FILE *fp;
   /*Number of blocks with nbProc*/
   aBloc = nbCore * nbPU;
   nBlocks = (int)(nbRep / aBloc);
   nbRes = nbRep % aBloc;
   /*Create shell scripts for MRIP technic*/
   if(nbRes == 0) genSimu(nbCore,prog,inFile);
   else {
      genSimu(nbCore,prog,inFile);
      genRes(nbCore, nBlocks, nbRes, prog, inFile);
      sprintf(command, "./simuRes %d %d", nBlocks, nbRes);
      system(command);
   }
   /*Runs blocks of programs, each block runs programs in parallel*/
   while(jj < nBlocks)
   {
      sprintf(command, "./simuMRIP %d", jj);
      system(command);
      jj++;
   } 
}

/* ------------------------------------------------------------ */
/*   runSim: Run new adding simulation in parallel              */
/*   Input: number of cores, number of replications, inputfile  */
/*          name of program                                     */
/*   Output: None                                               */
/* ------------------------------------------------------------ */
void runSimu_sup(int nbCore,    int    nbPU, int    nbSock, int nbRep_out,
                 int nbRep_new, char * prog, char * inFile)
{
   int jj, chFork, aBloc, nBlocks, nBloc_old;
   int nbRes, nbRep, nbRes_old, nbSup, nbFile;
   char command[200];
   int tmp;
   FILE *fp;
   /*Calculation existed*/
   aBloc = nbCore * nbPU;
   nBloc_old = (int)(nbRep_out/aBloc);
   nbRes_old = nbRep_out % aBloc;
   if(nbRes_old !=0)
   {
      sprintf(command, "find . -name 'simuRes_sup' -print0 | xargs -0 rm");
      system(command);
      genRes_sup(nbCore, nbRep_out, prog, inFile);
      sprintf(command, "./simuRes_sup");
      system(command);
      tmp = nBloc_old + 1;
   }
   else tmp = nBloc_old;
   /*Number of blocks with nbProc*/
   nBlocks =(int)(nbRep_new / aBloc);
   nbRes = nbRep_new % aBloc;
   /*Create shell scripts for MRIP technic*/
   if(nbRes != 0 || nBlocks == 0)
   {
      sprintf(command, "find . -name 'simuRes' -print0 | xargs -0 rm");
      system(command);
      genRes(nbCore, nBlocks, nbRes, prog, inFile);
      sprintf(command, "./simuRes");
      system(command);
   }
   /*Runs blocks of programs, each block runs programs in parallel*/
   while(jj < nBlocks)
   {
      sprintf(command, "./simuMRIP %d", jj);
      system(command);
      jj++;
   }
}
