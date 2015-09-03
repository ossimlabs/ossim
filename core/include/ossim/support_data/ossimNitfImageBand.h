//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfImageBand.h 22925 2014-10-28 22:01:09Z dburken $
#ifndef ossimNitfImageBand_HEADER
#define ossimNitfImageBand_HEADER 1
#include <iosfwd>
#include <string>
#include <ossim/support_data/ossimNitfImageLut.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>

class ossimNitfImageBand : public ossimObject
{
public:
   ossimNitfImageBand();
   virtual ~ossimNitfImageBand();
   virtual void parseStream(std::istream& in)=0;
   virtual void writeStream(std::ostream& out)=0;

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param prefix Like "image0."
    * @param band zero based band.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix,
                               ossim_uint32 band)const=0;

   virtual ossim_uint32 getNumberOfLuts()const=0;
   virtual const ossimRefPtr<ossimNitfImageLut> getLut(ossim_uint32 idx)const=0;
   virtual ossimRefPtr<ossimNitfImageLut> getLut(ossim_uint32 idx)=0;

   /** @return The band representation as an ossimString. */
   virtual ossimString getBandRepresentation() const = 0;

TYPE_DATA;
};

#endif /* #ifndef ossimNitfImageBand_HEADER */
