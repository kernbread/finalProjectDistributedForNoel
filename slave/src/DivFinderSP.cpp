#include "DivFinderSP.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/integer/common_factor.hpp>

DivFinderSP::DivFinderSP(LARGEINT number):DivFinder(number) {
}

DivFinderSP::~DivFinderSP() {
}

// The following defines can be found in config.h and can be adjusted based on
// need. uint32_t and uint64_t are easier to debug but not big enough for challenging
// problems. LARGEINT2X should be twice as big as LARGEINT and LARGESIGNED2X should
// hold a LARGEINT2X in its positive section (so uint256_t would need int512_t)

/* "Unsigned int type to hold original value and calculations" */
#define LARGEINT uint128_t

/* "Unsigned int twice as large as LARGEINT (bit-wise)" */
#define LARGEINT2X uint256_t

/* "Signed int made of twice the bits as LARGEINT2X" */
#define LARGESIGNED2X int512_t

void DivFinderSP::PolRho(std::list<LARGEINT> &prime_factors){
   primes.clear();
   DivFinder::setVerbose(3);
   factor();
   if(checkBool()){
      clean_up();
      return;
   }
   combinePrimes(prime_factors);
   clean_up();

   return;
}

/*******************************************************************************
 *
 * isPrimeBF - Uses a simple brute force primality test with 6k +/- 1 optimization
 *             
 *
 *    Params:  n - the number to test for prime
 *             divisor - return value of the discovered divisor if not prime
 *
 *    Returns: true if prime, false otherwise
 *
 *******************************************************************************/

bool DivFinderSP::isPrimeBF(LARGEINT n, LARGEINT &divisor) {
   if (verbose >= 3)
      std::cout << "Checking if prime: " << n << std::endl;

   divisor = 0;

   // Take care of simple cases
   if (n <= 3) {
      return n > 1;
   }
   else if ((n % 2) == 0) {
      divisor = 2;
      return false;
   } else if ((n & 3) == 0) {
      divisor = 3;
      return false;
   }

   // Assumes all primes are to either side of 6k. Using 256 bit to avoid overflow
   // issues when calculating max range
   LARGEINT2X n_256t = n;
   for (LARGEINT2X k=5; k * k < n_256t; k = k+6) {
      if ((n_256t % k == 0) || (n_256t % (k+2) == 0)) {
         divisor = (LARGEINT) k;
         return false;
      }
   }
   return true;
}

/*******************************************************************************
 *
 * factor - Calculates a single prime of the given number and recursively calls
 *          itself to continue calculating primes of the remaining number. Variation
 *          on the algorithm by Yash Varyani on GeeksForGeeks. Uses a single
 *          process 
 *
 *
 ******************************************************************************/

void DivFinderSP::factor() {

   // First, take care of the '2' factors
   LARGEINT newval = getOrigVal();
   while (newval % 2 == 0) {
      primes.push_back(2);
      if (verbose >= 2)
         std::cout << "Prime Found: 2\n";
      newval = newval / 2;
   } 

   // Now the 3s
   while (newval % 3 == 0) {
      primes.push_back(3);
      std::cout << "Prime Found: 3\n";
      newval = newval / 3;
   }

   // Now use Pollards Rho to figure out the rest. As it's stochastic, we don't know
   // how long it will take to find an answer. Should return the final two primes
   factor(newval);
   
}

/*******************************************************************************
 *
 * factor - same as above function, but can be iteratively called as numbers are
 *          discovered as the number n can be passed in
 *
 *
 ******************************************************************************/

void DivFinderSP::factor(LARGEINT n) {

   // already prime
   if (n == 1) {
      return;
   }

   if (verbose >= 2)
      std::cout << "Factoring: " << n << std::endl;

   bool div_found = false;
   unsigned int iters = 0;

   while (!div_found) {
      if(checkBool())
         return;
      if (verbose >= 3)
         std::cout << "Starting iteration: " << iters << std::endl;

      // If we have tried Pollards Rho a specified number of times, run the
      // costly prime check to see if this is a prime number. Also, increment
      // iters after the check
      if (iters++ == primecheck_depth) {
         if (verbose >= 2)
	         std::cout << "Pollards rho timed out, checking if the following is prime: " << n << std::endl;
         LARGEINT divisor;
         if (isPrimeBF(n, divisor)) {
	         if (verbose >= 2)
	            std::cout << "Prime found: " << n << std::endl;
            primes.push_back(n);
	         return;
	      } else {   // We found a prime divisor, save it and keep finding primes
	         if (verbose >= 2)
	            std::cout << "Prime found: " << divisor << std::endl;
	         primes.push_back(divisor);
	         return factor(n / divisor);
	      }				
      }

      // We try to get a divisor using Pollards Rho
      LARGEINT d = calcPollardsRho(n);
      if(checkBool())
         return;
      if (d != n) {
         if (verbose >= 1)
            std::cout << "Divisor found: " << d << std::endl;

         // Factor the divisor
         factor(d);

         // Now the remaining number
         factor((LARGEINT) (n/d));
         return;
      }

      // If d == n, then we re-randomize and continue the search up to the prime check depth
   }
   throw std::runtime_error("Reached end of function--this should not have happened.");
   return;
}


