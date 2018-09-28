//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
// 
// Class definition for ossimQuickbirdMetaData.
// 
// This class parses a Space Imaging Quickbird meta data file.
//
//********************************************************************
// $Id: ossimQuickbirdMetaData.cpp 14431 2009-04-30 21:58:33Z dburken $

#include <ossim/support_data/ossimQuickbirdMetaData.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <cstdio>
#include <iostream>
#include <stack>
#include <memory>

RTTI_DEF1(ossimQuickbirdMetaData, "ossimQuickbirdMetaData", ossimObject);
 
// Define Trace flags for use within this file:
static ossimTrace traceExec  ("ossimQuickbirdMetaData:exec");
static ossimTrace traceDebug ("ossimQuickbirdMetaData:debug");

class QbMetadataParser
{
 public:
   bool openConnection(const ossimString &connectionString)
   {
      m_inputStream = ossim::StreamFactoryRegistry::instance()->createIstream(connectionString);
      return m_inputStream != nullptr;
   }
   bool nextLine(ossimString &line)
   {
      std::ostringstream out;
      line = "";
      if (m_inputStream->good())
      {
         char c = static_cast<char>(m_inputStream->get());
         while (!m_inputStream->eof() && m_inputStream->good() && c != '\n')
         {
            out.write(&c, 1);
            c = static_cast<char>(m_inputStream->get());
         }
         line = out.str();
      }

      return !line.empty();
   }
   bool parseConnection(const ossimString &connectionString)
   {
      bool result = false;

      if (openConnection(connectionString))
      {
         return parse();
      }

      return result;
   }
   bool parse();
   void splitKeyValue(const ossimString &input, ossimString &key, ossimString &value) const
   {
      key = input.before("=");
      value = input.after("=");
      key = key.trim();
      value = value.trim();
   }
   const std::shared_ptr<ossimKeywordlist> &getKwl() const
   {
      return m_kwl;
   }

   std::shared_ptr<ossim::istream> m_inputStream;
   std::shared_ptr<ossimKeywordlist> m_kwl;
   ossimString m_currentPrefix;
};

bool QbMetadataParser::parse() //std::istream& in, ossimKeywordlist& kwl, const ossimString& prefix)
{
   bool result = true;
   std::stack<ossimString> prefixStack;
   std::stack<ossim_int32> currentIdxStack;
   ossimString line;
   ossimString kwlString;
   ossim_int32 bandIdx = -1;
   ossim_int32 imageIdx = -1;
   ossimString fullKeyValue;
   ossimString key;
   ossimString value;
   prefixStack.push("");
   ossimString bandNames;
   m_kwl = std::make_shared<ossimKeywordlist>();
   while (result && !prefixStack.empty() && nextLine(line))
   {
      ossimString tempLine = line;
      tempLine = tempLine.trim();
      if (tempLine.startsWith("BEGIN_GROUP"))
      {
         ossimString currentPrefix = prefixStack.top();
         splitKeyValue(tempLine, key, value);
         ossimString test = value;
         test = test.downcase();
         if (test.startsWith("band"))
         {
            ossimString bandNameValue = value.after("_").downcase();
            if (bandNames.empty())
            {
               bandNames = bandNameValue;
            }
            else
            {
               bandNames += " " + bandNameValue;
            }
         }
         prefixStack.push(currentPrefix + value.downcase().trim() + ".");
      }
      else if (tempLine.startsWith("END_GROUP"))
      {
         prefixStack.pop();
         if (prefixStack.empty())
         {
            result = false;
         }
      }
      else if (tempLine.endsWith(";"))
      {
         if (!tempLine.startsWith("END"))
         {
            fullKeyValue += line;
            fullKeyValue = fullKeyValue.trim();
            splitKeyValue(fullKeyValue, key, value);
            value = value.trim().trim(";").trim("\"");
            key = prefixStack.top() + key.trim();
            m_kwl->add(key, value);
         }
         fullKeyValue = "";
      }
      else
      {
         fullKeyValue += line;
      }
   }
   if (!bandNames.empty())
   {
      m_kwl->add("band_name_list", bandNames);
   }
   return result;
}

ossimQuickbirdMetaData::ossimQuickbirdMetaData()
    : theGenerationDate("Unknown"),
      theBandId("Unknown"),
      theBitsPerPixel(0),
      theSatID("Unknown"),
      theTLCDate("Unknown"),
      theSunAzimuth(0.0),
      theSunElevation(0.0),
      theSatAzimuth(0.0),
      theSatElevation(0.0),
      theTDILevel(0),
      theAbsCalFactors(),
      theBandNameList("Unknown"),
      theImageSize(),
      thePNiirs(-1),
      theCloudCoverage(0.0)
{
   theImageSize.makeNan();
   theAbsCalFactors.clear();
}

ossimQuickbirdMetaData::~ossimQuickbirdMetaData()
{
}

bool ossimQuickbirdMetaData::open(const ossimFilename& imageFile,
                                  ossim_int32 qbParseTypes)
{
   static const char MODULE[] = "ossimQuickbirdMetaData::open";

   clearFields();

   //retrieve information from the metadata file
   //if the Quickbird tif is 02APR01105228-M1BS-000000128955_01_P001.TIF
   //the metadata file will be 02APR01105228-M1BS-000000128955_01_P001.IMD

   ossimFilename file = imageFile;

   if(qbParseTypes & QB_PARSE_TYPE_IMD)
   {
      file.setExtension(ossimString("IMD"));
      if (parseMetaData(file) == false)
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
                << MODULE << " errors parsing metadata" << std::endl;
         }
         return false;
      }
   }
   if (qbParseTypes & QB_PARSE_TYPE_ATT)
   {
      file.setExtension(ossimString("ATT"));
      if(!parse(m_attKwl, file))
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
                << MODULE << " errors parsing att" << std::endl;
         }
         return false;
      }
   }
   if (qbParseTypes & QB_PARSE_TYPE_EPH)
   {
      file.setExtension(ossimString("EPH"));
      if (!parse(m_ephKwl, file))
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
                << MODULE << " errors parsing eph" << std::endl;
         }
         return false;
      }
   }
   if (qbParseTypes & QB_PARSE_TYPE_GEO)
   {
      file.setExtension(ossimString("GEO"));
      if (!parse(m_geoKwl, file))
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
                << MODULE << " errors parsing geo" << std::endl;
         }
         return false;
      }
   }
   if (qbParseTypes & QB_PARSE_TYPE_RPB)
   {
      file.setExtension(ossimString("RPB"));
      if (!parse(m_rpbKwl, file))
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
                << MODULE << " errors parsing rpb" << std::endl;
         }
         return false;
      }
   }
   if (qbParseTypes & QB_PARSE_TYPE_TIL)
   {
      file.setExtension(ossimString("TIL"));
      if (!parse(m_tilKwl, file))
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
                << MODULE << " errors parsing til" << std::endl;
         }
         return false;
      }
   }

   return true;
}

void ossimQuickbirdMetaData::clearFields()
{
   theGenerationDate = "Unknown";
   theBitsPerPixel = 0;
   theBandId = "Unknown";
   theSatID = "Unknown";
   theTLCDate = "Unknown";
   theSunAzimuth = 0.0;
   theSunElevation = 0.0;
   theSatAzimuth = 0.0;
   theSatElevation = 0.0;
   theTDILevel = 0;
   theAbsCalFactors.clear();
   theBandNameList = "Unknown";
   theImageSize.makeNan();
   m_attKwl = nullptr;
   m_imdKwl = nullptr;
   m_ephKwl = nullptr;
   m_rpbKwl = nullptr;
   m_geoKwl = nullptr;
   m_tilKwl = nullptr;
   m_steKwl = nullptr;
}

std::ostream& ossimQuickbirdMetaData::print(std::ostream& out) const
{

   out << "\n----------------- Info on Quickbird Image -------------------"
       << "\n  "
       << "\n  Generation date:    " << theGenerationDate
       << "\n  Band Id:            " << theBandId
       << "\n  Bits per pixel:     " << theBitsPerPixel
       << "\n  Sat Id:             " << theSatID
       << "\n  TLC date:           " << theTLCDate
       << "\n  Sun Azimuth:        " << theSunAzimuth
       << "\n  Sun Elevation:      " << theSunElevation
       << "\n  Sat Azimuth:        " << theSatAzimuth
       << "\n  Sat Elevation:      " << theSatElevation
       << "\n  Band name list:     " << theBandNameList
       << "\n  TDI Level:          " << theTDILevel
       << "\n  abs Calibration Factors:   " 
       << std::endl;
   for(unsigned int i=0; i<theAbsCalFactors.size(); i++)
   {
      out<<theAbsCalFactors[i] << "   ";
   }
   out << "\n  Image Size:         " << theImageSize
       << "\n"
       << "\n---------------------------------------------------------"
       << "\n  " << std::endl;
   return out;
}

bool ossimQuickbirdMetaData::saveState(ossimKeywordlist& kwl,
				       const char* prefix)const
{
   if(m_imdKwl)
   {
      kwl.add(prefix,
              ossimKeywordNames::TYPE_KW,
              "ossimQuickbirdMetaData",
              true);

      kwl.add(prefix,
              "generation_date",
              theGenerationDate,
              true);

      // will use generation date as the acquisition
      kwl.add(prefix,
              "acquisition_date",
              theGenerationDate,
              true);

      kwl.add(prefix,
              "bits_per_pixel",
              theBitsPerPixel,
              true);

      kwl.add(prefix,
              "band_id",
              theBandId,
              true);

      kwl.add(prefix,
              "sat_id",
              theSatID,
              true);

      kwl.add(prefix,
              "mission_id",
              theSatID,
              true);

      kwl.add(prefix,
              "tlc_date",
              theTLCDate,
              true);

      kwl.add(prefix,
              ossimKeywordNames::AZIMUTH_ANGLE_KW,
              theSunAzimuth,
              true);

      kwl.add(prefix,
              ossimKeywordNames::ELEVATION_ANGLE_KW,
              theSunElevation,
              true);

      kwl.add(prefix,
              "sat_azimuth_angle",
              theSatAzimuth,
              true);

      kwl.add(prefix,
              "sat_elevation_angle",
              theSatElevation,
              true);

      kwl.add(prefix,
              "TDI_level",
              theTDILevel,
              true);
      kwl.add(prefix,
              "band_name_list",
              theBandNameList,
              true);

      if (thePNiirs > -1)
      {
         kwl.add(prefix,
                 "niirs",
                 thePNiirs,
                 true);
      }

      kwl.add(prefix,
              "cloud_cover",
              theCloudCoverage,
              true);

      if (theBandId == "Multi")
      {
         std::vector<ossimString> bandNameList = theBandNameList.split(" ");
         for (unsigned int i = 0; i < bandNameList.size(); ++i)
         {
            kwl.add(prefix,
                    bandNameList[i] + "_band_absCalFactor",
                    theAbsCalFactors[i],
                    true);
         }
      }
      else if (!theAbsCalFactors.empty())
      {
         kwl.add(prefix,
                 "absCalFactor",
                 theAbsCalFactors[0],
                 true);
      }
      kwl.add(ossimString(prefix) + "imd.", *m_imdKwl);
   }
   if (m_attKwl)
   {
      kwl.add(ossimString(prefix) + "att.", *m_attKwl);
   }
   if (m_ephKwl)
   {
      kwl.add(ossimString(prefix) + "eph.", *m_ephKwl);
   }
   if (m_geoKwl)
   {
      kwl.add(ossimString(prefix) + "geo.", *m_geoKwl);
   }
   if (m_rpbKwl)
   {
      kwl.add(ossimString(prefix) + "rpb.", *m_rpbKwl);
   }
   if (m_steKwl)
   {
      kwl.add(ossimString(prefix) + "ste.", *m_rpbKwl);
   }
   if (m_tilKwl)
   {
      kwl.add(ossimString(prefix) + "til.", *m_tilKwl);
   }

   return true;
}

bool ossimQuickbirdMetaData::loadState(const ossimKeywordlist& kwl,
				       const char* prefix)
{
   clearFields();

   const char* lookup = 0;
   ossimString s;
  
   lookup = kwl.find(prefix, ossimKeywordNames::TYPE_KW);
   if (lookup)
   {
      s = lookup;
      if(s != "ossimQuickbirdMetaData")
      {
         return false;
      }
   }

   lookup = kwl.find(prefix, "generation_date");
   if (lookup)
   {
      theGenerationDate = lookup;
   }

   lookup = kwl.find(prefix, "band_id");
   if (lookup)
   {
      theBandId = lookup;
   }
  
   lookup = kwl.find(prefix, "bits_per_pixel");
   if (lookup)
   {
      s = lookup;
      theBitsPerPixel = s.toInt();;
   }
    
   lookup = kwl.find(prefix, "sat_id");
   if (lookup)
   {
      theSatID = lookup;
   }

   lookup = kwl.find(prefix, "tlc_date");
   if (lookup)
   {
      theTLCDate= lookup;
   }

   lookup = kwl.find(prefix, "TDI_level");
   if (lookup)
   {
      s = lookup;
      theTDILevel = s.toInt();
   }

   lookup = kwl.find(prefix, ossimKeywordNames::AZIMUTH_ANGLE_KW);
   if (lookup)
   {
      s = lookup;
      theSunAzimuth = s.toFloat64();
   }

   lookup = kwl.find(prefix, ossimKeywordNames::ELEVATION_ANGLE_KW);
   if (lookup)
   {
      s = lookup;
      theSunElevation = s.toFloat64();
   }

   lookup = kwl.find(prefix, "sat_azimuth_angle");
   if (lookup)
   {
      s = lookup;
      theSatAzimuth = s.toFloat64();
   }

   lookup = kwl.find(prefix, "sat_elevation_angle");
   if (lookup)
   {
      s = lookup;
      theSatElevation = s.toFloat64();
   }

   lookup = kwl.find(prefix, "band_name_list");
   if (lookup)
   {
      theBandNameList= lookup;
   }
   lookup = kwl.find(prefix, "niirs");
   if (lookup)
   {
      thePNiirs = ossimString(lookup).toFloat64();
   }

   lookup = kwl.find(prefix, "cloud_cover");
   if(lookup)
   {
      theCloudCoverage = ossimString(lookup).toFloat64();
   }

   if(theBandId=="Multi")
   {
      std::vector<ossimString> bandNameList = theBandNameList.split(" ");
      theAbsCalFactors = std::vector<double>(bandNameList.size(), 1.);
      for(unsigned int i = 0 ; i < bandNameList.size() ; ++i)
      {
         lookup = kwl.find(prefix, bandNameList[i] + "_band_absCalFactor");
         if (lookup)
         {
            s = lookup;
            theAbsCalFactors[i] = s.toDouble();
         }
      }
   }
   else if (theBandId=="P")
   {
      theAbsCalFactors = std::vector<double>(1, 1.);
      lookup = kwl.find(prefix, "absCalFactor");
      if (lookup)
      {
         s = lookup;
         theAbsCalFactors[0] = s.toDouble();
      }	
   }
   
   if (kwl.getNumberOfKeysThatMatch("^" + ossimString(prefix) + "imd."))
   {
      m_imdKwl = std::make_shared<ossimKeywordlist>();
      m_imdKwl->add(kwl , ossimString(prefix) + "imd.");
   } 

   if (kwl.getNumberOfKeysThatMatch("^" + ossimString(prefix) + "att."))
   {
      m_attKwl = std::make_shared<ossimKeywordlist>();
      m_attKwl->add(kwl, ossimString(prefix) + "att.");
   }

   if (kwl.getNumberOfKeysThatMatch("^" + ossimString(prefix) + "geo."))
   {
      m_geoKwl = std::make_shared<ossimKeywordlist>();
      m_geoKwl->add(kwl, ossimString(prefix) + "geo.");
   }
   if (kwl.getNumberOfKeysThatMatch("^" + ossimString(prefix) + "rpb."))
   {
      m_rpbKwl = std::make_shared<ossimKeywordlist>();
      m_rpbKwl->add(kwl, ossimString(prefix) + "rpb.");
   }
   if (kwl.getNumberOfKeysThatMatch("^" + ossimString(prefix) + "ste."))
   {
      m_steKwl = std::make_shared<ossimKeywordlist>();
      m_steKwl->add(kwl, ossimString(prefix) + "ste.");
   }

   return true;
}

//*****************************************************************************
// PROTECTED METHOD: ossimQuickbirdMetaData::parseMetaData()
//
//  Parses the Quickbird IMD file.
//
//*****************************************************************************
bool ossimQuickbirdMetaData::parseMetaData(const ossimFilename& data_file)
{
   bool result = false;
   if (traceExec()) ossimNotify(ossimNotifyLevel_DEBUG) << "DEBUG ossimQuickbirdMetaData::parseMetaData(data_file): entering..." << std::endl;
   
   if( !data_file.exists() )
   {
      if (traceExec()) ossimNotify(ossimNotifyLevel_WARN) << "ossimQuickbirdMetaData::parseMetaData(data_file) WARN:" << "\nmetadate data file <" << data_file << ">. " << "doesn't exist..." << std::endl;
      return result;
   }
   m_imdKwl = nullptr;

   //     std::shared_ptr<QbMetadataParser>
   //         qbMetadataParser = std::make_shared<QbMetadataParser>();
   // if (qbMetadataParser->parseConnection(data_file))
   if(parse(m_imdKwl, data_file))
   {
      // m_imdKwl = std::make_shared<ossimKeywordlist>(*qbMetadataParser->getKwl());
      ossimString value = m_imdKwl->find("generationTime");
      if(!value.empty())
      {
         theGenerationDate = value;
      }
      theBandId = m_imdKwl->find("bandId");

      value = m_imdKwl->find("numRows");
      if(!value.empty())
      {
         theImageSize.y = value.toInt64();
      }
      value = m_imdKwl->find("numColumns");
      if (!value.empty())
      {
         theImageSize.x = value.toInt64();
      }
      theBandId = m_imdKwl->find("bandId");

      value = m_imdKwl->find("nbitsPerPixel");
      if(!value.empty())
      {
         theBitsPerPixel = value.toInt32();
      }
      theBandNameList = m_imdKwl->find("band_name_list");
      std::vector<ossimString> bandList;
      theBandNameList.split(bandList, " ");
      if(bandList.size()>0)
      {  
         ossim_uint32 idx = 0;
         theAbsCalFactors = std::vector<double>(bandList.size(), 1.0);
         for(auto band:bandList)
         {
            value = m_imdKwl->find("band_"+band+".absCalFactor");
            if(!value.empty())
            {
               theAbsCalFactors[idx] = value.toFloat64();
            }
            ++idx;
         }
      }
      theSatID = m_imdKwl->find("image_1.satId");
      theTLCDate = m_imdKwl->find("image_1.TLCTime");
      if(theTLCDate.empty())
      {
         theTLCDate = m_imdKwl->find("image_1.firstLineTime");
      }
      value = m_imdKwl->find("sunAz");
      if (!value.empty())
      {
         theSunAzimuth = value.toFloat64();
      }
      else
      {
         value = m_imdKwl->find("image_1.meanSunAz");
         if (!value.empty())
         {
            theSunAzimuth = value.toFloat64();
         }
      }
      value = m_imdKwl->find("sunEl");
      if (!value.empty())
      {
         theSunElevation = value.toFloat64();
      }
      else
      {
         value = m_imdKwl->find("image_1.meanSunEl");
         if (!value.empty())
         {
            theSunElevation = value.toFloat64();
         }
      }
      value = m_imdKwl->find("image_1.satAz");
      if (!value.empty())
      {
         theSatAzimuth = value.toFloat64();
      }
      else
      {
         value = m_imdKwl->find("image_1.meanSatAz");
         if (!value.empty())
         {
            theSatAzimuth = value.toFloat64();
         }
      }
      value = m_imdKwl->find("image_1.cloudCover");
      if (!value.empty())
      {
         theCloudCoverage = value.toFloat64();
      }
      value = m_imdKwl->find("image_1.PNIIRS");
      if (!value.empty())
      {
         thePNiirs = value.toFloat64();
      }

      result = true;
   }

   return result;
}

bool ossimQuickbirdMetaData::parse(std::shared_ptr<ossimKeywordlist> &kwl, 
                                   const ossimFilename &file)
{
   bool result = false;
   if (!file.exists())
   {
      if (traceExec())
         ossimNotify(ossimNotifyLevel_WARN) << "ossimQuickbirdMetaData::parse(file) WARN:"
                                            << "\ndata file <" << file << ">. "
                                            << "doesn't exist..." << std::endl;
      return result;
   }
   std::shared_ptr<QbMetadataParser> qbMetadataParser = std::make_shared<QbMetadataParser>();
   if (qbMetadataParser->parseConnection(file))
   {
      kwl = std::make_shared<ossimKeywordlist>(*qbMetadataParser->getKwl());
      result = kwl->getSize() > 0;
   }
   return result;
}

ossimString ossimQuickbirdMetaData::getSatID() const
{
   return theSatID;
}

bool ossimQuickbirdMetaData::getMapProjectionKwl( const ossimFilename& imd_file,
                                                  ossimKeywordlist& kwl )
{
   static const char MODULE[] = "ossimQuickbirdMetaData::getMapProjectionKwl";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }
   
   bool result = false;
   
   if( imd_file.exists() )
   {
      FILE* fptr = fopen (imd_file.c_str(), "r");
      if (fptr)
      {

         char* strptr(NULL);
         
         //---
         // Read the file into a buffer:
         //---
         ossim_int32 fileSize = static_cast<ossim_int32>(imd_file.fileSize());
         char* filebuf = new char[fileSize];
         fread(filebuf, 1, fileSize, fptr);
         strptr = filebuf;
         fclose(fptr);
         ossimString imd_key;
         ossimString tempStr;
         std::string key;
         std::string value;


         // Loop until we find all our keys or bust out with error.
         while ( 1 )
         {
            // Verify map projected.
            imd_key = "BEGIN_GROUP = MAP_PROJECTED_PRODUCT";
            if ( strstr( filebuf, imd_key.c_str() ) == NULL )
            {
               break; // Not a map projected product.
            }

            // Get datum:
            if( getEndOfLine( strptr, ossimString("\n\tdatumName = "), "%13c %s", tempStr) )
            {
               if ( tempStr.contains("WE") )
               {
                  key = "dataum";
                  value   = "WGE";
                  kwl.addPair(key, value);
               }
               else
               {
                  if(traceDebug())
                  {
                     ossimNotify(ossimNotifyLevel_WARN)
                        << "Unhandled datum: " << tempStr << "\n";
                  }
               }
            }

            // Get projection:
            if( getEndOfLine( strptr, ossimString("\n\tmapProjName = "), "%15c %s", tempStr) )
            {
               if ( tempStr.contains("UTM") )
               {
                  key = "type";
                  value   = "ossimUtmProjection";
                  kwl.addPair(key, value);
               }
               else
               {
                  if(traceDebug())
                  {
                     ossimNotify(ossimNotifyLevel_WARN)
                        << "Unhandled projection name: " << tempStr << "\n";
                  }
               }
            }

             // Get projection:
            if( getEndOfLine( strptr, ossimString("\n\tmapProjName = "), "%15c %s", tempStr) )
            {
               if ( tempStr.contains("UTM") )
               {
                  key = "type";
                  value   = "ossimUtmProjection";
                  kwl.addPair(key, value);

                  // Get UTM zone:
                  if( getEndOfLine( strptr, ossimString("\n\tmapZone = "), "%11c %s", tempStr) )
                  {
                     key = "zone";
                     value = tempStr.trim(";").string();
                     kwl.addPair(key, value);
                  }
                  else
                  {
                     break;
                  }
                  
                  // Get UTM hemisphere:
                  if( getEndOfLine( strptr, ossimString("\n\tmapHemi = "), "%11c %s", tempStr) )
                  {
                     key = "hemisphere";
                     tempStr = tempStr.trim(";");
                     tempStr = tempStr.trim("\"");
                     value = tempStr.string();
                     kwl.addPair(key, value);
                  }
                  else
                  {
                     break;
                  }
                  
               } // End UTM:
            }

            // Get projection units:
            std::string units;
            if( getEndOfLine( strptr, ossimString("\n\tproductUnits = "), "%16c %s", tempStr) )
            {
               if ( tempStr == "\"M\";" )
               {
                  key = "units";
                  units = "meters";
                  kwl.addPair(key, units);
               }
               else
               {
                  if(traceDebug())
                  {
                     ossimNotify(ossimNotifyLevel_WARN)
                        << "Unhandled units: " << tempStr << "\n";
                  }
               }
            }

            // Get projection tie point:
            ossimDpt dpt;
            dpt.makeNan();
            if( getEndOfLine( strptr, ossimString("\n\toriginX = "), "%11c %s", tempStr) )
            {
               tempStr = tempStr.trim(";");
               dpt.x = tempStr.toFloat64();
            }
            else
            {
               break;
            }
            if( getEndOfLine( strptr, ossimString("\n\toriginY = "), "%11c %s", tempStr) )
            {
               tempStr = tempStr.trim(";");
               dpt.y = tempStr.toFloat64();
            }
            else
            {
               break;
            }
            if ( dpt.hasNans() == false )
            {  
               key = "tie_point_units";
               kwl.addPair(key, units);

               key = "tie_point_xy";
               value = dpt.toString().string();
               kwl.addPair( key, value );
            }
            else
            {
               if(traceDebug())
               {
                  ossimNotify(ossimNotifyLevel_WARN)
                     << "tie point has nans!";
               }
               break;
            }

            // Get projection scale:
            dpt.makeNan();
            if( getEndOfLine( strptr, ossimString("\n\tcolSpacing = "), "%14c %s", tempStr) )
            {
               tempStr = tempStr.trim(";");
               dpt.x = tempStr.toFloat64();
            }
            else
            {
               break;
            }
            if( getEndOfLine( strptr, ossimString("\n\trowSpacing = "), "%14c %s", tempStr) )
            {
               tempStr = tempStr.trim(";");
               dpt.y = tempStr.toFloat64();
            }
            else
            {
               break;
            }

            if ( dpt.hasNans() == false )
            {  
               key = "pixel_scale_units";
               kwl.addPair(key, units);

               key = "pixel_scale_xy";
               value = dpt.toString().string();
               kwl.addPair( key, value );
            }
            else
            {
               if(traceDebug())
               {
                  ossimNotify(ossimNotifyLevel_WARN)
                     << "scale has nans!";
               }
               break;
            }
            
            //---
            // End of key look up. If we get here set the status to true and
            // bust out of loop.
            //---
            result = true;
            break;
         }

         if ( result == false )
         {
            if(traceDebug())
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << "ERROR: Missing or unhandled key in metadat: " << imd_key << "\n";
            }
         }

         delete [] filebuf;
         filebuf = 0;
      }
      else
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "ossimQuickbirdRpcModel::parseMetaData(imd_file) DEBUG:"
               << "\nCould not open Meta data file:  " << imd_file
	    << "\nreturning with error...\n";
         }
      }
   }

   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status = " << (result?"true":"false") << "\n";
   }

   return result;
}
const std::shared_ptr<ossimKeywordlist> ossimQuickbirdMetaData::getImdKwl() const
{
   return m_imdKwl;
}

const std::shared_ptr<ossimKeywordlist> ossimQuickbirdMetaData::getAttKwl() const
{
   return m_attKwl;
}

const std::shared_ptr<ossimKeywordlist> ossimQuickbirdMetaData::getGeoKwl() const
{
   return m_geoKwl;
}

const std::shared_ptr<ossimKeywordlist> ossimQuickbirdMetaData::getEphKwl() const
{
   return m_ephKwl;
}

const std::shared_ptr<ossimKeywordlist> ossimQuickbirdMetaData::getRpbKwl() const
{
   return m_rpbKwl;
}
//*****************************************************************************
// PROTECTED METHOD: ossimQuickbirdMetaData::getEndOfLine
//
// //  Parse a char * to find another char *. Change the pointer only if the second char * is found.
//
//*****************************************************************************
bool ossimQuickbirdMetaData::getEndOfLine( char * fileBuf,
                                           ossimString lineBeginning,
                                           const char * format,
                                           ossimString & name)
{
   //char * res = strstr(fileBuf, lineBeginning.c_str());
   //if (!res)
   //{
   //  return false;
   // }
   //// if the lineBeginning is found, update the start pointer adress
   //fileBuf = res;
   
   //char dummy[80], nameChar[80];
   //sscanf(res, format, dummy, nameChar);    
   //name = ossimString(nameChar);
    
   char * res = strstr(fileBuf, lineBeginning.c_str());
   if(!res)
   {
      return false;
   }
   fileBuf = strstr(fileBuf, lineBeginning.c_str());
   char dummy[80], nameChar[80];
   sscanf(fileBuf, format, dummy, nameChar);  
   name = ossimString(nameChar);
    
   return true;
}

const ossimIpt& ossimQuickbirdMetaData::getImageSize() const
{
   return theImageSize;
}
