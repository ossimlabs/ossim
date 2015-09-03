//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// ossimWkt class definition.  A well known text(WKT) utility class.
// 
//----------------------------------------------------------------------------
// $Id$

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
 **************************************************************************************************/
#include <ossim/support_data/ossimWkt.h>
#include <cstdio>
#include <iostream>
#include <sstream>

using namespace std; // tmp drb...

ossimWkt::ossimWkt()
   : m_kwl()
{
}

ossimWkt::~ossimWkt()
{
}

bool ossimWkt::parse( const std::string& wkt )
{
   bool result = false;
   if ( wkt.size() )
   {
      std::istringstream is( wkt );
      result = parseWktGroup( is, m_kwl );
   }
   return result;
}

const ossimKeywordlist& ossimWkt::getKwl() const
{
   return m_kwl;
}

bool ossimWkt::parseWktGroup( std::istringstream& is, ossimKeywordlist& kwl )
{
   bool result = false;

   if ( is.good() )
   {
      char c;

      // Get the wkt group name up to '[', e.g. "PROJCS[".
      std::string prefix = "";      
      std::string object;
      // std::string v;
      while ( is.good() )
      {
         is.get(c);
         if ( is.good() )
         {
            // Look for parens or square brackets.
            if ( (c != '[') && (c != '(') )
            {
               object.push_back(c);
            }
            else
            {
               result = parseObject( is, prefix, object, kwl );
            }
         }
         else
         {
            break;
         }
      }
   }
   
   return result;
}

bool ossimWkt::parseObject( std::istringstream& is,
                            const std::string& prefix,
                            const std::string& object,
                            ossimKeywordlist& kwl )
{
   bool result = false;

   result = parseName( is, prefix, object, kwl );

   if ( result && is.good() )
   {
      char c;
      ossim_uint32 myObjectIndex = 0;
      ossim_uint32 paramIndex = 0;
      while ( is.good() )
      {
         is.get(c);
         if ( is.good() )
         {  
            if ( c == ',' )
            {
               parseParam( is, prefix, object, myObjectIndex, paramIndex, kwl );
            }
            else if ( (c == ']') || (c == ')') )
            {
               break; // End of object.
            }
         }
      }
      
   }
   
   return result;
}

bool ossimWkt::parseName( std::istringstream& is,
                          const std::string& prefix,
                          const std::string& object,
                          ossimKeywordlist& kwl )
{
   bool result = false;
   char c;
   std::string name;

   // Find the first quote:
   while ( is.good() )
   {
      is.get(c);
      if ( is.good() )
      {  
         if ( c == '"' )
         {
            break;
         }
      }
   }

   // Get the name:
   while ( is.good() )
   {
      is.get(c);
      if ( is.good() )
      {  
         if ( c != '"' )
         {
            name.push_back(c);
         }
         else
         {
            break; // End quote:
         }
      }
   }

   if ( name.size() )
   {
      // Add to keyword list.
      std::string key;
      if ( prefix.size() )
      {
         key += prefix;
      }
      key += object;
      key += ".name";
      kwl.addPair( key, name );
      result = true;
   }
   
   return result;
}

bool ossimWkt::parseParam( std::istringstream& is,
                           const std::string& prefix,
                           const std::string& object,
                           ossim_uint32& objectIndex,
                           ossim_uint32& paramIndex,
                           ossimKeywordlist& kwl )
{
   bool result = false;
   char c;
   std::string name;

   // Get the name:
   while ( is.good() )
   {
      int i = is.peek(); // Don't gobble the trailing comma or bracket.
      
      if ( (i == ',') || (i == ']') || (i == ')') )
      {
         // End of param.
         if ( name.size() )
         {
            // Add to keyword list.
            std::string key;
            if ( prefix.size() )
            {
               key += prefix;
            }
            key += object;
            key += ".param";
            key += ossimString::toString(paramIndex).string();
            kwl.addPair( key, name );
            name = "";
            ++paramIndex;
            result = true;
         }
         
         break; // Next param or at end of object.
      }

      is.get(c);
      if ( is.good() )
      {
         // Look nexted object.
         if ( (c == '[') || (c == '(') )
         {
            std::string myPrefix;
            if ( prefix.size() )
            {
               myPrefix += prefix;
            }
            
            myPrefix += object;

            myPrefix += ".";

            //---
            // Special hack for duplicated keyword "PARAMETER"
            //---
            if ( name == "PARAMETER" )
            {
               name += ossimString::toString(objectIndex).string();
               ++objectIndex;
            }

            result = parseObject( is, myPrefix, name, kwl );

            name = "";
         }
         else
         {
            name.push_back(c);
         }
      }
   }
   
   return result;
}

