//*******************************************************************
// OSSIM
//
// License:  See top level LICENSE.txt file.
//
//*******************************************************************
//  $Id: ossim-equation.cpp 23407 2015-07-06 15:59:23Z okramer $
#include <iostream>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/util/ossimEquationUtil.h>

using namespace std;

int main(int argc, char *argv[])
{
	ossimArgumentParser argParser(&argc, argv);
	ossimInit::instance()->addOptions(argParser);
	ossimInit::instance()->initialize(argParser);

   // Run:
	ossimEquationUtil equationUtil (argParser);
   bool success = equationUtil.execute();

   if (success)
      exit(0);
   exit(1);
}
