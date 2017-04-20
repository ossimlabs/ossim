//---
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Container class for J2K "Start Of Tile" (SOT) record.
//
// marker: FF90
//
// See document BPJ2K01.00 Table 7-3 Image and tile size (15444-1 Annex A.4.2)
// 
//---
// $Id$

#include <ossim/support_data/ossimJ2kSotRecord.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>

#include <iostream>
#include <iomanip>

ossimJ2kSotRecord::ossimJ2kSotRecord()
   :
   theLsot(0),
   theIsot(0),
   thePsot(0),
   theTpsot(0),
   theTnsot(0)
{
}

ossimJ2kSotRecord::~ossimJ2kSotRecord()
{
}

void ossimJ2kSotRecord::parseStream(std::istream& in)
{
   // Note: marker not read...
   
   in.read((char*)&theLsot,  2);
   in.read((char*)&theIsot,  2);
   in.read((char*)&thePsot,  4);
   in.read((char*)&theTpsot, 1);
   in.read((char*)&theTnsot, 1);

   if (ossim::byteOrder() == OSSIM_LITTLE_ENDIAN)
   {
      // Stored big endian, must swap.
      ossimEndian endian;
      endian.swap(theLsot);
      endian.swap(theIsot);
      endian.swap(thePsot);
   }
}

void ossimJ2kSotRecord::writeStream(std::ostream& out)
{
   ossimEndian* endian = 0;
   if (ossim::byteOrder() == OSSIM_LITTLE_ENDIAN)
   {
      // Stored in file big endian, must swap.
      endian = new ossimEndian();
      endian->swap( theLsot );
      endian->swap( theIsot );
      endian->swap( thePsot );      
   }
   
   // Marker 0xff90:
   out.put( 0xff );
   out.put( 0x90 );
   
   out.write( (char*)&theLsot,  2);
   out.write( (char*)&theIsot,  2);
   out.write( (char*)&thePsot,  4);
   out.write( (char*)&theTpsot, 1);
   out.write( (char*)&theTnsot, 1);   
      
   if ( endian )
   {
      // Swap back to native:
      endian->swap(theLsot);
      endian->swap(theIsot);
      endian->swap(thePsot);

      delete endian;
      endian = 0;
   }  
}

void ossimJ2kSotRecord::setIsot( ossim_uint16 isot )
{
   theIsot = isot;
}

std::ostream& ossimJ2kSotRecord::print(std::ostream& out,
                                       const std::string& prefix) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = out.flags();

   std::string pfx = prefix;
   pfx += "sot.";

   out.setf(std::ios_base::hex, std::ios_base::basefield);
   out << pfx << "marker: 0xff90\n";
   out.setf(std::ios_base::fmtflags(0), std::ios_base::basefield);

   out << pfx << "Lsot:   "  << theLsot      << "\n"
       << pfx << "Isot:   "  << theIsot      << "\n"
       << pfx << "Psot:   "  << thePsot      << "\n"
       << pfx << "Tpsot:  " << int(theTpsot) << "\n"
       << pfx << "Tnsot:  " << int(theTnsot)
       << std::endl;

   // Reset flags.
   out.setf(f);

   return out;
}

std::ostream& operator<<(std::ostream& out, const ossimJ2kSotRecord& obj)
{
   return obj.print(out);
}
