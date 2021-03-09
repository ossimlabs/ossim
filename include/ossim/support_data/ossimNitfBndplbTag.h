//---
//
// License: MIT
//
// Author:  David Burken
//
// Description: BNDPLB tag class declaration.
//
// Reference documents:
// - MIL-PRF-32466A, Appendix C, C.2.1.7
// - Digital Geographic Information Exchange Standard (DIGEST),
//   Part 2 - Annex D, D1.2.7.7 BNDPL - Bounding Polygon
//
//---
// $Id$

#ifndef ossimNitfBndplbTag_H
#define ossimNitfBndplbTag_H 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIosFwd.h>
#include <string>
#include <vector>

class ossimDpt;

class OSSIM_DLL ossimNitfBndplbTag : public ossimNitfRegisteredTag
{
public:
   enum 
   {
      NUM_PTS_SIZE = 4,
      PT_SIZE      = 15
   };
   
   /** default constructor */
   ossimNitfBndplbTag();
  
   /**
    * Parse method.
    *
    * @param in Stream to parse.
    */
   void parseStream(ossim::istream& in) final;
   
   /**
    * Write method.
    *
    * @param out Stream to write to.
    */
   void writeStream(ossim::ostream& out) final;
   
   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix) const final;

   ossim_uint32 getNumberOfPoints() const;
   
protected:

   /**
    * Clears all string fields within the record to some default nothingness.
    */
   void clearFields();

   /**
    * FIELD: NUM_PTS
    *
    * required 4 byte field, BCS-N positive integer, 00124 TO 99964
    *
    * Number of points in bounding polygon:
    * This field shall contain the number of points (coordinate pairs)
    * constituting the bounding polygon. The first and last points shall be
    * the same. Coordinate values shall refer to the coordinate system and
    * units defined in GEOPS (and possibly in PRJPS).
    */
   char m_numPts[NUM_PTS_SIZE+1];
   
   /**
    * FIELD: LON
    * 
    * Required 15 byte field, BCS-N recurring for each point.
    *
    * Longitude/Easting: 
    * This field shall contain the easting (when the value of GEOPS.UNI is M) or
    * longitude (otherwise) of the nth bounding polygon.
    */
   std::vector<std::string> m_lon;

   /**
    * FIELD: LAT
    * 
    * Required 15 byte field, BCS-N recurring for each point.
    *
    * Latitude/Northing:
    * This field shall contain the northing (when the value of GEOPS.UNI is M)
    * or latitude(otherwise) of the nth bounding polygon.
    */
   std::vector<std::string> m_lat;
   
TYPE_DATA   
};

#endif /* End of "#ifndef ossimNitfBndplbTag_H" */
