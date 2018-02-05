/* ------------------------------------------------------------ */
/*   simur.cpp: Run only a stochastic simulation of demographic */
/*   Input  : Parameters of simulation                          */
/*   Output : Number of population for this simulation          */
/* ------------------------------------------------------------ */

#include <iostream>
#include <cstdio>
#include <sstream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cstring>

#include "TGenMT.hpp"
#include "simuPo.hpp"

using namespace std;

int main(int argc, char **argv)
{
   double    res, lambda;
   int       idRep; 
   FILE     *fp;
   simuPo   *sim = NULL;
   char      outFile[100], inFile[50];
   ifstream  readfile;

   if(argc < 2 || argc > 3)
   {
      cout << "simur.cpp - e000: invalid args:";
      cout << "./simu data_file idRep" << endl;
      return -1;
   }
    
   strcpy(inFile,argv[1]);
   idRep  = atoi(argv[2]);
     
   /*Get inputs from file*/
   readfile.open(inFile);
   readfile >> lambda;

   sim = new simuPo(lambda, idRep);
   res = sim->growPo();

   delete sim;

   sprintf(outFile, "working/resRep_%06d", idRep);

   fp = fopen(outFile, "w");

   if(fp == NULL){
      cout << "simur.cpp - e001: Can't open file" << endl;
      return -1;
   }

   fprintf(fp, "%lf\n", res);

   fclose(fp);

   return 0;
}
/* -------------------------------------------------------------*/
