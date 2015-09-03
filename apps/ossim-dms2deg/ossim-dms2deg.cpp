//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Application to convert degrees, minutes seconds (DMS) 
// to decimal degrees.
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
   
void getMinutes(double& min)
{
   std::string s;
   cout << "Enter minutes:  " << flush;
   cin >> s;
   if ( (s == "q") || (s == "Q") )
   {
      exit(0);
   }
   else
   {
      istringstream is(s);
      is >> min;
      if(is.fail())
      {
         cerr << "Must enter a valid number between 0 and 60 for minutes."
              << "  Exiting..." << endl;
         exit(1);
      }
      else if ( (min < 0.0) || (min > 60.0) ) // Check the range.
      {
         cerr << "Minutes must be between 0 and 60.  Exiting\n";
         exit(1);
      }
   }
}

void getSeconds(double& sec)
{
   std::string s;
   cout << "Enter seconds:  " << flush;
   cin >> s;
   if ( (s == "q") || (s == "Q") )
   {
      exit(0);
   }
   else
   {
      istringstream is(s);
      is >> sec;
      if( is.fail() )
      {
         cerr << "Must enter a valid number between 0 and 60 for seconds."
              << "  Exiting..." << endl;
         exit(1);
      }
      else if ( (sec < 0.0) || (sec > 60.0) ) // Check the range. 
      {
         cerr << "Seconds must be between 0 and 60.  Exiting\n";
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
           << "\nThis application will convert degrees, minutes, seconds to "
           << "decimal degrees.\nIt take no arguments, you will be prompted "
           << "for the degrees, minutes, seconds.\n" << endl;
      return 0;
   }

   
   cout << "\nEnter \"q\" to quit.\n\n";

   double deg = 0.0;
   double min = 0.0;
   double sec = 0.0;

   cout << setiosflags(ios::fixed) << setprecision(15);

   while (FOREVER)
   {
      // Get the degrees.
      getDegrees(deg);

      // Get the minutes.
      getMinutes(min);

      // Get the the seconds.
      getSeconds(sec);

      // Compute the answer.
      if (deg < 0.0)
      {
         double d = fabs(deg);
         deg = -(d + min/60.0 + sec/3600.0);
      }
      else
      {
         deg = deg + min/60.0 + sec/3600.0;
      }

      if ( deg < -180.0 )
      {
         deg += 360.0;
      }
      else if ( deg > 180.0 )
      {
         deg -= 360.0;
      }
      cout << "Decimal degrees = " << deg << "\n" << endl;
      
   }
      
   return 0;
}
