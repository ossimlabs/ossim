//----------------------------------------------------------------------------
//
// File: ossimLasReader.cpp
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM LAS LIDAR reader.
//
//----------------------------------------------------------------------------
// $Id$

#include <ossim/imaging/ossimLasReader.h>
#include <ossim/base/ossimBooleanProperty.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageGeometryRegistry.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/support_data/ossimFgdcTxtDoc.h>
#include <ossim/support_data/ossimLasHdr.h>
#include <ossim/support_data/ossimLasPointRecordInterface.h>
#include <ossim/support_data/ossimLasPointRecord0.h>
#include <ossim/support_data/ossimLasPointRecord1.h>
#include <ossim/support_data/ossimLasPointRecord2.h>
#include <ossim/support_data/ossimLasPointRecord3.h>
#include <ossim/support_data/ossimLasPointRecord4.h>
#include <ossim/base/ossimConstants.h>

#include <ossim/support_data/ossimTiffInfo.h>

#include <fstream>
#include <limits>
#include <sstream>

RTTI_DEF1(ossimLasReader, "ossimLasReader", ossimImageHandler)

static ossimTrace traceDebug("ossimLasReader:debug");

static const char GSD_KW[]  = "gsd";
static const char SCAN_KW[] = "scan"; // boolean

ossimLasReader::ossimLasReader()
   : ossimImageHandler(),
     m_str(),
     m_hdr(0),
     m_proj(0),
     m_ul(),
     m_lr(),
     m_maxZ(0.0),
     m_minZ(0.0),
     m_gsd(),
     m_tile(0),
     m_entry(0),
     m_mutex(),
     m_scan(false), // ???
     m_units(OSSIM_METERS),
     m_unitConverter(0)   
{
   //---
   // Nan out as can be set in several places, i.e. setProperty,
   // loadState and initProjection.
   //---
   m_gsd.makeNan();
}

ossimLasReader::~ossimLasReader()
{
   close();
}

bool ossimLasReader::open()
{
   static const char M[] = "ossimLasReader::open";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " entered...\nfile: " << theImageFile << "\n";
   }
   
   bool result = false;

   close();

   m_str.open(theImageFile.c_str(), std::ios_base::in | std::ios_base::binary);
   if ( m_str.good() )
   {
      m_hdr = new ossimLasHdr();
      if ( m_hdr->checkSignature( m_str ) )
      {
         m_str.seekg(0, std::ios_base::beg);
         m_hdr->readStream(m_str);
         ossim_uint32 dataFormatId = m_hdr->getPointDataFormatId();

         if ( (dataFormatId == 0) || (dataFormatId == 1) || (dataFormatId == 2) ||
              (dataFormatId == 3) || (dataFormatId == 4) )
         {
            result = init();
            
            if ( result )
            {
               establishDecimationFactors();
               
               if ( traceDebug() )
               {
                  ossimNotify(ossimNotifyLevel_DEBUG) << *m_hdr << "\n";
               }
            }
         }
         else
         {
            if ( traceDebug() )
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << "Unhandled point type: " << int(m_hdr->getPointDataFormatId()) << "\n";
            }
         }
      
         if ( traceDebug() && result )
         {
            ossimNotify(ossimNotifyLevel_DEBUG) << *m_hdr << "\n";
         }
         
      } // if ( m_hdr->checkSignature( m_str ) )
      
   } // if ( m_str.good() )

   if ( !result ) close();

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exit status = " << (result?"true\n":"false\n");
   }
   
   return result;
}

void ossimLasReader::completeOpen()
{
   establishDecimationFactors();
}

void ossimLasReader::close()
{
   if ( isOpen() )
   {
      m_str.close();
      delete m_hdr;
      m_hdr = 0;
      m_entry = 0;
      m_tile  = 0;
      m_proj  = 0;
      ossimImageHandler::close();
   }
}

ossimRefPtr<ossimImageData> ossimLasReader::getTile(
   const  ossimIrect& tile_rect, ossim_uint32 resLevel)
{
   if ( m_tile.valid() == false )
   {
      initTile(); // First time through.
   }
   
   if ( m_tile.valid() )
   {
      // Image rectangle must be set prior to calling getTile.
      m_tile->setImageRectangle(tile_rect);

      if ( getTile( m_tile.get(), resLevel ) == false )
      {
         if (m_tile->getDataObjectStatus() != OSSIM_NULL)
         {
            m_tile->makeBlank();
         }
      }
   }

   return m_tile;
}

bool ossimLasReader::getTile(ossimImageData* result, ossim_uint32 resLevel)
{
   // static const char MODULE[] = "ossimLibLasReader::getTile(ossimImageData*, level)";

   bool status = false;


   if ( m_hdr && result && (result->getScalarType() == OSSIM_FLOAT32||result->getScalarType() == OSSIM_UINT16) &&
        (result->getDataObjectStatus() != OSSIM_NULL) &&
        !m_ul.hasNans() && !m_gsd.hasNans() )
   {
      status = true;
      
      const ossimIrect  TILE_RECT   = result->getImageRectangle();
      const ossim_int32 TILE_HEIGHT = static_cast<ossim_int32>(TILE_RECT.height());
      const ossim_int32 TILE_WIDTH  = static_cast<ossim_int32>(TILE_RECT.width());
      const ossim_int32 TILE_SIZE   = static_cast<ossim_int32>(TILE_RECT.area());

      const ossim_uint16 ENTRY = m_entry;

      // Get the scale for this resLevel:
      ossimDpt scale;
      getScale(scale, resLevel);
      
      // Set the starting upper left of upper left pixel for this tile.
      const ossimDpt UL_PROG_PT( m_ul.x - scale.x / 2.0 + TILE_RECT.ul().x * scale.x,
                                 m_ul.y + scale.y / 2.0 - TILE_RECT.ul().y * scale.y);
      //const ossimDpt UL_PROG_PT( m_ul.x + TILE_RECT.ul().x * scale.x,
      //                           m_ul.y + scale.y / 2.0 - TILE_RECT.ul().y * scale.y);

      //---
      // Set the lower right to the edge of the tile boundary.  This looks like an
      // "off by one" error but it's not.  We want the ossimDrect::pointWithin to
      // catch any points in the last line sample.
      //---
      const ossimDpt LR_PROG_PT( UL_PROG_PT.x + TILE_WIDTH  * scale.x,
                                 UL_PROG_PT.y - TILE_HEIGHT * scale.y);
      
      const ossimDrect PROJ_RECT(UL_PROG_PT, LR_PROG_PT, OSSIM_RIGHT_HANDED);

#if 0  /* Please leave for debug. (drb) */
      cout << "m_ul: " << m_ul
           << "\nm_gsd: " << m_gsd
           << "\nscale:   " << scale
           << "\nresult->getScalarType(): " << result->getScalarType()
           << "\nresult->getDataObjectStatus(): " << result->getDataObjectStatus()
           << "\nPROJ_RECT: " << PROJ_RECT
           << "\nTILE_RECT: " << TILE_RECT
           << "\nUL_PROG_PT: " << UL_PROG_PT << endl;
#endif

      const ossim_float64 SCALE_X  = m_hdr->getScaleFactorX();
      const ossim_float64 SCALE_Y  = m_hdr->getScaleFactorY();
      const ossim_float64 SCALE_Z  = m_hdr->getScaleFactorZ();
      const ossim_float64 OFFSET_X = m_hdr->getOffsetX();
      const ossim_float64 OFFSET_Y = m_hdr->getOffsetY();
      const ossim_float64 OFFSET_Z = m_hdr->getOffsetZ();

      // Create array of buckets.
      std::vector<ossimLasReader::Bucket> bucket( TILE_SIZE );

      // Loop through the point data.
      ossimLasPointRecordInterface* lasPtRec = getNewPointRecord();
      ossimDpt lasPt;

      m_str.clear();
      m_str.seekg(m_hdr->getOffsetToPointData());

      while ( m_str.good() )
      {
         // m_str.read((char*)lasPtRec, 28);
         lasPtRec->readStream( m_str );

         //if ( lasPtRec->getReturnNumber() == ENTRY )
         //{
            lasPt.x = lasPtRec->getX() * SCALE_X + OFFSET_X;
            lasPt.y = lasPtRec->getY() * SCALE_Y + OFFSET_Y;
            if ( m_unitConverter )
            {
               convertToMeters(lasPt.x);
               convertToMeters(lasPt.y);
            }
            if ( PROJ_RECT.pointWithin( lasPt ) )
            {
               // Compute the bucket index:
               ossim_int32 line = static_cast<ossim_int32>((UL_PROG_PT.y - lasPt.y) / scale.y);
               ossim_int32 samp = static_cast<ossim_int32>((lasPt.x - UL_PROG_PT.x) / scale.x );
               ossim_int32 bucketIndex = line * TILE_WIDTH + samp;
               
               // Range check and add if in there.
               if ( ( bucketIndex >= 0 ) && ( bucketIndex < TILE_SIZE ) )
               {
                  ossim_float64 z = lasPtRec->getZ() * SCALE_Z + OFFSET_Z;
                  if (  m_unitConverter ) convertToMeters(z);
                  bucket[bucketIndex].add( z ); 
        bucket[bucketIndex].setRed(lasPtRec->getRed());
        bucket[bucketIndex].setGreen(lasPtRec->getGreen());
        bucket[bucketIndex].setBlue(lasPtRec->getBlue());
        bucket[bucketIndex].setIntensity(lasPtRec->getIntensity());

               }
            }
         //}
         if ( m_str.eof() ) break;
      }
      delete lasPtRec;
      lasPtRec = 0;

      //---
      // We must always blank out the tile as we may not have a point for every
      // point.
      //---
      result->makeBlank();

      //ossim_float32* buf = result->getFloatBuf(); // Tile buffer to fill.
      if(m_entry == 1)
      {
        ossim_uint32 BANDS = getNumberOfOutputBands();
        std::vector<ossim_uint16> tempBuf(TILE_SIZE * BANDS);
   ossim_uint16* buffer = &tempBuf.front();
   for (ossim_int32 band = 0; band < BANDS; ++band)
   {
     for (ossim_int32 i = 0; i < TILE_SIZE; ++i)
     {
       if(band == 0) buffer[i] = bucket[i].getRed();
       if(band == 1) buffer[i] = bucket[i].getGreen();
       if(band == 2) buffer[i] = bucket[i].getBlue();
     }
        }
   result->loadTile(buffer, TILE_RECT, TILE_RECT, OSSIM_BIP);
      }
      else if (m_entry == 2)
      {
   ossim_uint16* buf = result->getUshortBuf();
   for (ossim_int32 i = 0; i < TILE_SIZE; ++i)
   {
     buf[i] = bucket[i].getIntensity();
   }
      }
      else
      {
   ossim_float32* buf = result->getFloatBuf();
      
        // Fill the tile.  Currently no band loop:
        for (ossim_int32 i = 0; i < TILE_SIZE; ++i)
        {
           buf[i] = bucket[i].getValue();
        }
      }

      // Revalidate.
      result->validate();
   }

   return status;
   
} // End: bool ossimLibLasReader::getTile(ossimImageData* result, ossim_uint32 resLevel)

ossim_uint32 ossimLasReader::getNumberOfInputBands() const
{
   return 1; // tmp
}

ossim_uint32 ossimLasReader::getNumberOfOutputBands() const
{
  if(m_entry == 1) return 3;
  return 1;
}

ossim_uint32 ossimLasReader::getNumberOfLines(ossim_uint32 resLevel) const
{
   ossim_uint32 result = 0;
   if ( isOpen() )
   {
      result = static_cast<ossim_uint32>(ceil((m_ul.y - m_lr.y) / m_gsd.y));
      if (resLevel) result = (result>>resLevel);
   }
   return result;
}

ossim_uint32 ossimLasReader::getNumberOfSamples(ossim_uint32 resLevel) const
{
   ossim_uint32 result = 0;
   if ( isOpen() )
   {
      result = static_cast<ossim_uint32>(ceil((m_lr.x - m_ul.x) / m_gsd.x));
      if (resLevel) result = (result>>resLevel);
   }
   return result;
}

ossim_uint32 ossimLasReader::getImageTileWidth() const
{
   return 0;
}

ossim_uint32 ossimLasReader::getImageTileHeight() const
{
   return 0;
}

ossim_uint32 ossimLasReader::getTileWidth() const
{
   ossimIpt ipt;
   ossim::defaultTileSize(ipt);
   return ipt.x;
}

ossim_uint32 ossimLasReader::getTileHeight() const
{
   ossimIpt ipt;
   ossim::defaultTileSize(ipt);
   return ipt.y; 
}

ossimScalarType ossimLasReader::getOutputScalarType() const
{
   //return OSSIM_FLOAT32;
   ossimScalarType stype = OSSIM_FLOAT32;
   if(m_entry == 1 || m_entry == 2) stype = OSSIM_UINT16;
   return stype;
}

void ossimLasReader::getEntryList(std::vector<ossim_uint32>& entryList)const
{
   if ( isOpen() )
   {
      entryList.push_back(0);
      entryList.push_back(1);
      entryList.push_back(2);
      //for ( ossim_uint32 entry = 0; entry < 15; ++entry )
      //{
      //   if ( m_hdr->getNumberOfPoints(entry) ) entryList.push_back(entry);
      //}
   }
   else
   {
      entryList.clear();
   }
}

ossim_uint32 ossimLasReader::getCurrentEntry() const
{
   return static_cast<ossim_uint32>(m_entry);
}

bool ossimLasReader::setCurrentEntry(ossim_uint32 entryIdx)
{
   bool result = false;
   if ( isOpen() )
   {
      std::vector<ossim_uint32> entryList;
      getEntryList( entryList );
      std::vector<ossim_uint32>::const_iterator i = entryList.begin();
      while ( i != entryList.end() )
      {
         if ( (*i) == entryIdx )
         {
            m_entry = entryIdx;
            result = true;
         }
         ++i;
      }
   }
   if(result) initTile();
   return result;
}

ossimString ossimLasReader::getShortName() const
{
   return ossimString("las");
}
   
ossimString ossimLasReader::getLongName()  const
{
   return ossimString("ossim las reader");
}

ossimRefPtr<ossimImageGeometry> ossimLasReader::getImageGeometry()
{
   if ( !theGeometry )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();
      if ( theGeometry.valid() == true )
      {
         // Picked up an external geometry file(dot.geom).
         m_proj = theGeometry->getProjection();

         ossimRefPtr<ossimMapProjection> proj = dynamic_cast<ossimMapProjection*>( m_proj.get() );
         if ( proj.valid() == true )
         {
            // Set the units:
            if ( proj->isGeographic() )
            {
               m_units = OSSIM_DEGREES;
            }
            else
            {
               // Currently hard coding to meters. May need to add property to override this.
               m_units = OSSIM_METERS;
            }

            // Call initValues to set the tie point / bounds:
            initValues();

            // Set the tie and gsd:
            if ( proj->isGeographic() )
            {
               m_units = OSSIM_DEGREES;
               ossimGpt gpt(m_ul.y, m_ul.x, 0.0, proj->getDatum() );
               proj->setUlTiePoints( gpt );
               theGeometry->getDegreesPerPixel( m_gsd );
            }
            else
            {
               // Currently hard coding to meters. May need to add property to override this.
               m_units = OSSIM_METERS;
               proj->setUlTiePoints(m_ul);
               theGeometry->getMetersPerPixel( m_gsd );
            }

            ossimIpt imgSize;
            imgSize.x = (ossim_int32)getNumberOfSamples(0);
            imgSize.y = (ossim_int32)getNumberOfLines(0);

            theGeometry->setImageSize( imgSize );
         }
      }

      if ( !theGeometry )
      {
         theGeometry = new ossimImageGeometry();
         if ( m_proj.valid() )
         {
            theGeometry->setProjection( m_proj.get() );
         }
         else
         {
            //---
            // WARNING:
            // Must create/set theGeometry at this point or the next call to 
            // ossimImageGeometryRegistry::extendGeometry will put us in an infinite loop
            // as it does a recursive call back to ossimImageHandler::getImageGeometry().
            //---         

            // Try factories for projection.
            ossimImageGeometryRegistry::instance()->extendGeometry(this);
         }
      }
      
      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }
   
   return theGeometry;
}

double ossimLasReader::getMinPixelValue(ossim_uint32 /* band */) const
{
   return m_minZ;
}

double ossimLasReader::getMaxPixelValue(ossim_uint32 /* band */) const
{
   return m_maxZ;
}

double ossimLasReader::getNullPixelValue(ossim_uint32 /* band */) const
{
   return -99999.0;
}

ossim_uint32 ossimLasReader::getNumberOfDecimationLevels() const
{
   // Can support any number of rlevels.
   ossim_uint32 result = 1;
   const ossim_uint32 STOP_DIMENSION = 16;
   ossim_uint32 largestImageDimension = getNumberOfSamples(0) > getNumberOfLines(0) ?
      getNumberOfSamples(0) : getNumberOfLines(0);
   while(largestImageDimension > STOP_DIMENSION)
   {
      largestImageDimension /= 2;
      ++result;
   }
   return result; 
}

bool ossimLasReader::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   kwl.add( prefix, GSD_KW, m_gsd.toString().c_str(), true );
   kwl.add( prefix, SCAN_KW,  ossimString::toString(m_scan).c_str(), true );
   return ossimImageHandler::saveState(kwl, prefix);
}

bool ossimLasReader::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   bool result = false;
   if ( ossimImageHandler::loadState(kwl, prefix) )
   {
      result = open();
      if ( result )
      {
         // Get our keywords:
         const char* lookup = kwl.find(prefix, GSD_KW);
         if ( lookup )
         {
            m_gsd.toPoint( ossimString(lookup) );
         }
         lookup = kwl.find(prefix, SCAN_KW);
         if ( lookup )
         {
            ossimString s = lookup;
            m_scan = s.toBool();
         }
      }
   }
   return result;
}

void ossimLasReader::setProperty(ossimRefPtr<ossimProperty> property)
{
   if ( property.valid() )
   {
      if ( property->getName() == GSD_KW )
      {
         ossimString s;
         property->valueToString(s);
         ossim_float64 d = s.toFloat64();
         if ( ossim::isnan(d) == false )
         {
            setGsd( d );
         }
      }
      else if ( property->getName() == SCAN_KW )
      {
         ossimString s;
         property->valueToString(s);
         m_scan = s.toBool();
      }
      else
      {
         ossimImageHandler::setProperty(property);
      }
   }
}

ossimRefPtr<ossimProperty> ossimLasReader::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> prop = 0;
   if ( name == GSD_KW )
   {
      ossimString value = ossimString::toString(m_gsd.x);
      prop = new ossimStringProperty(name, value);
   }
   else if ( name == SCAN_KW )
   {
      prop = new ossimBooleanProperty(name, m_scan);
   }
   else
   {
      prop = ossimImageHandler::getProperty(name);
   }
   return prop;
}

void ossimLasReader::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   propertyNames.push_back( ossimString(GSD_KW) );
   propertyNames.push_back( ossimString(SCAN_KW) );
   ossimImageHandler::getPropertyNames(propertyNames);
}

bool ossimLasReader::init()
{
   bool result = false;

   if ( isOpen() )
   {
      // Check for external geometry file for projection stuff.
      ossimFilename geomFile;
      getFilenameWithThisExt( ossimString(".geom" ), geomFile );
      if ( geomFile.exists() == true )
      {
         // Call get image geometry to initialize ourself.
         ossimRefPtr<ossimImageGeometry> geom = getImageGeometry();
         if ( geom.valid() == true )
         {
            // Check for map projection.
            ossimRefPtr<ossimProjection> proj = geom->getProjection();
            if ( proj.valid() == true )
            {
               if ( dynamic_cast<ossimMapProjection*>( proj.get() ) )
               {
                  result = true;
               }
            }
         }
      }

      // There is nothing we can do if parseVarRecords fails.
      // VAR record is optional, so guess the projection
      // Moved to setCurrentEntry
      //if ( result )
      //{
      //   initTile();
      //}
   }
   
   return result;
}

bool ossimLasReader::initProjection()
{
   bool result = true;
   
   ossimMapProjection* proj = dynamic_cast<ossimMapProjection*>( m_proj.get() );
   if ( proj )
   {
      //---
      // Set the tie and scale:
      // Note the scale can be set in other places so only set here if it
      // has nans.
      //---
      if ( proj->isGeographic() )
      {
         ossimGpt gpt(m_ul.y, m_ul.x, 0.0, proj->getDatum() );
         proj->setUlTiePoints( gpt );

         if ( m_gsd.hasNans() )
         {
            m_gsd = proj->getDecimalDegreesPerPixel();
            if ( m_gsd.hasNans() || !m_gsd.x || !m_gsd.y )
            {
               // Set to some default:
               m_gsd.x = 0.000008983; // About 1 meter at the Equator.
               m_gsd.y = m_gsd.x;
               proj->setDecimalDegreesPerPixel( m_gsd );
            }
            
         }
      }
      else
      {
         proj->setUlTiePoints(m_ul);

         if ( m_gsd.hasNans() )
         {
            m_gsd = proj->getMetersPerPixel();
            if ( m_gsd.hasNans() || !m_gsd.x || !m_gsd.y )
            {
               // Set to some default:
               m_gsd.x = 1.0;
               m_gsd.y = 1.0;
               proj->setMetersPerPixel( m_gsd );
            }
         }
      }
   }
   else
   {
      result = false;
      m_ul.makeNan();
      m_lr.makeNan();
      m_gsd.makeNan();
      
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimLasReader::initProjection WARN Could not cast to map projection!"
         << std::endl;
   }

   return result;
   
} // bool ossimLasReader::initProjection()

void ossimLasReader::initTile()
{
   ossim_uint32 BANDS = getNumberOfOutputBands();

   m_tile = new ossimImageData(this,
                               getOutputScalarType(),
                               BANDS,
                               getTileWidth(),
                               getTileHeight());

   for(ossim_uint32 band = 0; band < BANDS; ++band)
   {
      if (m_entry == 0 || m_entry == 1)
      {
        m_tile->setMinPix(getMinPixelValue(band),   band);
        m_tile->setMaxPix(getMaxPixelValue(band),   band);
        m_tile->setNullPix(getNullPixelValue(band), band);
      }
   }

   m_tile->initialize();
}

void ossimLasReader::initUnits(const ossimKeywordlist& geomKwl)
{
   ossimMapProjection* proj = dynamic_cast<ossimMapProjection*>( m_proj.get() );
   if ( proj )
   {
      if ( proj->isGeographic() )
      {
         m_units = OSSIM_DEGREES;
      }
      else
      {
         const char* lookup = geomKwl.find("image0.linear_units");
         if ( lookup )
         {
            std::string units = lookup;
            if ( units == "meters" )
            {
               m_units = OSSIM_METERS;
            }  
            else if ( units == "feet" )
            {
               m_units = OSSIM_FEET;
            }
            else if ( units == "us_survey_feet" )
            {
               m_units = OSSIM_US_SURVEY_FEET;
            }
            else
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << "ossimLibLasReader::initUnits WARN:\n"
                  << "Unhandled linear units code: " << units << std::endl;
            }
         }
      }
   }

   // Don't make a unit converter for decimal degrees...
   if ( (m_units != OSSIM_DEGREES) && (m_units != OSSIM_METERS) && !m_unitConverter )
   {
      m_unitConverter = new ossimUnitConversionTool();
   }
}

void ossimLasReader::initValues()
{
   static const char M[] = "ossimLasReader::initValues";

   if ( m_scan )
   {
      // Set to bogus values to start.
      m_ul.x = numeric_limits<ossim_float64>::max();
      m_ul.y = numeric_limits<ossim_float64>::min();
      m_lr.x = numeric_limits<ossim_float64>::min();
      m_lr.y = numeric_limits<ossim_float64>::max();
      m_maxZ = numeric_limits<ossim_float64>::min();
      m_minZ = numeric_limits<ossim_float64>::max();
      
      const ossim_float64 SCALE_X  = m_hdr->getScaleFactorX();
      const ossim_float64 SCALE_Y  = m_hdr->getScaleFactorY();
      const ossim_float64 SCALE_Z  = m_hdr->getScaleFactorZ();
      const ossim_float64 OFFSET_X = m_hdr->getOffsetX();
      const ossim_float64 OFFSET_Y = m_hdr->getOffsetY();
      const ossim_float64 OFFSET_Z = m_hdr->getOffsetZ();
      
      ossimLasPointRecordInterface* lasPtRec = getNewPointRecord();
      
      m_str.clear();
      m_str.seekg(m_hdr->getOffsetToPointData());
      
      ossim_float64 x;
      ossim_float64 y;
      ossim_float64 z;
      
      while ( m_str.good() )
      {
         lasPtRec->readStream( m_str );
         
#if 0 /* Please leave for debug. (drb) */
         lasPtRec->print( ossimNotify(ossimNotifyLevel_DEBUG) );
#endif
         
         x = lasPtRec->getX() * SCALE_X + OFFSET_X;
         y = lasPtRec->getY() * SCALE_Y + OFFSET_Y;
         z = lasPtRec->getZ() * SCALE_Z + OFFSET_Z;

    //std::cout << "X: " << lasPtRec->getX() << " SCALE: " << SCALE_X << " OFFSET: " << OFFSET_X << " XNORM: " << x << std::endl;
    //std::cout << "Y: " << lasPtRec->getY() << " SCALE: " << SCALE_Y << " OFFSET: " << OFFSET_Y << " YNORM: " << y << std::endl;
    //std::cout << "Z: " << lasPtRec->getZ() << " SCALE: " << SCALE_Z << " OFFSET: " << OFFSET_Z << " ZNORM: " << z << std::endl;
         
         if ( x < m_ul.x ) m_ul.x = x;
         if ( x > m_lr.x ) m_lr.x = x;
         if ( y > m_ul.y ) m_ul.y = y;
         if ( y < m_lr.y ) m_lr.y = y;
         if ( z > m_maxZ ) m_maxZ = z;
         if ( z < m_minZ ) m_minZ = z;
         
         if ( m_str.eof() ) break;
      }
      delete lasPtRec;
      lasPtRec = 0;
   }
   else
   {
      // Set the upper left (tie).
      m_ul.x = m_hdr->getMinX();
      m_ul.y = m_hdr->getMaxY();
      
      // Set the lower right.
      m_lr.x = m_hdr->getMaxX();
      m_lr.y = m_hdr->getMinY();
      
      // Set the min/max:
      m_minZ = m_hdr->getMinZ();
      m_maxZ = m_hdr->getMaxZ();
   }

   if ( m_unitConverter ) // Need to convert to meters.
   {
      convertToMeters(m_ul.x);
      convertToMeters(m_ul.y);
      
      convertToMeters(m_lr.x);
      convertToMeters(m_lr.y);
      
      convertToMeters(m_maxZ);
      convertToMeters(m_minZ);
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " DEBUG:\nBounds from header:";
      ossimDpt pt;
      pt.x = m_hdr->getMinX();
      pt.y = m_hdr->getMaxY();
      if ( m_unitConverter )
      {
         convertToMeters(pt.x);
         convertToMeters(pt.y);
      }
      ossimNotify(ossimNotifyLevel_DEBUG) << "\nul:   " << pt;
      pt.x = m_hdr->getMaxX();
      pt.y = m_hdr->getMinY();

      if ( m_unitConverter )
      {
         convertToMeters(pt.x);
         convertToMeters(pt.y); 
      }
      ossimNotify(ossimNotifyLevel_DEBUG) << "\nlr:   " << pt;
      pt.x = m_hdr->getMinZ();
      pt.y = m_hdr->getMaxZ();
      if ( m_unitConverter )
      {
         convertToMeters(pt.x);
         convertToMeters(pt.y); 
      }
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "\nminZ: " << pt.x
         << "\nmaxZ: " << pt.y << "\n";
      if ( m_scan )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "Bounds from scan:"
            << "\nul:   " << m_ul
            << "\nlr:   " << m_lr
            << "\nminZ: " << m_minZ
            << "\nmaxZ: " << m_maxZ << "\n";
      }
   }
}

bool ossimLasReader::parseVarRecords()
{
   static const char M[] = "ossimLasReader::parseVarRecords";
   if (traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   
   bool result = false;

   if ( isOpen() )
   {
      std::streampos origPos = m_str.tellg();
      std::streamoff pos = static_cast<std::streamoff>(m_hdr->getHeaderSize());

      m_str.clear();
      m_str.seekg(pos, std::ios_base::beg);

      ossim_uint32 vlrCount = m_hdr->getNumberOfVlrs();
      ossim_uint16 reserved;
      char uid[17];
      uid[16]='\n';
      ossim_uint16 recordId;
      ossim_uint16 length;
      char des[33];
      des[32] = '\n';

      //---
      // Things we need to save for printGeoKeys:
      //---
      ossim_uint16*  geoKeyBlock     = 0;
      ossim_uint64   geoKeyLength    = 0;
      ossim_float64* geoDoubleBlock  = 0;
      ossim_uint64   geoDoubleLength = 0;
      ossim_int8*    geoAsciiBlock   = 0;
      ossim_uint64   geoAsciiLength  = 0;
     
      ossimEndian* endian = 0;
      // LAS LITTLE ENDIAN:
      if ( ossim::byteOrder() == OSSIM_BIG_ENDIAN )
      {
         endian = new ossimEndian;
      }
      
      for ( ossim_uint32 i = 0; i < vlrCount; ++i )
      {
         m_str.read((char*)&reserved, 2);
         m_str.read(uid, 16);
         m_str.read((char*)&recordId, 2);
         m_str.read((char*)&length, 2);
         m_str.read(des, 32);

         // LAS LITTLE ENDIAN:
         if ( endian )
         {
            endian->swap(recordId);
            endian->swap(length);
         }
 
         if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "uid:      " << uid
               << "\nrecordId: " << recordId
               << "\nlength:   " << length
               << "\ndes:      " << des
               << std::endl;
         }
         
         if (recordId == 34735) // GeoTiff projection keys.
         {
            geoKeyLength = length/2;
            if ( geoKeyBlock )
            {
               delete [] geoKeyBlock;
            }
            geoKeyBlock = new ossim_uint16[geoKeyLength];
            m_str.read((char*)geoKeyBlock, length);
            if ( endian )
            {
               endian->swap(geoKeyBlock, geoKeyLength);
            }
         }
         else if (recordId == 34736) // GeoTiff double parameters.
         {
            geoDoubleLength = length/8;
            if ( geoDoubleBlock )
            {
               delete [] geoDoubleBlock;
            }
            geoDoubleBlock = new ossim_float64[geoDoubleLength];
            m_str.read((char*)geoDoubleBlock, length);
            if ( endian )
            {
               endian->swap(geoDoubleBlock, geoDoubleLength);
            }
         }
         else if (recordId == 34737) // GeoTiff ascii block.
         {
            geoAsciiLength = length;
            if (geoAsciiBlock)
            {
               delete [] geoAsciiBlock;
            }
            geoAsciiBlock = new ossim_int8[length];
            m_str.read((char*)geoAsciiBlock, length);
         }
         else
         {
            m_str.seekg(length, ios_base::cur);
         }
      }

      //---
      // Must have at mimimum the geoKeyBlock for a projection.
      // Note the geoDoubleBlock is needed for some.
      // Note the geoAsciiBlock is not needed, i.e. only informational.
      //---
      if ( geoKeyBlock )
      {       
         //---
         // Give the geokeys to ossimTiffInfo to get back a keyword list that can be fed to
         // ossimProjectionFactoryRegistry::createProjection
         //---
         ossimTiffInfo info;
         ossimKeywordlist geomKwl;
         info.getImageGeometry(geoKeyLength, geoKeyBlock,
                               geoDoubleLength,geoDoubleBlock,
                               geoAsciiLength,geoAsciiBlock,
                               geomKwl);
         
         // Create the projection.
         m_proj = ossimProjectionFactoryRegistry::instance()->createProjection(geomKwl);
         if (m_proj.valid())
         {
            // Units must be set before initValues and initProjection.
            initUnits(geomKwl);
            
            // Must be called before initProjection.
            initValues();
            
            result = initProjection();  // Sets the ties and scale...
            
            if (traceDebug())
            {
               m_proj->print(ossimNotify(ossimNotifyLevel_DEBUG));
            }
         }
      }
      else
      {
   /*  Current data samples appear to be UTM but not sure where to find the zone
      const char* prefix = "image0.";
           // Build a default projection
           ossimKeywordlist geomKwl;
           geomKwl.add(prefix, "datum", "WGE", true);
           geomKwl.add(prefix, "pixel_type", "pixel_is_area",  true);
           geomKwl.add(prefix, "type", "ossimEquDistCylProjection", true);
           m_proj = ossimProjectionFactoryRegistry::instance()->createProjection(geomKwl);
           if(m_proj.valid())
           {
             initUnits(geomKwl);
             initValues();
             result = initProjection(); // Sets the ties and scale...
             if (traceDebug())
             {
               m_proj->print(ossimNotify(ossimNotifyLevel_DEBUG));
             }
           }
   */
      }

      if ( geoKeyBlock )
      {
         delete [] geoKeyBlock;
         geoKeyBlock = 0;
      }
      if (geoDoubleBlock)
      {
         delete [] geoDoubleBlock;
         geoDoubleBlock = 0;
      }
      if (geoAsciiBlock)
      {
         delete [] geoAsciiBlock;
         geoAsciiBlock = 0;
      }

      m_str.seekg(origPos);

      if ( endian )
      {
         delete endian;
         endian = 0;
      }
   }  

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exit status = " << (result?"true\n":"false\n");
   }   
   return result;
}

bool ossimLasReader::initFromExternalMetadata()
{
   static const char M[] = "ossimLasReader::initFromExternalMetadata";
   if (traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   
   bool result = false;

   ossimFilename fgdcFile = theImageFile;
   fgdcFile.setExtension("txt");
   if ( fgdcFile.exists() == false )
   {
      fgdcFile.setExtension("TXT");
   }

   if ( fgdcFile.exists() )
   {
      ossimRefPtr<ossimFgdcTxtDoc> fgdcDoc = new ossimFgdcTxtDoc();
      if ( fgdcDoc->open( fgdcFile ) )
      {
         fgdcDoc->getProjection( m_proj );
         if ( m_proj.valid() )
         {
            // Units must be set before initValues and initProjection.
            std::string units;
            fgdcDoc->getAltitudeDistanceUnits(units);
            if ( ( units == "feet" ) || ( units == "international feet" ) )
            {
               m_units = OSSIM_FEET;
            }
            else if ( units == "survey feet" )
            {
               m_units = OSSIM_US_SURVEY_FEET;
            }
            else
            {
               m_units = OSSIM_METERS;
            }
            
            // Must be called before initProjection.
            initValues();
            
            result = initProjection();  // Sets the ties and scale...
            
            if (traceDebug())
            {
               m_proj->print(ossimNotify(ossimNotifyLevel_DEBUG));
            }
         }
      }
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exit status = " << (result?"true\n":"false\n");
   }   
   return result;
}

ossimLasPointRecordInterface* ossimLasReader::getNewPointRecord() const
{
   ossimLasPointRecordInterface* result = 0;

   switch(m_hdr->getPointDataFormatId())
   {
      case 0:
      {
         result = new ossimLasPointRecord0();
         break;
      }
      case 1:
      {
         result = new ossimLasPointRecord1();
         break;
      }
      case 2:
      {
         result = new ossimLasPointRecord2();
         break;
      }
      case 3:
      {
         result = new ossimLasPointRecord3();
         break;
      }
      case 4:
      {
         result = new ossimLasPointRecord4();
         break;
      }
      default:
      {
         break;
      }
   }

   return result;
}

void ossimLasReader::getScale(ossimDpt& scale, ossim_uint32 resLevel) const
{
   // std::pow(2.0, 0) returns 1.
   ossim_float64 d = std::pow(2.0, static_cast<double>(resLevel));
   scale.x = m_gsd.x * d;
   scale.y = m_gsd.y * d;
}

void ossimLasReader::setGsd( const ossim_float64& gsd )
{
   m_gsd.x = gsd;
   m_gsd.y = m_gsd.x;

   if ( m_proj.valid() && ( m_gsd.hasNans() == false ) )
   {
      ossimMapProjection* proj = dynamic_cast<ossimMapProjection*>( m_proj.get() );
      if ( proj && ( m_gsd.hasNans() == false ) )
      {
         if ( proj->isGeographic() )
         {
            proj->setDecimalDegreesPerPixel( m_gsd );
         }
         else
         {
            proj->setMetersPerPixel( m_gsd );
         }
      }
   }
}