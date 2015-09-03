//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Application to convert decimal degrees, to degrees, minutes,
// seconds (DMS).
//
//----------------------------------------------------------------------------
// $Id$

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <sstream>

using namespace std;

void getDegrees(double& deg)
{
   std::string s;
   cout << "Enter degrees:  " << flush;
   cin >> s;
   if ( (s == "q") || (s == "Q") )
   {
      exit(0);
   }
   else
   {
      istringstream is(s);
      is >> deg;
      if( is.fail() )
      {
         cerr << "Must enter a valid number between -180 and 180 for degrees."
              << "  Exiting..." << endl;
         exit(1);
      }
      else if ( (deg < -180.0) || (deg > 180.0) ) // Check the range.
      {
         cerr << "Degrees must be between -180 and 180.  Exiting..."
              << endl;
         exit(1);
      }
   }
}
   
int main(int argc, char* argv[])
{
   enum
   {
      FOREVER=1
   };

   if (argc != 1)
   {
      cout << "Usage: " << argv[0]
           << "\nThis application will convert decimal degrees to degrees, "
           << "minutes, seconds.\nIt take no arguments, you will be prompted "
           << "for the decimal degrees.\n" << endl;
      return 0;
   }
   
   cout << "\nEnter \"q\" to quit.\n\n";

   double deg = 0.0;
   double min = 0.0;
   double sec = 0.0;
   int degInt = 0;
   int minInt = 0;

   cout << setiosflags(ios::fixed) << setprecision(15);

   while (FOREVER)
   {
      // Get the degrees.
      getDegrees(deg);

      // Compute the answer.
      
      bool isNegative = (deg < 0.0);

      if (isNegative)
      {
         deg = fabs(deg);
      }

      degInt = static_cast<int>(deg);
      
      min = (deg - degInt) * 60.0;

      minInt = static_cast<int>(min);
      
      sec = (min - minInt) * 60.0;

      if (isNegative)
      {
         deg = -deg;
      }
      cout << deg << " = "
           << (isNegative?"-":"") << degInt << " degrees, "
           << minInt << " minutes, "
           << sec << " seconds."
           << endl;
   }
      
   return 0;
}
