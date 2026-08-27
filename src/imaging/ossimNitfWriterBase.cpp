//---
//
// License: MIT
// 
// Author:  David Burken
//
// Description: OSSIM nitf writer base class to hold methods common to
// all nitf writers.
//
//---
// $Id$

#include <ossim/imaging/ossimNitfWriterBase.h>
#include <ossim/base/ossimBooleanProperty.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimMapProjectionInfo.h>
#include <ossim/projection/ossimUtmProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimRpcSolver.h>
#include <ossim/support_data/ossimNitfBlockaTag.h>
#include <ossim/support_data/ossimNitfDesInformation.h>
#include <ossim/support_data/ossimNitfFileHeader.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
// #include <ossim/support_data/ossimNitfFileHeaderV2_1.h>
#include <ossim/support_data/ossimNitfGeolobTag.h>
#include <ossim/support_data/ossimNitfImageHeader.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_X.h>
// #include <ossim/support_data/ossimNitfImageHeaderV2_1.h>
#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/support_data/ossimNitfTagInformation.h>
#include <ossim/base/ossim2dBilinearTransform.h>
#include <ossim/support_data/ossimNitfIchipbTag.h>
#include <ossim/base/ossimStringProperty.h>

static const char ENABLE_BLOCKA_KW[] = "enable_blocka_tag";
static const char ENABLE_RPCB_KW[]   = "enable_rpcb_tag";
static const char ENABLE_GEOLOB_KW[] = "enable_geolob_tag";

RTTI_DEF1(ossimNitfWriterBase, "ossimNitfWriterBase", ossimImageFileWriter)
   
static ossimTrace traceDebug(ossimString("ossimNitfWriterBase:debug"));

ossimNitfWriterBase::ossimNitfWriterBase()
   : ossimImageFileWriter(),
     m_enableRpcbTagFlag(false),
     m_enableBlockaTagFlag(true),
     m_enableGeolobTagFlag(true),
     m_tags(0),
     m_fileHeaderProps(0),
     m_imageHeaderProps(0)
{
}

ossimNitfWriterBase::ossimNitfWriterBase(const ossimFilename& filename,
                                         ossimImageSource* inputSource)
   : ossimImageFileWriter(filename, inputSource, 0),
     m_enableRpcbTagFlag(false),
     m_enableBlockaTagFlag(true),
     m_enableGeolobTagFlag(true),
     m_tags(0),
     m_fileHeaderProps(0),
     m_imageHeaderProps(0)
{
}

ossimNitfWriterBase::~ossimNitfWriterBase()
{
}

void ossimNitfWriterBase::setProperty(ossimRefPtr<ossimProperty> property)
{
   if(property.valid())
   {
      ossimString name = property->getName();

      if (name == ENABLE_RPCB_KW)
      {
         m_enableRpcbTagFlag = property->valueToString().toBool();
      }
      else if (name == ENABLE_BLOCKA_KW)
      {
         m_enableBlockaTagFlag = property->valueToString().toBool();
      }
      else if (name == ENABLE_GEOLOB_KW)
      {
         m_enableGeolobTagFlag = property->valueToString().toBool();
      }
      else
      {
         ossimImageFileWriter::setProperty(property);
      }
   }
}

ossimRefPtr<ossimProperty> ossimNitfWriterBase::getProperty(
   const ossimString& name)const
{
   ossimRefPtr<ossimProperty> result = 0;
   
   if(name == ENABLE_BLOCKA_KW)
   {
      result = new ossimBooleanProperty(name, m_enableBlockaTagFlag);
   }
   else if (name == ENABLE_GEOLOB_KW)
   {
      result = new ossimBooleanProperty(name, m_enableGeolobTagFlag);
   }
   else if(name == ENABLE_RPCB_KW)
   {
      result = new ossimBooleanProperty(name, m_enableRpcbTagFlag);
   }
   else
   {
      result = ossimImageFileWriter::getProperty(name);
   }

   return result;
}

void ossimNitfWriterBase::getPropertyNames(
   std::vector<ossimString>& propertyNames)const
{
   ossimImageFileWriter::getPropertyNames(propertyNames);

   propertyNames.push_back(ENABLE_BLOCKA_KW);
   propertyNames.push_back(ENABLE_GEOLOB_KW);
   propertyNames.push_back(ENABLE_RPCB_KW);
}


bool ossimNitfWriterBase::saveState(ossimKeywordlist& kwl,
                                    const char* prefix) const
{
   kwl.add(prefix, ENABLE_BLOCKA_KW, m_enableBlockaTagFlag, true);
   kwl.add(prefix, ENABLE_GEOLOB_KW, m_enableGeolobTagFlag, true);
   kwl.add(prefix, ENABLE_RPCB_KW, m_enableRpcbTagFlag, true);   

   return ossimImageFileWriter::saveState(kwl, prefix);
}

bool ossimNitfWriterBase::loadState(const ossimKeywordlist& kwl,
                                    const char* prefix)
{
   // Look for the rpcb enable flag keyword.


   // Look for the blocka enable flag keyword.
   const char* lookup = kwl.find(prefix, ENABLE_BLOCKA_KW);
   if(lookup)
   {
      ossimString os = lookup;
      m_enableBlockaTagFlag = os.toBool();
   }

   // Look for the geolob enable flag keyword.
   lookup = kwl.find(prefix, ENABLE_GEOLOB_KW);
   if(lookup)
   {
      ossimString os = lookup;
      m_enableGeolobTagFlag = os.toBool();
   }

   lookup = kwl.find(prefix, ENABLE_RPCB_KW);
   if(lookup)
   {
      ossimString os = lookup;
      m_enableRpcbTagFlag = os.toBool();
   }

   return ossimImageFileWriter::loadState(kwl, prefix);
}

void ossimNitfWriterBase::writeGeometry(ossimNitfImageHeaderV2_X* hdr,
                                        ossimImageSourceSequencer* seq)
{
   if (hdr && seq)
   {
      ossimRefPtr<ossimImageGeometry> geom = theInputConnection->getImageGeometry();

      if (geom.valid()&&geom->hasProjection())
      {
         // Get the requested bounding rectangles.
         ossimIrect rect = seq->getBoundingRect();
         
         // See if it's a map projection; else, a sensor model.
         ossimMapProjection* mapProj = PTR_CAST(ossimMapProjection, geom->getProjection());
         if (mapProj)
         {
            // Use map info to get the corners.
            ossimMapProjectionInfo mapInfo(mapProj, rect);
            mapInfo.setPixelType(OSSIM_PIXEL_IS_AREA);
            
            // See if it's utm.
            ossimUtmProjection* utmProj = PTR_CAST(ossimUtmProjection, mapProj);
            if(utmProj)
            {
               ossimDpt ul = mapInfo.ulEastingNorthingPt();
               ossimDpt ur = mapInfo.urEastingNorthingPt();
               ossimDpt lr = mapInfo.lrEastingNorthingPt();
               ossimDpt ll = mapInfo.llEastingNorthingPt();
               
               if(utmProj->getHemisphere() == 'N')
               {
                  hdr->setUtmNorth(utmProj->getZone(), ul, ur, lr, ll);
               }
               else
               {
                  hdr->setUtmSouth(utmProj->getZone(), ul, ur, lr, ll);
               }
            }
            else
            {
               ossimGpt ul = mapInfo.ulGroundPt();
               ossimGpt ur = mapInfo.urGroundPt();
               ossimGpt lr = mapInfo.lrGroundPt();
               ossimGpt ll = mapInfo.llGroundPt();
               hdr->setGeographicLocationDms(ul, ur, lr, ll);
            }

            if (m_enableBlockaTagFlag)
            {
               addBlockaTag(mapInfo, hdr);
            }

            if ( m_enableGeolobTagFlag )
            {
               addGeolobTag( mapInfo, hdr );
            }
         }
         else
         {
            ossimGpt ul, ur, lr, ll;
            bool status = geom->getCornerGpts(ul, ur, lr, ll);
            if (status)
            {
                hdr->setGeographicLocationDms(ul, ur, lr, ll);
            }
         }
         
         if (m_enableRpcbTagFlag)
         {
            addRpcbTag(rect, geom->getProjection(), hdr);
         }
         
      } // matches:  if (proj.valid())

      if ( m_tags.size() )
      {
         const auto& cv = m_tags;
         for ( auto&& i : cv )
         {
            // Add the tag to the header.
            ossimNitfTagInformation tagInfo(i);
            hdr->addTag(i);
         }
      }

      // Apply explicit image-header overrides after geometry has been
      // generated.  This allows specialized writers to intentionally replace
      // fields such as ICORDS/IGEOLO without changing the generic geometry
      // handling above.
      addImageHeaderProperties(hdr);
      
   } // matches: if (hdr && seq)
}

void ossimNitfWriterBase::addBlockaTag(ossimMapProjectionInfo& mapInfo,
                                       ossimNitfImageHeaderV2_X* hdr)
{
   if (hdr)
   {
      // Capture the current pixel type.
      ossimPixelType originalPixelType = mapInfo.getPixelType();
      
      // This tag wants corners as area:
      mapInfo.setPixelType(OSSIM_PIXEL_IS_AREA);
      
      // Stuff the blocka tag which has six digit precision.
      ossimRefPtr<ossimNitfBlockaTag> blockaTag = new ossimNitfBlockaTag();
      
      // Set the block number.
      blockaTag->setBlockInstance(1);
      
      // Set the number of lines.
      blockaTag->setLLines(mapInfo.linesPerImage());
      
      // Set first row, first column.
      blockaTag->setFrfcLoc(ossimDpt(mapInfo.ulGroundPt()));
      
      // Set first row, last column.
      blockaTag->setFrlcLoc(ossimDpt(mapInfo.urGroundPt()));
      
      // Set last row, last column.
      blockaTag->setLrlcLoc(ossimDpt(mapInfo.lrGroundPt()));
      
      // Set last row, first column.
      blockaTag->setLrfcLoc(ossimDpt(mapInfo.llGroundPt()));
      
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimNitfWriterBase::addBlockaTag DEBUG:"
            << "\nBLOCKA Tag:" << *((ossimObject*)(blockaTag.get()))
            << std::endl;
      }
      
      // Add the tag to the header.
      ossimRefPtr<ossimNitfRegisteredTag> blockaTagRp = blockaTag.get();
      ossimNitfTagInformation blockaTagInfo(blockaTagRp);
      hdr->addTag(blockaTagInfo);
      
      // Reset the pixel type to original value
      mapInfo.setPixelType(originalPixelType);
      
   } // matches: if (hdr)
}

void ossimNitfWriterBase::addIchipbTag(ossimImageGeometry* geom,
                                       ossimNitfImageHeaderV2_X* hdr)
{
   // Tag is valid only if the geometry has a 2D transform:
   auto ichipb = new ossimNitfIchipbTag;
   bool tagValid = ichipb->initFromGeometry(geom);
   if (tagValid)
   {
      ossimRefPtr<ossimNitfRegisteredTag> ichipbTag = ichipb;
      ossimNitfTagInformation ichipbTagInfo(ichipbTag);
      hdr->addTag(ichipbTagInfo);

      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimNitfWriterBase::addIchipbTag DEBUG:"
            << "\nICHIPB Tag:" << std::endl;
         ichipb->print(ossimNotify(ossimNotifyLevel_DEBUG));
      }
   }
}

void ossimNitfWriterBase::addGeolobTag(ossimMapProjectionInfo& mapInfo,
                                       ossimNitfImageHeaderV2_X* hdr)
{
   if ( hdr && mapInfo.getProjection() )
   {
      if ( mapInfo.getProjection()->isGeographic() == true )
      {
         // This tag wants corners as area:
         mapInfo.setPixelType(OSSIM_PIXEL_IS_AREA);

         // Get the scale:
         ossimDpt gsd = mapInfo.getDecimalDegreesPerPixel();
         if ( (gsd.hasNans() == false) && (gsd.x > 0.0) && (gsd.y > 0.0) )
         {
            ossimGpt tie = mapInfo.ulGroundPt();
            if ( tie.hasNans() == false )
            {
               if ( (tie.lat <= 90.0) && (tie.lon >= -180.0) )
               {
                  ossimRefPtr<ossimNitfGeolobTag> geolobTag = new ossimNitfGeolobTag();
                  geolobTag->setDegreesPerPixelLon( gsd.x );
                  geolobTag->setDegreesPerPixelLat( gsd.y );
                  geolobTag->setLso( tie.lon ); // Origin Longitude
                  geolobTag->setPso( tie.lat ); // Origin Latitude

                  // Add the tag to the header.
                  ossimRefPtr<ossimNitfRegisteredTag> geolobTagRp = geolobTag.get();
                  ossimNitfTagInformation geolobTagInfo(geolobTagRp);
                  hdr->addTag(geolobTagInfo);

                  if (traceDebug())
                  {
                     ossimNotify(ossimNotifyLevel_DEBUG)
                        << "ossimNitfWriterBase::addGeolobTag DEBUG:"
                        << "\nAdded GEOLOB Tag:\n" << *(geolobTag.get())
                        << "\n";
                  }
               }
            }
         }
      }
   }
}

void ossimNitfWriterBase::addRpcbTag(const ossimIrect& rect,
                                     ossimProjection* proj,
                                     ossimNitfImageHeaderV2_X* hdr)
{
   if (proj && hdr)
   {
      bool useElevation = false;
      
      if (PTR_CAST(ossimMapProjection, proj))
      {
         // If we're already map projected turn the elevation off.
         useElevation = false;
      }
      
      // Make an rpc solver.
      ossimRefPtr<ossimRpcSolver> rs = new ossimRpcSolver(useElevation);
      
      // Compute the coefficients.
      rs->solveCoefficients(ossimDrect(rect), proj, 64, 64);
      
      // Add the tag.
      ossimRefPtr<ossimNitfRegisteredTag> tag = rs->getNitfRpcBTag();
      ossimNitfTagInformation tagInfo(tag);
      hdr->addTag(tagInfo);
      
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimNitfWriterBase::addRpcbTag DEBUG:"
            << "\nRPCB Tag:" << *((ossimObject*)(tag.get()))
            << "\nProjection:\n";
         
         proj->print(ossimNotify(ossimNotifyLevel_DEBUG));
         
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "\nRect: " << rect << std::endl;
      }
      
   } // matches: if (proj && hdr)
}

void ossimNitfWriterBase::setComplexityLevel(std::streamoff endPosition,
                                             ossimNitfFileHeaderV2_X* hdr)
{
   if (hdr)
   {
      //---
      // See MIL-STD-2500C, Table A-10:
      //
      // Lots of rules here, but for now we will key off of file size.
      //---
      const std::streamoff MB   = 1024 * 1024;
      const std::streamoff MB50 = 50   * MB;
      const std::streamoff GIG  = 1000 * MB;
      const std::streamoff GIG2 = 2    * GIG;
      
      ossimString complexity = "03"; // Less than 50 mb.
      
      if ( (endPosition >= MB50) && (endPosition < GIG) )
      {
         complexity = "05";
      }
      else if ( (endPosition >= GIG) && (endPosition < GIG2) )
      {
         complexity = "06";
      }
      else if (endPosition >= GIG2)
      {
         complexity = "07";
      }
      
      hdr->setComplexityLevel(complexity);
   }
}

// Alternate calculation option for complexity.  This seems to match more test data.
void ossimNitfWriterBase::setComplexityLevel(ossimNitfFileHeaderV2_X* hdr, ossim_uint64 width, ossim_uint64 height)
{
   if (hdr)
   {
      ossim_uint64 max = (width > height) ? width : height;
      ossimString complexity = "03";
      if (max > 65536) complexity = "07";
      else if (max > 8192) complexity = "06";
      else if (max > 2048) complexity = "05";

      hdr->setComplexityLevel(complexity);
   }
}

ossimString ossimNitfWriterBase::getExtension() const
{
   return ossimString("ntf");
}

void ossimNitfWriterBase::addRegisteredTag(ossimRefPtr<ossimNitfRegisteredTag> registeredTag )
{
   m_tags.push_back( registeredTag );
}

void ossimNitfWriterBase::addRegisteredTag(
   ossimRefPtr<ossimNitfRegisteredTag> registeredTag, bool unique )
{
   addRegisteredTag(registeredTag, unique, 1, ossimString("IXSHD"));
}

void ossimNitfWriterBase::addDesInfo(const ossimNitfDesInformation& /* des */)
{
   ossimNotify(ossimNotifyLevel_WARN)
      << getClassName() << " WARNING:\n"
      << "addDesInfo() not implemented!!!" << std::endl;
}

#if 0 /* Not called by anyone. */
void ossimNitfWriterBase::setFileHeaderV2_1(ossimRefPtr<ossimNitfFileHeaderV2_1>, bool /* preferSource */)
{
}

void ossimNitfWriterBase::setImageHeaderV2_1(ossimRefPtr<ossimNitfImageHeaderV2_1>, bool /* preferSource */)
{
}
#endif

void ossimNitfWriterBase::initializeDefaultsFromConfigFile( ossimNitfFileHeaderV2_X* fileHdr,
                                                            ossimNitfImageHeaderV2_X* imgHdr )
{
   // Look in prefs for site configuration file:
   const char* lookup = ossimPreferences::instance()->
      findPreference("nitf_writer.site_configuration_file");
   if ( lookup && fileHdr && imgHdr )
   {
      ossimKeywordlist kwl;
      if ( kwl.addFile( lookup ) )
      {
         fileHdr->loadState( kwl, "nitf.file." );
         imgHdr->loadState( kwl, "nitf.image." );
      }
   }
}

// public:
void ossimNitfWriterBase::setFileHeaderProperty( ossimRefPtr<ossimProperty> property )
{
   m_fileHeaderProps.push_back( property );
}

// public:
void ossimNitfWriterBase::setImageHeaderProperty( ossimRefPtr<ossimProperty> property )
{
   m_imageHeaderProps.push_back( property );
}

// protected:
void ossimNitfWriterBase::addFileHeaderProperties( ossimNitfFileHeaderV2_X* hdr )
{
   if ( hdr && m_fileHeaderProps.size() )
   {
      const auto& cv = m_fileHeaderProps;
      for ( auto&& i : cv )
      {
         hdr->setProperty( i );
      }
   }
}

// protected:
void ossimNitfWriterBase::addImageHeaderProperties( ossimNitfImageHeaderV2_X* hdr )
{
   if ( hdr && m_imageHeaderProps.size() )
   {
      const auto& cv = m_imageHeaderProps;
      for ( auto&& i : cv )
      {
         hdr->setProperty( i );
      }
   }
}
