//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Base class for Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/base/ossimKeywordlist.h>
#include <sstream>

ossimInfoBase::ossimInfoBase()
   : theOverviewFlag(true)
{}

ossimInfoBase::~ossimInfoBase()
{}

void ossimInfoBase::setProcessOverviewFlag(bool flag)
{
   theOverviewFlag = flag;
}

bool ossimInfoBase::getProcessOverviewFlag() const
{
   return theOverviewFlag;
}

bool ossimInfoBase::getKeywordlist(ossimKeywordlist& kwl)const
{  
   // Do a print to a memory stream.
   std::ostringstream out;
   print(out);

   std::istringstream in(out.str());
   // Give the result to the keyword list.
   return kwl.parseStream(in);
}

bool ossimInfoBase::getKeywordlist(ossimKeywordlist& kwl,
                                   ossim_uint32 /* entryIndex */ )const
{
   // If this gets hit the specific info object does not support entry indexes.
   return getKeywordlist( kwl );
}
