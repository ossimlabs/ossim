//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Image handler class for a Shuttle Radar Topography Mission (SRTM) file.
//
//----------------------------------------------------------------------------
// $Id: ossimSrtmTileSource.cpp 22769 2014-05-08 19:35:04Z dburken $

#include <ossim/imaging/ossimSrtmTileSource.h>
#include <ossim/base/ossimDirectory.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/support_data/ossimSrtmSupportData.h>

RTTI_DEF1(ossimSrtmTileSource,
          "ossimSrtmTileSource",
          ossimGeneralRasterTileSource)

static ossimTrace traceDebug("ossimSrtmTileSource:debug");

ossimSrtmTileSource::ossimSrtmTileSource()
   :
   ossimGeneralRasterTileSource(),
   m_SrtmSupportData()
{
}

ossimSrtmTileSource::~ossimSrtmTileSource()
{
}

bool ossimSrtmTileSource::open()
{
   static const char MODULE[] = "ossimSrtmTileSource::open";
   if (traceDebug())
   {
      CLOG << " Entered..." << std::endl
           << "srtm " << theImageFile << std::endl;
   }
   
   if(isOpen())
   {
      close();
   }

   bool result = false;
   
   //---
   // Set the scan for min max flag to true.  This will only scan if not found
   // in a omd file so this is a one time hit typically taken when building
   // reduced res sets.
   //---
   if ( m_SrtmSupportData.setFilename(theImageFile, true) )
   {
      ossimKeywordlist kwl;
      m_SrtmSupportData.saveState(kwl);

      ossimGeneralRasterInfo generalRasterInfo;
      if ( generalRasterInfo.loadState(kwl) )
      {
         theMetaData = generalRasterInfo.getImageMetaData();

         result =  ossimGeneralRasterTileSource::open(generalRasterInfo);
      }
   }
   else
   {
      if (traceDebug()) CLOG << " Unable to set filename"<< std::endl;
      
      return false;
   }

   if (traceDebug())
   {
      CLOG << " Exited with " << (result?"true":"false") << " ..."
           << std::endl;
   }
   
   return result;
}
   
//************************************************************************************************
//! Returns the image geometry object associated with this tile source or NULL if non defined.
//! The geometry contains full-to-local image transform as well as projection (image-to-world)
//************************************************************************************************
ossimRefPtr<ossimImageGeometry> ossimSrtmTileSource::getImageGeometry()
{
   if(!theGeometry.valid())
   {
      // First time through set the projection.
      theGeometry = new ossimImageGeometry();

      theGeometry->setProjection( m_SrtmSupportData.getProjection().get() );

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }
   return theGeometry;
}

bool ossimSrtmTileSource::saveState(ossimKeywordlist& kwl,
                                    const char* prefix)const
{
   if (m_SrtmSupportData.saveState(kwl, prefix))
   {
      return ossimImageHandler::saveState(kwl,prefix);
   }
   return false;
}

bool ossimSrtmTileSource::loadState(const ossimKeywordlist& kwl,
                                    const char* prefix)
{
   bool result = false;
   
   std::string pfx = "";
   if (prefix) // Cannot give null to std::string.
   {
      pfx = prefix;
   }

   std::string key   = ossimKeywordNames::TYPE_KW;
   std::string value = kwl.findKey( pfx, key );
   if ( value == "ossimSrtmTileSource" )
   {
      key = ossimKeywordNames::FILENAME_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         // Set the file name:
         theImageFile = value;

         // Call ossimSrtmTileSource::open which will do the rest of the work.
         if ( open() )
         {
            // Must call to pick up id for connections.
            result = ossimImageSource::loadState(kwl, prefix);
         }
      }
   }

   return result;
}

ossimRefPtr<ossimProperty> ossimSrtmTileSource::getProperty(
   const ossimString& name)const
{
   if(name == "file_type")
   {
      return new ossimStringProperty(name, getShortName());
   }
   
   return ossimImageHandler::getProperty(name);
}

void ossimSrtmTileSource::getPropertyNames(
   std::vector<ossimString>& propertyNames)const
{
   ossimImageHandler::getPropertyNames(propertyNames);
   propertyNames.push_back("file_type");
}

ossimString ossimSrtmTileSource::getShortName() const
{
   return ossimString("SRTM");
}
   
ossimString ossimSrtmTileSource::getLongName() const
{
   return ossimString("SRTM reader");
}
 
