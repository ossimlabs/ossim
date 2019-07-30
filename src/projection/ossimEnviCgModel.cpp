//*****************************************************************************
// FILE: ossimCoarseGridModel.cc
//
// License:  See LICENSE.txt file in the top level directory.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION:
//   Contains implementation of class ossimCoarseGridModel. This is an
//   implementation of an interpolation sensor model. 
//
//   IMPORTANT: The lat/lon grid is for ground points on the ellipsoid.
//   The dLat/dHgt and dLon/dHgt partials therefore are used against
//   elevations relative to the ellipsoid.
//
//*****************************************************************************
//  $Id: ossimCoarseGridModel.cpp 22825 2014-07-07 23:14:52Z dburken $

#include <ossim/projection/ossimEnviCgModel.h>

RTTI_DEF1(ossimEnviCgModel, "ossimEnviCgModel", ossimCoarseGridModel);

#include <ossim/elevation/ossimElevManager.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/support_data/ossimSupportFilesList.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/base/ossimKeywordlist.h>

using namespace std;

static const char* MODEL_TYPE = "ossimEnviCgModel";

//*****************************************************************************
ossimEnviCgModel::ossimEnviCgModel()
      : ossimCoarseGridModel()
{}

//*****************************************************************************
ossimEnviCgModel::ossimEnviCgModel(const ossimFilename& geoc_file)
   : ossimCoarseGridModel()
{
   loadEnviGeocFile(geoc_file);
}


//*****************************************************************************
bool ossimEnviCgModel::loadEnviGeocFile(const ossimFilename& geoc_file)
{
   theHeightEnabledFlag = false;
   setErrorStatus();

   ossimFilename geoc_hdr (geoc_file + ".hdr");

   ifstream geocHdrStream (geoc_hdr.c_str());
   if (geocHdrStream.fail())
   {
      cout << "ossimEnviCgModel:loadEnviGeocFile() Error encountered opening GEOC header file <"<<geoc_hdr<<">"<<endl;
      return false;
   }

   char line[1024];
   ossimIpt imageSize;
   geocHdrStream.getline(line, 1024);
   ossimString oline(line);
   int bigEndian=0;
   if (!oline.contains("ENVI"))
   {
      cout << "ossimEnviCgModel:loadEnviGeocFile() Error: GEOC header file <"<<geoc_hdr
           <<"> missing \"ENVI\" magic number."<<endl;
      return false;
   }
   while (!geocHdrStream.eof())
   {
      geocHdrStream.getline(line, 1024);
      oline = line;
      vector<ossimString> strSplit;
      if (oline.contains("samples"))
         imageSize.x = oline.after("=").trim().toInt();
      else if (oline.contains("lines"))
         imageSize.y = oline.after("=").trim().toInt();
      else if (oline.contains("byte order"))
         bigEndian = oline.after("=").trim().toInt();
   }

   ossimDpt gridOrigin(0,0);

   ifstream geocStream (geoc_file.c_str(), ios_base::binary|ios_base::in);
   if (geocStream.fail())
   {
      cout << "ossimEnviCgModel:Ctor Error encountered opening GEOC file <"<<geoc_file<<">"<<endl;
      return false;
   }

   ossimDpt spacing (1,1);
   theLatGrid.initialize(imageSize, gridOrigin, spacing);
   theLonGrid.initialize(imageSize, gridOrigin, spacing);
   theDlatDhGrid.initialize(imageSize, gridOrigin, spacing);
   theDlonDhGrid.initialize(imageSize, gridOrigin, spacing);

   double value;
   for (int y=0; (y<imageSize.y) && !geocStream.eof() ; ++y)
   {
      for (int x=0; (x<imageSize.x) && !geocStream.eof() ; ++x)
      {
         value = readValue(geocStream, (bool) bigEndian);
         theLonGrid.setNode(x, y, value);
      }
   }
   for (int y=0; (y<imageSize.y) && !geocStream.eof() ; ++y)
   {
      for (int x=0; (x<imageSize.x) && !geocStream.eof() ; ++x)
      {
         value = readValue(geocStream, (bool) bigEndian);
         theLatGrid.setNode(x, y, value);
      }
   }

   theDlatDhGrid.fill(0.0);
   theDlonDhGrid.fill(0.0);
   theLatGrid.setDomainType(ossimDblGrid::SAWTOOTH_90);
   theLonGrid.setDomainType(ossimDblGrid::WRAP_180);

   ossimIrect imageBounds(0, 0, imageSize.x-1, imageSize.y-1);
   initializeModelParams(imageBounds);

   return true;
}

bool ossimEnviCgModel::saveState(ossimKeywordlist& kwl, const char* prefix) const
{
   bool status = ossimCoarseGridModel::saveState(kwl, prefix);
   if (status)
      kwl.add(prefix, "type", "ossimCoarseGridModel");
   return status;
}

double ossimEnviCgModel::readValue(ifstream& s, bool bigEndian) const
{
   double value;
   s.read( reinterpret_cast<char*>(&value) , sizeof(double) );
   if (bigEndian)
   {
      char * pc = reinterpret_cast<char *>(&value);
      std::swap(pc[0], pc[7]);
      std::swap(pc[1], pc[6]);
      std::swap(pc[2], pc[5]);
      std::swap(pc[3], pc[4]);
      cout<<endl;
   }
   return value;
}
