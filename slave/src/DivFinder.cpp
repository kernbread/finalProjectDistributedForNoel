#include "DivFinder.h"
#include <cstdlib>
#include "config.h"

DivFinder::DivFinder(LARGEINT number):_orig_val(number) {
}

DivFinder::~DivFinder() {
}

void DivFinder::setVerbose(int lvl) {
   if ((lvl < 0) || (lvl > 3))
      throw std::runtime_error("Attempt to set invalid verbosity level. Lvl: (0-3)\n");
   verbose = lvl;
}

/********************************************************************************************
 * modularPow - function to gradually calculate (x^n)%m to avoid overflow issues for
 *              very large non-prime numbers using the stl function pow (floats)
 *
 *    Params:  base - the base value x in the above function
 *             exponent - the exponent n
 *             modulus - the modulus m
 *
 *    Returns: resulting number
 ********************************************************************************************/
LARGEINT2X DivFinder::modularPow(LARGEINT2X base, int exponent, LARGEINT2X modulus) {
   LARGEINT2X result = 1;

   while (exponent > 0) {

      // If the exponent is odd, calculate our results 
      if (exponent & 1) {
         result = (result * base) % modulus;
      }

      // exponent = exponent / 2 (rounded down)
      exponent = exponent >> 1;

      base = (base * base) % modulus;
   }
   return result;

}

/**********************************************************************************************
 * calcPollardsRho - Do the actual Pollards Rho calculations to attempt to find a divisor
 *
 *    Params:  n - the number to find a divisor within
 *    
 *    Returns: a divisor if found, otherwise n
 *
 *
 **********************************************************************************************/

LARGEINT DivFinder::calcPollardsRho(LARGEINT n) {
   if (n <= 3)
      return n;

   // Initialize our random number generator
   srand(time(NULL));

   // pick a random number from the range [2, N)
   LARGEINT2X x = (rand()%(n-2)) + 2;
   LARGEINT2X y = x;    // Per the algorithm

   // random number for c = [1, N)
   LARGEINT2X c = (rand()%(n-1)) + 1;

   LARGEINT2X d = 1;

   // Loop until either we find the gcd or gcd = 1
   while (d == 1) {
      if(checkBool()){
         return 0;
      }
      // "Tortoise move" - Update x to f(x) (modulo n)
      // f(x) = x^2 + c f
      x = (modularPow(x, 2, n) + c + n) % n;

      // "Hare move" - Update y to f(f(y)) (modulo n)
      y = (modularPow(y, 2, n) + c + n) % n;
      y = (modularPow(y, 2, n) + c + n) % n;

      // Calculate GCD of |x-y| and tn
      LARGESIGNED2X z = (LARGESIGNED2X) x - (LARGESIGNED2X) y;
      if (z < 0)
         d = boost::math::gcd((LARGEINT2X) -z, (LARGEINT2X) n);
      else
         d = boost::math::gcd((LARGEINT2X) z, (LARGEINT2X) n);

      // If we found a divisor, factor primes out of each side of the divisor
      if ((d != 1) && (d != n)) {
         return (LARGEINT) d;

      }

   }
   return (LARGEINT) d;
}


void DivFinder::combinePrimes(std::list<LARGEINT> &dest) {
   dest.insert(dest.end(), primes.begin(), primes.end());
}


void DivFinder::clean_up(){
   primes.clear();
   cancel_bool = false;
}
void DivFinder::cancel_op(){
   cancel_bool = true;
}
bool DivFinder::checkBool(){
   return cancel_bool;
}
