#include "TGenMT.hpp"
using namespace std;

class simuPo {
   public:
            simuPo();
            simuPo(double lam, int id);
            void   setSimu(double lamda, int id);
            double getLamd();
            int    getId();
            unsigned long long int poisson();
            unsigned long long int growPo();
   private:
            double    lambda;
            int       idSimu;
            TGenMT  * mtGen;
};
