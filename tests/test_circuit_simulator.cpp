#include <cmath>
#include <iostream>

#include "CSimulator.h"

int main(int argc, char * argv[])
{

      // dummy vector. Replace in your code with the actual circuit vector.
      int vec1[] = {0, 1, 2, 3, 4, 5, 6, 0, 1, 1, 2, 2, 3, 0, 1, 0}  ;

      // dummy vector. Replace in your code with the actual circuit vector.

      int vec2[] = {0, 2, 4, 6, 0, 2, 4, 1, 3, 0, 5, 5, 3, 5, 0, 6 }  ;

      // Test value based on stub circuit_performance function.
      // Replace with actual performance value.

      std::cout << "circuit_performance(16, vec1) close to 29200.0 :\n";
      double result = circuit_performance(16, vec1);
      std::cout << "circuit_performance(16, vec1) = "<< result <<"\n";

      if (std::fabs(result - 29200.0)<1.0e-8) {
                  std::cout << "pass\n";
            } else {
	        std::cout << "fail\n";
              return 1;
           }

      // Test value based on stub circuit_performance function.
      // Replace with actual performance value.
      
      std::cout << "circuit_performance(16, vec2) close to 29300.0:\n";
      result =  circuit_performance(16, vec2);
      std::cout << "circuit_performance(16, vec2) = "<< result<<"\n";
      if (std::fabs(result - 29300.0)<1.0e-8)
	        std::cout << "pass\n";
      else
            { 
	        std::cout << "fail";
              return 1;
            }
	
}
