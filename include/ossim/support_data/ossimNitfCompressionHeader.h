//*******************************************************************
// Copyright (C) 2004 Garrett Potts
//
// LICENSE: MIT
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#ifndef ossimNitfCompressionHeader_HEADER
#define ossimNitfCompressionHeader_HEADER 1

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimIosFwd.h>
#include <ossim/base/ossimString.h>

#include <iosfwd>
#include <string>

class OSSIM_DLL ossimNitfCompressionHeader : public ossimObject
{
public:
   virtual void parseStream(ossim::istream& in) = 0;

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const=0;
   
   virtual bool saveState(ossimKeywordlist& kwl, const ossimString& prefix="")const;
protected:

TYPE_DATA;   
};
#endif
