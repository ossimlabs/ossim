//*************************************************************************************************
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// DESCRIPTION:
// Projection Factory for EPSG coded projections. These are projections
// that are fully defined in database files and specified via a projection coordinate system (PCS)
// code.
//
//*************************************************************************************************
//  $Id: ossimWktProjectionFactory.cpp 23379 2015-06-17 19:17:24Z okramer $

#include <ossim/projection/ossimWktProjectionFactory.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimCylEquAreaProjection.h>
#include <ossim/projection/ossimTransMercatorProjection.h>
#include <ossim/projection/ossimMercatorProjection.h>
#include <ossim/projection/ossimLambertConformalConicProjection.h>
#include <ossim/projection/ossimUtmProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimEpsgProjectionDatabase.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimEpsgDatumFactory.h>
#include <ossim/support_data/ossimWkt.h>
#include <sstream>

ossimWktProjectionFactory* ossimWktProjectionFactory::m_instance = 0;

static const ossimString WKT_MAGIC("WKT_PCS");

/*************************************************************************************************
 * Sample WKT string (for reference, line feeds and spaces added for human readability)
 *
 * PROJCS["NAD_1983_HARN_Lambert_Conformal_Conic",
 *        GEOGCS["GCS_North_American_1983_HARN",
 *               DATUM["NAD83_High_Accuracy_Regional_Network",
 *                     SPHEROID["GRS_1980",6378137,298.257222101, AUTHORITY["EPSG","7019"]],
 *                     AUTHORITY["EPSG","6152"]],
 *               PRIMEM["Greenwich",0],
 *               UNIT["degree",0.0174532925199433]],
 *        PROJECTION["Lambert_Conformal_Conic_2SP"],
 *        PARAMETER["standard_parallel_1",43],
 *        PARAMETER["standard_parallel_2",45.5],
 *        PARAMETER["latitude_of_origin",41.75],
 *        PARAMETER["central_meridian",-120.5],
 *        PARAMETER["false_easting",1312335.958005249],
 *        PARAMETER["false_northing",0],
 *        UNIT["foot",0.3048, AUTHORITY["EPSG","9002"]]]
 *
 * Another sample (with EPSG at the top for easy instantiation of full projection):
 *
 * PROJCS["WGS 84 / UTM zone 41N",
 *        AUTHORITY["EPSG","32641"],
 *        GEOGCS["WGS 84",
 *               DATUM["WGS_1984",
 *                     SPHEROID["WGS 84",6378137,298.2572235630016, AUTHORITY["EPSG","7030"]],
 *                     AUTHORITY["EPSG","6326"]],
 *               PRIMEM["Greenwich",0],
 *               UNIT["degree",0.0174532925199433]],
 *        PROJECTION["Transverse_Mercator"],
 *        PARAMETER["latitude_of_origin",0],
 *        PARAMETER["central_meridian",63],
 *        PARAMETER["scale_factor",0.9996],
 *        PARAMETER["false_easting",500000],
 *        PARAMETER["false_northing",0],
 *        UNIT["metre",1, AUTHORITY["EPSG","9001"]]]
 *
 **************************************************************************************************/

//*************************************************************************************************
//! Implements singleton pattern
//*************************************************************************************************
ossimWktProjectionFactory* ossimWktProjectionFactory::instance()
{
   if (!m_instance)
      m_instance = new ossimWktProjectionFactory;
   return m_instance;
}

//*************************************************************************************************
//! Loads all SRS CSV files specified in the ossim prefs
//*************************************************************************************************
void ossimWktProjectionFactory::loadRecords() const
{
   // Fetch filename of WKT projection DB file specified in ossim_preferences:
   ossimFilename db_name = ossimPreferences::instance()->preferencesKWL().find("wkt_database_file");
   if (!db_name.isReadable())
      return;

   // Create only once outside the loop:
   ossimString format_id;
   ossimString line;

   // Open the DB file:
   std::ifstream db_stream(db_name.chars());
   bool good_file = false;
   if (db_stream.good())
   {
      // Format specification implied in file's magic number:
      std::getline(db_stream, format_id.string());
      format_id.trim();
      if (format_id == WKT_MAGIC)
         good_file = true;
   }
   if (!good_file)
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimWktProjectionDatabase::loadRecords() -- Encountered bad WKT database file <"
            << db_name << ">. Skipping this file." << endl;
      db_stream.close();
      return;
   }

   // The file is good. Skip over the column descriptor line:
   std::getline(db_stream, line.string());

   // Loop to read all data records:
   while (!db_stream.eof())
   {
      std::getline(db_stream, line.string());
      std::vector<ossimString> csvRecord = line.explode(","); // ONLY CSV FILES CONSIDERED HERE
      if (csvRecord.size())
      {
         std::pair<std::string, ossim_uint32> projRecord;
         projRecord.first = csvRecord[1].string();
         projRecord.second = csvRecord[0].toUInt32();
         m_wktProjRecords.insert(projRecord);
      }
   }

//   for ( std::map<ossimString, ossim_uint32>::const_iterator it = m_wktProjRecords.begin();
//         it != m_wktProjRecords.end(); it++)
//     cout << "[" << it->first << ", " << it->second << "]"<<endl;

   db_stream.close();
}

//*************************************************************************************************
//! From keywordlist (as generated typically by ossimWkt class)
//*************************************************************************************************
ossimProjection* ossimWktProjectionFactory::createProjection(const ossimKeywordlist &keywordList,
                                                             const char *prefix) const
{
   // The WKT for the horizontal projection may be part of a compound coordinate system, as
   // indicated by the "COMPD_CS" prefix. Need to remove that prefix first.
   ossimString compd_cs ("COMPD_CS.");
   ossimKeywordlist temp_kwl (keywordList);
   temp_kwl.stripPrefixFromAll(compd_cs);

   ossimProjection* proj = 0;
   ossimString pcs_name = temp_kwl.find(prefix, "PROJCS.name");
   if (pcs_name.empty())
      return 0;

   if (m_wktProjRecords.empty())
      loadRecords();

   // Search the WKT DB for a mapping of projection name to EPSG code. This should take care of
   // majority of cases, like UTM:
   ossimString epsg_code;
   std::map<std::string, ossim_uint32>::const_iterator it = m_wktProjRecords.find(pcs_name.string());
   if (it != m_wktProjRecords.end())
   {
      // Found an entry by this name, fetch the EPSG code:
      epsg_code = ossimString::toString(it->second);
   }
   else
   {
      // The name specified in the WKT could not be found in our WKT->EPSG map.
      // Check for EPSG code in WKT itself:
      ossimString auth_name = temp_kwl.find(prefix, "PROJCS.AUTHORITY.name");
      if (auth_name == "EPSG")
         epsg_code = temp_kwl.find(prefix, "PROJCS.AUTHORITY.param0");
   }

   // Use EPSG if determined:
   if (!epsg_code.empty())
   {
      proj = ossimEpsgProjectionDatabase::instance()->findProjection(epsg_code.toUInt32());
      if (proj)
         return proj;
   }

   // Not EPSG, so check conventional proj spec in WKT:
   ossimString proj_name = temp_kwl.find(prefix, "PROJCS.PROJECTION.name");
   if (proj_name.empty())
      return 0;
   proj_name.downcase();
   proj_name.gsub(" ", "_", true);

   // Note that prefix is ignored. This KWL is assumed to come from ossimWkt that doesn't prefix:
   if (proj_name.contains("transverse_mercator"))
      proj = doTransverseMercator(temp_kwl);
   else if (proj_name.contains("mercator"))
      proj = doMercator(temp_kwl);
   else if (proj_name.contains("lambert"))
      proj = doLambertConformalConic(temp_kwl);
   else if (proj_name.contains("equirectangular"))
      proj = doEquiDistCylindrical(temp_kwl);
   else if (proj_name.contains("cylindrical_equal_area"))
      proj = doEquiAreaCylindrical(temp_kwl);

   return proj;
}

//*************************************************************************************************
// This is the principal factory method. It accepts a WKT string, e.g.
//
//    "Anguilla_1957_British_West_Indies_Grid",
//
//  or complete WKT, e.g.
//
//    PROJCS["Anguilla_1957_British_West_Indies_Grid", GEOGCS[...
//
// IMPORTANT NOTE: Image tie-points cannot be conveyed by a WKT projection string. The projection
// created here will not be fully initialized for use in rendering imagery.
//*************************************************************************************************
ossimProjection* ossimWktProjectionFactory::createProjection(const ossimString &spec) const
{
   ossimProjection* proj = 0;
   ossimWkt wkt;
   if (wkt.parse(spec))
   {
      proj = createProjection(wkt.getKwl());
   }
   return proj;
}

//*************************************************************************************************
ossimObject* ossimWktProjectionFactory::createObject(const ossimString& typeName) const
{
   return createProjection(typeName);
}

//*************************************************************************************************
ossimObject* ossimWktProjectionFactory::createObject(const ossimKeywordlist& kwl,
                                                     const char* prefix) const
{
   return createProjection(kwl, prefix);
}

//*************************************************************************************************
//! STUB. Not implemented
//*************************************************************************************************
ossimProjection* ossimWktProjectionFactory::createProjection(const ossimFilename& /* filename */,
                                                             ossim_uint32 /* entryIdx */) const
{
   return 0;
}

//*************************************************************************************************
//! This returns the type name of all objects in all factories. This is the name used to 
//! construct the objects dynamically and this name must be unique.
//*************************************************************************************************
void ossimWktProjectionFactory::getTypeNameList(std::vector<ossimString>& typeList) const
{
   if (m_wktProjRecords.empty())
      loadRecords();

   std::map<std::string, ossim_uint32>::iterator db_iter = m_wktProjRecords.begin();
   while (db_iter != m_wktProjRecords.end())
   {
      typeList.push_back(ossimString(db_iter->first));
      db_iter++;
   }
   return;
}

ossimProjection* ossimWktProjectionFactory::doTransverseMercator(const ossimKeywordlist& kwl) const
{
   ossimTransMercatorProjection* proj = new ossimTransMercatorProjection();
   doMapCommon<ossimTransMercatorProjection*>(kwl, proj);

   // Determine first the units:
   ossimString name_read;
   ossimString val_read;

   // Read projection parameters from WKT KWL:
   int param_idx = 0;
   do
   {
      ossimString param("PROJCS.PARAMETER");
      param += ossimString::toString(param_idx++);
      name_read = kwl.find(param + ".name");
      val_read = kwl.find(param + ".param0");

      if (name_read.contains("scale_factor"))
      {
         proj->setScaleFactor(val_read.toDouble());
         break;
      }

   } while (!name_read.empty());

   return proj;
}

ossimProjection* ossimWktProjectionFactory::doMercator(const ossimKeywordlist& kwl) const
{
   ossimMercatorProjection* proj = new ossimMercatorProjection();
   doMapCommon<ossimMercatorProjection*>(kwl, proj);

   // Determine first the units:
   ossimString name_read;
   ossimString val_read;

   // Read projection parameters from WKT KWL:
   int param_idx = 0;
   do
   {
      ossimString param("PROJCS.PARAMETER");
      param += ossimString::toString(param_idx++);
      name_read = kwl.find(param + ".name");
      val_read = kwl.find(param + ".param0");

      if (name_read.contains("scale_factor"))
      {
         proj->setScaleFactor(val_read.toDouble());
         break;
      }

   } while (!name_read.empty());

   return proj;
}

ossimProjection* ossimWktProjectionFactory::doLambertConformalConic(const ossimKeywordlist& kwl) const
{
   ossimLambertConformalConicProjection* proj = new ossimLambertConformalConicProjection();
   doMapCommon<ossimLambertConformalConicProjection*>(kwl, proj);

   // Determine first the units:
   ossimString name_read;
   ossimString val_read;

   // Read projection parameters from WKT KWL:
   int param_idx = 0;
   do
   {
      ossimString param("PROJCS.PARAMETER");
      param += ossimString::toString(param_idx++);
      name_read = kwl.find(param + ".name");
      val_read = kwl.find(param + ".param0");

      if (name_read.contains("standard_parallel_1"))
         proj->setStandardParallel1(val_read.toDouble());

      else if (name_read.contains("standard_parallel_2"))
         proj->setStandardParallel2(val_read.toDouble());

   } while (!name_read.empty());

   return proj;
}

ossimProjection* ossimWktProjectionFactory::doEquiDistCylindrical(const ossimKeywordlist& kwl) const
{
   ossimEquDistCylProjection* proj = new ossimEquDistCylProjection();
   doMapCommon<ossimEquDistCylProjection*>(kwl, proj);

   return proj;
}

ossimProjection* ossimWktProjectionFactory::doEquiAreaCylindrical(const ossimKeywordlist& kwl) const
{
   ossimCylEquAreaProjection* proj = new ossimCylEquAreaProjection();
   doMapCommon<ossimCylEquAreaProjection*>(kwl, proj);

   return proj;
}

template<class T>
void ossimWktProjectionFactory::doMapCommon(const ossimKeywordlist& kwl, T proj) const
{
   ossimString name_read;
   ossimString val_read;

   const ossimDatum* datum = 0;
   name_read = kwl.find("PROJCS.GEOGCS.AUTHORITY.name");
   val_read = kwl.find("PROJCS.GEOGCS.AUTHORITY.param0");
   if ((name_read == "EPSG") && (!val_read.empty()))
   {
      datum = ossimEpsgDatumFactory::instance()->create(val_read.toUInt32());
      if (datum)
         proj->setDatum(datum);
      // TODO: HARN Datum not being handled
   }

   // Determine first the units:
   name_read = kwl.find("PROJCS.UNIT.name");
   double conv_factor = 1.0;
   if (name_read.contains("f")) // feet|foot
   {
      conv_factor = MTRS_PER_FT;
      proj->setProjectionUnits(OSSIM_FEET);
   }

   // Read projection parameters from WKT KWL:
   int param_idx = 0;
   ossimGpt origin(0, 0, 0, datum);
   do
   {
      ossimString param("PROJCS.PARAMETER");
      param += ossimString::toString(param_idx++);
      name_read = kwl.find(param + ".name");
      val_read = kwl.find(param + ".param0");

      if (name_read.contains("central_meridian"))
         origin.lon = val_read.toDouble();

      else if (name_read.contains("latitude_of_origin"))
         origin.lat = val_read.toDouble();

      else if (name_read.contains("false_easting"))
         proj->setFalseEasting(val_read.toDouble() * conv_factor);

      else if (name_read.contains("false_northing"))
         proj->setFalseNorthing(val_read.toDouble() * conv_factor);

   } while (!name_read.empty());

   proj->setOrigin(origin);
}

