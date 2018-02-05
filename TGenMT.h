/* -------------------------------------------------------------*/
/* C code for Matsumoto & Nishimura - Mersene Twister           */
/* This file is updated by DAO Van Toan - May 2016              */ 
/* from the updates developped version adapted by David. Hill   */
/* -------------------------------------------------------------*/
void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
void saveStatus(char * inFileName);
void restoreStatus(char * inFileName);
double mtRand(void);
void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void);
double genrand_real1(void);
double genrand_real2(void);
