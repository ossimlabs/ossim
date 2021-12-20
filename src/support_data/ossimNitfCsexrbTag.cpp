//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Scott Bortman
//
// Description: CSEXRB tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document  STDI-0002-1 Appendix AH: GLAS/GFM 1.0for more info.
// 
//----------------------------------------------------------------------------
// $Id

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>

#include <ossim/support_data/ossimNitfCsexrbTag.h>

using namespace std;


RTTI_DEF1(ossimNitfCsexrbTag, "ossimNitfCsexrbTag", ossimNitfRegisteredTag);

ossimNitfCsexrbTag::ossimNitfCsexrbTag()
   : ossimNitfRegisteredTag(std::string("CSEXRB"), 11)
{
   clearFields();


}

ossimNitfCsexrbTag::~ossimNitfCsexrbTag()
{
}

void ossimNitfCsexrbTag::parseStream(std::istream& in)
{
   clearFields();

   ossim_uint32 byteCount = 0;

   byteCount = sizeof(theImageUUID);
   in.read(theImageUUID, byteCount);
   m_tagLength += byteCount;

   byteCount = sizeof(theNumAssocDES);
   in.read(theNumAssocDES, byteCount);
   m_tagLength += byteCount;

   ossim_uint32 desCount = atoi(theNumAssocDES);

   for ( int i = 0; i < desCount; i++ ) {
      char assocDes[37];

      byteCount = sizeof(assocDes);
      in.read(assocDes, byteCount);
      theAssocDES.push_back(assocDes);
      m_tagLength += byteCount;
   }

#if 0
   in.read(theSensor, 6);
   in.read(theTileFirstLine, 12);
   in.read(theImageTimeDuration, 12);
   in.read(theMaxGsd, 5);
   in.read(theAlongScanGsd, 5);
   in.read(theCrossScanGsd, 5);
   in.read(theGeoMeanGsd, 5);
   in.read(theAlongScanVertGsd, 5);
   in.read(theCrossScanVertGsd, 5);
   in.read(theGeoMeanVertGsd, 5);
   in.read(theGeoBetaAngle, 5);
   in.read(theDynamicRange, 5);
   in.read(theLine, 7);
   in.read(theSamples, 5);
   in.read(theAngleToNorth, 7);
   in.read(theObliquityAngle, 6);
   in.read(theAzOfObliquity, 7);
   in.read(theGrdCover, 1);
   in.read(theSnowDepthCategory, 1);
   in.read(theSunAzimuth, 7);
   in.read(theSunElevation, 7);
   in.read(thePredictedNiirs, 3);
   in.read(theCircularError, 3);
   in.read(theLinearError, 3);
#endif   
}

void ossimNitfCsexrbTag::writeStream(std::ostream& out)
{
   out.write(theImageUUID, sizeof(theImageUUID));
   out.write(theNumAssocDES, sizeof(theNumAssocDES));

   ossim_uint32 desCount = atoi(theNumAssocDES);

   for ( int i = 0; i < desCount; i++ ) {
      out.write(theAssocDES[i], sizeof(theImageUUID));
   }   
#if 0   
   out.write(theSensor, 6);
   out.write(theTileFirstLine, 12);
   out.write(theImageTimeDuration, 12);
   out.write(theMaxGsd, 5);
   out.write(theAlongScanGsd, 5);
   out.write(theCrossScanGsd, 5);
   out.write(theGeoMeanGsd, 5);
   out.write(theAlongScanVertGsd, 5);
   out.write(theCrossScanVertGsd, 5);
   out.write(theGeoMeanVertGsd, 5);
   out.write(theGeoBetaAngle, 5);
   out.write(theDynamicRange, 5);
   out.write(theLine, 7);
   out.write(theSamples, 5);
   out.write(theAngleToNorth, 7);
   out.write(theObliquityAngle, 6);
   out.write(theAzOfObliquity, 7);
   out.write(theGrdCover, 1);
   out.write(theSnowDepthCategory, 1);
   out.write(theSunAzimuth, 7);
   out.write(theSunElevation, 7);
   out.write(thePredictedNiirs, 3);
   out.write(theCircularError, 3);
   out.write(theLinearError, 3);
#endif   
}

void ossimNitfCsexrbTag::clearFields()
{
   //---
   // No attempt made to set to defaults.
   // BCS-N's to '0's, BCS-A's to ' '(spaces)
   //---

   ossim_uint32 byteCount = 0;

   byteCount = sizeof(theImageUUID);
   memset(theImageUUID, ' ',  byteCount);
   theImageUUID[byteCount] = '\0';

   byteCount = sizeof(theNumAssocDES);
   memset(theNumAssocDES, ' ',  byteCount);
   theNumAssocDES[byteCount] = '\0';

   theAssocDES.clear();
#if 0
   memset(theSensor, ' ',  6);
   memset(theTileFirstLine, 0, 12);
   memset(theImageTimeDuration, 0, 12);
   memset(theMaxGsd, 0, 5);
   memset(theAlongScanGsd, ' ', 5);
   memset(theCrossScanGsd, ' ', 5);
   memset(theGeoMeanGsd, ' ', 5);
   memset(theAlongScanVertGsd, ' ', 5);
   memset(theCrossScanVertGsd, ' ', 5);
   memset(theGeoMeanVertGsd, ' ', 5);
   memset(theGeoBetaAngle, ' ', 5);
   memset(theDynamicRange, 0, 5);
   memset(theLine, 0, 7);
   memset(theSamples, 0, 5);
   memset(theAngleToNorth, 0, 7);
   memset(theObliquityAngle, 0, 6);
   memset(theAzOfObliquity, 0, 7);
   memset(theGrdCover, 0, 1);
   memset(theSnowDepthCategory, 0, 1);
   memset(theSunAzimuth, 0, 7);
   memset(theSunElevation, 0, 7);
   memset(thePredictedNiirs, ' ', 3);
   memset(theCircularError, 0, 3);
   memset(theLinearError, 0, 3);
#endif

#if 0
   theSensor[6] = '\0';
   theTileFirstLine[12] = '\0';
   theImageTimeDuration[12] = '\0';
   theMaxGsd[5] = '\0';
   theAlongScanGsd[5] = '\0';
   theCrossScanGsd[5] = '\0';
   theGeoMeanGsd[5] = '\0';
   theAlongScanVertGsd[5] = '\0';
   theCrossScanVertGsd[5] = '\0';
   theGeoMeanVertGsd[5] = '\0';
   theGeoBetaAngle[5] = '\0';
   theDynamicRange[5] = '\0';
   theLine[7] = '\0';
   theSamples[5] = '\0';
   theAngleToNorth[7] = '\0';
   theObliquityAngle[6] = '\0';
   theAzOfObliquity[7] = '\0';
   theGrdCover[1] = '\0';
   theSnowDepthCategory[1] = '\0';
   theSunAzimuth[7] = '\0';
   theSunElevation[7] = '\0';
   thePredictedNiirs[3] = '\0';
   theCircularError[3] = '\0';
   theLinearError[3] = '\0';
#endif   
}

std::ostream& ossimNitfCsexrbTag::print(
   std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"
       << getTagLength() << "\n"
       << pfx << std::setw(24) << "IMAGE_UUID:"
       << theImageUUID << "\n"
       << pfx << std::setw(24) << "NUM_ASSOC_DES:"
       << theNumAssocDES << "\n";

   ossim_uint32 desCount = atoi(theNumAssocDES);

   for ( int i = 0; i < desCount; i++ ) {
       out  << pfx << std::setw(24) << "ASSOC_DES_UUID:" << i 
            << theAssocDES[i] << "\n";
   }   

#if 0   
       << pfx << std::setw(24) << "SENSOR:"
       << theSensor << "\n"
       << pfx << std::setw(24) << "TIME_FIRST_LINE_IMAGE:"
       << theTileFirstLine << "\n"
       << pfx << std::setw(24) << "TIME_IMAGE_DURATION:"
       << theImageTimeDuration << "\n"
       << pfx << std::setw(24) << "MAX_GSD:"
       << theMaxGsd << "\n"
       << pfx << std::setw(24) << "ALONG_SCAN_GSD:"
       << theAlongScanGsd << "\n"
       << pfx << std::setw(24) << "CROSS_SCAN_GSD:"
       << theCrossScanGsd << "\n"
       << pfx << std::setw(24) << "GEO_MEAN_GSD:"
       << theGeoMeanGsd << "\n"
       << pfx << std::setw(24) << "A_S_VERT_GSD:"
       << theAlongScanVertGsd << "\n"
       << pfx << std::setw(24) << "C_S_VERT_GSD:"
       << theCrossScanVertGsd << "\n"
       << pfx << std::setw(24) << "GEO_MEAN_VERT_GSD:"
       << theGeoMeanVertGsd << "\n"
       << pfx << std::setw(24) << "GEO_BETA_ANGLE:"
       << theGeoBetaAngle << "\n"
       << pfx << std::setw(24) << "DYNAMIC_RANGE:"
       << theDynamicRange << "\n"
       << pfx << std::setw(24) << "NUM_LINES:"
       << theLine << "\n"
       << pfx << std::setw(24) << "NUM_SAMPLES:"
       << theSamples << "\n"
       << pfx << std::setw(24) << "ANGLE_TO_NORTH:"
       << theAngleToNorth << "\n"
       << pfx << std::setw(24) << "OBLIQUITY_ANGLE:"
       << theObliquityAngle << "\n"
       << pfx << std::setw(24) << "AZ_OF_OBLIQUITY:"
       << theAzOfObliquity << "\n"
       << pfx << std::setw(24) << "GRD_COVER:"
       << theGrdCover << "\n"
       << pfx << std::setw(24) << "SNOW_DEPTH_CAT:"
       << theSnowDepthCategory << "\n"
       << pfx << std::setw(24) << "SUN_AZIMUTH:"
       << theSunAzimuth << "\n"
       << pfx << std::setw(24) << "SUN_ELEVATION:"
       << theSunElevation << "\n"
       << pfx << std::setw(24) << "PREDICTED_NIIRS:"
       << thePredictedNiirs << "\n"
       << pfx << std::setw(24) << "CIRCL_ERR:"
       << theCircularError << "\n"
       << pfx << std::setw(24) << "LINEAR_ERR:"
       << theLinearError
   #endif
      std::cout << "\n";

   return out;
}

ossimString ossimNitfCsexrbTag::getImageUUID() const
{
   return ossimString(theImageUUID);
}

ossimString ossimNitfCsexrbTag::getNumAssocDES() const
{
   return ossimString(theNumAssocDES);
}

void ossimNitfCsexrbTag::addAssocDES(ossimString uuid) 
{
   theAssocDES.push_back(uuid);
}

#if 0
ossimString ossimNitfCsexrbTag::getSensor() const
{
   return ossimString(theSensor);
}
   
ossimString ossimNitfCsexrbTag::getTimeFirstLineImage() const
{
   return ossimString(theTileFirstLine);
}
   
ossimString ossimNitfCsexrbTag::getTimeImageDuration() const
{
   return ossimString(theImageTimeDuration);
}
   
ossimString ossimNitfCsexrbTag::getMaxGsd() const
{
   return ossimString(theMaxGsd);
}
   
ossimString ossimNitfCsexrbTag::getAlongScanGsd() const
{
   return ossimString(theAlongScanGsd);
}
   
ossimString ossimNitfCsexrbTag::getCrossScanGsd() const
{
   return ossimString(theCrossScanGsd);
}
   
ossimString ossimNitfCsexrbTag::getGeoMeanGsd() const
{
   return ossimString(theGeoMeanGsd);
}
   
ossimString ossimNitfCsexrbTag::getAlongScanVerticalGsd() const
{
   return ossimString(theAlongScanVertGsd);
}
   
ossimString ossimNitfCsexrbTag::getCrossScanVerticalGsd() const
{
   return ossimString(theCrossScanVertGsd);
}

ossimString ossimNitfCsexrbTag::getGeoMeanVerticalGsd() const
{
   return ossimString(theGeoMeanVertGsd);
}

ossimString ossimNitfCsexrbTag::getGeoBetaAngle() const
{
   return ossimString(theGeoBetaAngle);
}

ossimString ossimNitfCsexrbTag::getDynamicRange() const
{
   return ossimString(theDynamicRange);
}
   
ossimString ossimNitfCsexrbTag::getNumLines() const
{
   return ossimString(theLine);
}
   
ossimString ossimNitfCsexrbTag::getNumSamples() const
{
   return ossimString(theSamples);
}
   
ossimString ossimNitfCsexrbTag::getAngleToNorth() const
{
   return ossimString(theAngleToNorth);
}

ossimString ossimNitfCsexrbTag::getObliquityAngle() const
{
   return ossimString(theObliquityAngle);
}

ossimString ossimNitfCsexrbTag::getAzimuthOfObliquity() const
{
   return ossimString(theAzOfObliquity);
}
   
ossimString ossimNitfCsexrbTag::getGroundCover() const
{
   return ossimString(theGrdCover);
}

ossimString ossimNitfCsexrbTag::getSnowDepth() const
{
   return ossimString(theSnowDepthCategory);
}
   
ossimString ossimNitfCsexrbTag::getSunAzimuth() const
{
   return ossimString(theSunAzimuth);
}

ossimString ossimNitfCsexrbTag::getSunElevation() const
{
   return ossimString(theSunElevation);
}

ossimString ossimNitfCsexrbTag::getPredictedNiirs() const
{
   return ossimString(thePredictedNiirs);
}
   
ossimString ossimNitfCsexrbTag::getCE90() const
{
   return ossimString(theCircularError);
}

ossimString ossimNitfCsexrbTag::getLE90() const
{
   return ossimString(theLinearError);
}
#endif