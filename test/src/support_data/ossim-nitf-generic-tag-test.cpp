//----------------------------------------------------------------------------
//
// File ossim-fgdc-txt-doc-test.cpp
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: Test for the generic nitf tags
//
//----------------------------------------------------------------------------
// $Id:

#include <ossim/support_data/ossimNitfCsexrbTag.h>
#include <ossim/support_data/ossimNitfCssfabDes.h>
#include <ossim/support_data/ossimNitfCsattbDes.h>
#include <ossim/support_data/ossimNitfBandsbTag.h>

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/init/ossimInit.h>

#include <iostream>
#include <sstream>

#include "support_data/ossimNitfCephbDes.h"
#include "support_data/ossimNitfXmlTag.h"

using namespace std;

int main(int argc, char *argv[])
{
   cout << "Hello World! Courtesy of OSSIM." << endl;
   ossimNitfCsexrbTag csexrb = ossimNitfCsexrbTag();
   ossimNitfCssfabDes cssfab = ossimNitfCssfabDes();
   ossimNitfCsattbDes csattb = ossimNitfCsattbDes();
   ossimNitfCsephbDes csephb = ossimNitfCsephbDes();
   ossimNitfBandsbTag bandsb = ossimNitfBandsbTag();
   ossimString ossimHome = getenv("OSSIM_HOME");
   ossimNitfXmlTag sorbxa = ossimNitfXmlTag("SORBXA");
   ossimNitfXmlTag soddxa = ossimNitfXmlTag("SODDXA");

   string defaultHeader(36 + 3 + + 3 + 4, '0');
   istringstream stream (defaultHeader+ "12311000.02000000020240607235000.30000000000002-04650413.20+01029613.77+05110465.09-04651390.88+01030212.19+05109457.17000000157011000000145Y-00004888.95+00002992.17-00005039.01-00000002.54+00000007.21-00000000.37-00004887.78+00002992.08-00005040.21-00000002.53+00000007.23-00000000.36");
   csephb.parseStream(stream);
   csephb.print(std::cout, "");
   stream.clear();
   stream.str(defaultHeader + "12311000.02000000020240607235000.30000000000002-0.180248530324575-0.838233121509416-0.409883760738082-0.311241070560455-0.180394839854912-0.838122329325916-0.409819429670391-0.311539239749436000000000");
   csattb.parseStream(stream);
   csattb.print(std::cout, "");
   return 0;
}
