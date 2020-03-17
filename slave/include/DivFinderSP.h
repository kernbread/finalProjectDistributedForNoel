#ifndef DIVFINDERSP_H
#define DIVFINDERSP_H

#include <string>
#include "DivFinder.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/math/common_factor.hpp>
#include <atomic>

// If factor has been called on a number x times, check to see if the number is prime
const unsigned int primecheck_depth = 10;

/******************************************************************************************
 * DivFinderSP - Used as a recursive calculator for prime numbers using Pollards Rho algorithm.
 *            A simple, recursive single process/thread version.
 *
 *  	   DivFinder(Const): 
 *  	   ~DivFinder(Dest):
 *
 *         PolRho - function that executes the stochastic Pollards' Rho algorithm for finding
 *                  divisors
 *         ifPrimeBF - uses brute force using 6k+-1 optimization to determine if a number is prime
 *
 *  	   Exceptions: sub-classes should throw a std::exception with the what string field
 *  	               populated for any issues.
 *
 *****************************************************************************************/

class DivFinderSP : public DivFinder { 
   public:
      DivFinderSP(LARGEINT input_value);
      virtual ~DivFinderSP();

      virtual void PolRho(std::list<LARGEINT> &prime_factors) override;

      bool isPrimeBF(LARGEINT n, LARGEINT &divisor);


   protected:
      void factor();
      void factor(LARGEINT n);

      


   private:

};

#endif
