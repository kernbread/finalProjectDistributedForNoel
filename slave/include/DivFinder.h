#ifndef POLRHO_H
#define POLRHO_H

#include <list>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>
#include <atomic>
#include "config.h"

using namespace boost::multiprecision;

/******************************************************************************************
 * DivFinder - Parent class for a set of single-process and multithreaded methods for finding
 *             prime numbers
 *
 *  	   DivFinder(Const):  Takes in an input value of LARGEINT (defined in configure.ac as
 *  	                      something like uint128_t)
 *  	   
 *  	   ~PFactors(Dest):  Doesn't do much currently
 *
 *  	   Exceptions: sub-classes should throw a std::exception with the what string field
 *  	               populated for any issues.
 *
 *****************************************************************************************/

class DivFinder { 
   public:
      DivFinder(LARGEINT input_value);
      virtual ~DivFinder();

      // Overload me 
      virtual void PolRho(std::list<LARGEINT> &prime_factors) = 0;

      LARGEINT getOrigVal() { return _orig_val; }

      virtual void combinePrimes(std::list<LARGEINT> &dest);
      LARGEINT calcPollardsRho(LARGEINT n);

      void setVerbose(int lvl);

      void cancel_op();

   protected:

      LARGEINT2X modularPow(LARGEINT2X base, int exponent, LARGEINT2X modulus);

      std::list<LARGEINT> primes;
		
      int verbose = 0;
      
      bool checkBool();
      void clean_up();
      std::atomic<bool> cancel_bool{false};

      // Do not forget, your constructor should call this constructor

   private:
      DivFinder() {}; // Prevent instantiation without calling initialization 

      LARGEINT _orig_val;

      // Stuff to be left alone
};

#endif
