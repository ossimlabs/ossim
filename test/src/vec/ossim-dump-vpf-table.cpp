//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Does a table dump on any vpf table.
//
//********************************************************************
// $Id: ossim-dump-vpf-table.cpp 23495 2015-08-28 15:25:19Z okramer $
#include <ossim/vec/ossimVpfTable.h>
#include <ossim/base/ossimErrorCodes.h>
#include <iostream>

using namespace std;

void displayUsage()
{
   cout << "usage:\ndump_table <full path to table>" << endl;
}

int main(int argc, char *argv[])
{
   if(argc == 2)
   {
      ossimVpfTable table;
   
      if(table.openTable(argv[1]) == ossimErrorCodes::OSSIM_OK)
      {
         cout << table << endl;
      }
   }
   else
   {
      displayUsage();
   }
}
