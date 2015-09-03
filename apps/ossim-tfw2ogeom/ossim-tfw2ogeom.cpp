//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: tfw2ogeom.cpp 13025 2008-06-13 17:06:30Z sbortman $

#include <cstdlib>
#include <fstream>

#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimEpsgProjectionFactory.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimUnitTypeLut.h>
#include <ossim/support_data/ossimTiffWorld.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>


void usage()
{
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "ossim-tfw2ogeom "
      << "<template_proj> <tif world file> [<output file>]\n\n"
      << "Options:\n"
      << "-h                 Display usage.\n\n"
      << "-w <template_file> Write a template to template_file.\n\n" 
      << "<template_proj>   See template.kwl for descriptions\n\n"
      << "[<output file>]   optional argumaent and if not supplied\n"
      << "                  defaults to the tif world file with a\n"
      << "                  .geom extension" << std::endl;
}

void outputGeom(const ossimFilename& templateFile,
                const ossimFilename& tfwFile,
                const ossimFilename& output)
{
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "tfw2ogeom:"
      << "\ntemplate file:         " << templateFile
      << "\ntfw file:              " << tfwFile
      << "\noutput geometry file:  " << output
      << std::endl;
   
   ossimKeywordlist geomKwl(templateFile.c_str());

   ossimTiffWorld tfw;
   const char* pixel_type = geomKwl.find(ossimKeywordNames::PIXEL_TYPE_KW);
   const char* pcs_code   = geomKwl.find(ossimKeywordNames::PCS_CODE_KW);
   ossimUnitType unitType = (ossimUnitType)ossimUnitTypeLut::instance()->getEntryNumber(geomKwl);
   
   if(ossimString(pixel_type).upcase().contains("POINT"))
   {
      tfw = ossimTiffWorld(tfwFile.c_str(), OSSIM_PIXEL_IS_POINT, unitType);
   }
   else
   {
      tfw = ossimTiffWorld(tfwFile.c_str(), OSSIM_PIXEL_IS_AREA, unitType);
   }
   
   if(pcs_code)
   {
      ossimString epsg_spec ("EPSG:"); 
      epsg_spec += ossimString::toString(pcs_code);
      ossimProjection* proj = ossimEpsgProjectionFactory::instance()->createProjection(epsg_spec);
      if (proj)
         proj->saveState(geomKwl);
   }

   tfw.saveToOssimGeom(geomKwl);

   geomKwl.write(output.c_str());

   ossimNotify(ossimNotifyLevel_NOTICE)
      << "Wrote file:  " << output.c_str() << std::endl;
}

void writeTemplate(const ossimFilename& templateFile)
{
   std::ofstream out(templateFile.c_str());
   if (!out)
   {
      return;
   }
   
   out << "// tfw2ogeom keyword list template\n"
       << "// Note:  Use c++ comments \"//\" for comments.\n\n\n"
       << "// Units can be any of the below; although typically,\n"
       << "// meters or degrees.\n"
       << "//     us_survey_feet\n"
       << "//     feet\n"
       << "//     meters\n"
       << "//     degrees\n"
       << "//     seconds\n"
       << "//     minutes\n"
       << "//     miles\n"
       << "//     nautical_miles\n"
       << "units: meters\n\n"
       << "// Pixel type specifies where the easting northing values\n"
       << "// refer to.\n"
       << "// \"pixel_is_area\" means the tie point is relative to\n"
       << "// the upper left corner of the pixel.\n"
       << "// \"pixel_is_point\" means the tie point is relative to\n"
       << "// the center of the pixel.\n"
       << "pixel_type: pixel_is_area\n\n\n"
       << "// Give a pcs code you don't need to give the full projection\n"
       << "// information defined below.\n"
       << "// pcs_code: 32149\n\n\n"
       << "// Projection type.  Note that type is the ossim class\n"
       << "// for the projection. Some common names are listed below.\n"
       << "// Have only one type or a pcs code.\n"
       << "// type: ossimAlbersProjection\n"
       << "// type: ossimEquDistCylProjection\n"
       << "// type: ossimLambertConformalConicProjection\n"
       << "// type: ossimUtmProjection\n"
       << "// type: ossimTransMercatorProjection\n\n\n"
       << "// Projection pararmeters.  These are some common ones.\n"
       << "// origin_latitude:\n"
       << "// central_meridian:\n"
       << "// scale_factor:\n"
       << "// zone:\n"
       << "// std_parallel_1:\n"
       << "// std_parallel_2:\n"
       << "// Use application \"datums\" to get a list of datum codes.\n"
       << "// datum:\n"
       << "// false_easting:\n"
       << "// false_northing:\n\n\n"
       << std::endl;

   out.close();

   ossimNotify(ossimNotifyLevel_NOTICE)
      << "Wrote file:  " << templateFile.c_str() << std::endl;
}

int main(int argc, char* argv[])
{
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   argumentParser.getApplicationUsage()->
      setApplicationName(argumentParser.getApplicationName());

   argumentParser.getApplicationUsage()->
      setDescription(argumentParser.getApplicationName()+" Takes tiff world file and a projection template and makes an ossim geometry file.  ");
   
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" <template_proj> <tif world file> [<output file>]");

   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display usage.");
   
   argumentParser.getApplicationUsage()->addCommandLineOption("-w", "Writes a template file out to argument after the -w");

   if (argumentParser.read("-h") ||
       argumentParser.read("--help")||(argumentParser.argc() < 3))
   {
      usage();
      exit(0);
   }
   while(argumentParser.read("-w", stringParam))
   {
      ossimFilename geometryTemplateFile = tempString.c_str();
      writeTemplate(geometryTemplateFile);
      exit(0);
   }

   ossimFilename templateFile;
   ossimFilename tfw;
   ossimFilename output;
   
   if (argumentParser.argc() == 3)
   {
      templateFile = argumentParser.argv()[argumentParser.argc()-2];
      tfw          = argumentParser.argv()[argumentParser.argc()-1];
      output       = tfw;
      output.setExtension("geom");
   }
   else if (argumentParser.argc() == 4)
   {
      templateFile = argumentParser.argv()[argumentParser.argc()-3];
      tfw          = argumentParser.argv()[argumentParser.argc()-2];
      output       = argumentParser.argv()[argumentParser.argc()-1];
   }
   else
   {
      usage();
      exit(0);
   }
   
   outputGeom(templateFile, tfw, output);

   return 0;
}
