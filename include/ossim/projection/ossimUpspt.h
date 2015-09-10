//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts(gpotts@imagelinks.com)
//
// Description:
//
// This is the Universal Polar Stereographic (UPS) point.  Allows
// easy onversion between different coordinates.
//*******************************************************************
//  $Id: ossimUpspt.h 23353 2015-06-01 19:19:10Z dburken $

#ifndef ossimUpspt_HEADER
#define ossimUpspt_HEADER 1

#include <ossim/base/ossimConstants.h>

class ossimUtmpt;
class ossimGpt;
class ossimEcefPoint;
class ossimDatum;

class OSSIM_DLL ossimUpspt
{
public:
   ossimUpspt(const ossimGpt &aPt);
   ossimUpspt(const ossimEcefPoint &aPt);

   char hemisphere()const{return theHemisphere;}
   double easting()const {return theEasting;}
   double northing()const{return theNorthing;}
   const ossimDatum* datum()const{return theDatum;}
   
private:
   char    theHemisphere;
   double  theEasting;
   double  theNorthing;
   ossimDatum  *theDatum;

   void convertFromGeodetic(const ossimGpt &aPt);
};

#endif
