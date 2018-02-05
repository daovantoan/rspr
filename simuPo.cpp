/* ------------------------------------------------------------ */
/*  simuPo.cpp: Create a stochastic simulation of demographic   */
/*               using Poison law with Monte Carlo method       */
/*  Input : Lambda and identification of simulation             */
/*  Output: None                                                */
/* ------------------------------------------------------------ */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <cmath>

#include "TGenMT.hpp"
#include "simuPo.hpp"

#define NBGENERATION 20
#define NBINDIV 2

using namespace std;

/* ------------------------------------------------------------ */
/*   Define funtions and construct a new object                 */
/* ------------------------------------------------------------ */
simuPo::simuPo()
{
   lambda = 0.;
   idSimu = 0;
}

simuPo::simuPo(double lam, int id )
{
   setSimu(lam,id);
}

void simuPo::setSimu(double lam, int id)
{
   lambda = lam;
   idSimu = id;
}

double simuPo::getLamd()
{
   return lambda;
}

int simuPo::getId()
{
   return idSimu;
}

/* ------------------------------------------------------------ */
/* Ouput: A value of the poisson law with MT generator          */
/* ------------------------------------------------------------ */
unsigned long long int simuPo::poisson(){
   double        s = getLamd();
   long long int r = -1;

   while(s >= 0)
   {
       r = r + 1;
       s = s + log(mtGen->mtRand());
   }
   
   return r;
}

/* -------------------------------------------------------------*/
/* Ouput: A value of the population growth for a replication    */
/* -------------------------------------------------------------*/
unsigned long long int simuPo::growPo() {
   unsigned long long int  individu = NBINDIV, nbDes;
   int                     iter = 0, ind;
   int                     replicationId = getId();
   char                   *randFileStatus = new char[80]; 

   sprintf(randFileStatus,"data/inStatus_%06d", replicationId);

   mtGen = new TGenMT(randFileStatus);  

   while(individu > 0 && iter < NBGENERATION)
   {
      nbDes = 0;

      for(ind = 1; ind <= individu; ind++)
      {
         nbDes = nbDes + poisson();
      }

      individu = nbDes;
      iter++;
   }
  
   delete []randFileStatus;

   delete mtGen;

   return individu;
}
