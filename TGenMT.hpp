#pragma once

#include <string>

#define N 624
#define M 397

#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

class TGenMT {
        public:
                TGenMT();
                TGenMT(char * filename);
                TGenMT(unsigned long s);
                TGenMT(unsigned long init_key[], int key_length);


                double mtRand();
                unsigned long genrand_int32();
                double genrand_real1();
                double genrand_real2();
                void restoreStatus(char * inFileName);
                void saveStatus(char * inFileName);


        private:
                void init_genrand(unsigned long s);
                void init_by_array(unsigned long init_key[], int key_length);

                unsigned long mt[N]; /* the array for the state vector  */
                int mti; /* mti==N+1 means mt[N] is not initialized */
};
