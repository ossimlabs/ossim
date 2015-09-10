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
// Generic geoid source which uses an image handler for reading the grid.
// 
//----------------------------------------------------------------------------

#include <ossim/base/ossimGeoidImage.h>

#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/projection/ossimProjection.h>
#include <cmath>

ossimGeoidImage::ossimGeoidImage()
   : m_geom(0),
     m_handler(0),
     m_cacheTile(0),
     m_geoidTypeName(),
     m_memoryMapFlag(false),
     m_enabledFlag(true),
     m_imageRect()
{
}

ossimGeoidImage::~ossimGeoidImage()
{
   m_geom = 0;
   m_handler = 0;
   m_cacheTile = 0;
}

bool ossimGeoidImage::open( const ossimFilename& file, ossimByteOrder /* byteOrder */ )
{
   static const char MODULE[] = "ossimGeoidImage::open";
   
   m_geom = 0;
   m_cacheTile = 0;
   if ( m_handler.valid() )
   {
      m_handler->close();
   }

   m_handler =
      ossimImageHandlerRegistry::instance()->open(file,
                                                  true, // try suffix 1st
                                                  false); // open overview
   if ( m_handler.valid() )
   {
      // Store the bounding rectangle:
      m_imageRect = m_handler->getImageRectangle( 0 );

      // Scalar type:
      m_scalarType = m_handler->getOutputScalarType();
      
      // Get the geometry info.  Need this to compute grid indexes.
      m_geom = m_handler->getImageGeometry();

      if ( m_geom.valid() )
      {
         // Verify the geometry object has a good projection.
         if ( m_geom->getProjection() )
         {
            if ( m_memoryMapFlag )
            {
               try
               {
                  m_cacheTile = m_handler->getTile( m_imageRect, 0 );
                  
                  if ( m_cacheTile.valid() )
                  {
                     // Close the image handler:
                     m_handler->close();
                     m_handler = 0;
                  }
               }
               catch ( const ossimException& e )
               {
                  m_memoryMapFlag = false;
                  m_cacheTile = 0;
                  
                  ossimNotify(ossimNotifyLevel_WARN)
                     << MODULE << " ERROR: Caught Exception!"
                     << "\n" << e.what()
                     << "\nMemory mapping entire grid into memory has been disabled..."
                     << std::endl;
               }
            }
         }
         else
         {
            m_handler = 0;
            m_geom    = 0;
            ossimNotify(ossimNotifyLevel_WARN)
               << MODULE << " ERROR:"
               << "\nGeometry object has null projection!"
               << std::endl;
         }
      }
         
      else
      {
         m_handler = 0;
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " ERROR:"
            << "\nCould not get geometry info!"
            << std::endl;
      }
   }
   
   return ( m_geom.valid() && (m_handler.valid() || (m_memoryMapFlag && m_cacheTile.valid()) ));
}

ossimString ossimGeoidImage::getShortName() const
{
   return m_geoidTypeName;
}

void ossimGeoidImage::setShortName( const std::string& geoidTypeName )
{
   m_geoidTypeName = geoidTypeName;
}

bool ossimGeoidImage::getMemoryMapFlag() const
{
   return m_memoryMapFlag;
}

void ossimGeoidImage::setMemoryMapFlag( bool flag )
{
   m_memoryMapFlag = flag; 
}
   
bool ossimGeoidImage::saveState( ossimKeywordlist& kwl,
                                 const char* prefix ) const
{
   std::string myPrefix = ( prefix ? prefix : "" );

   // Save the connection string:
   std::string key = "connection_string";
   kwl.addPair( myPrefix, key, m_connectionString, true );

   // Save the geoid type name:
   key = "geoid.type";
   kwl.addPair( myPrefix, key, m_geoidTypeName.string(), true );

   // Save the memory map flag:
   key = "memory_map";
   std::string value = (m_memoryMapFlag ? "true" : "false");
   kwl.addPair( myPrefix, key, value, true );

   // Save the enabledFlag:
   key = ossimKeywordNames::ENABLED_KW;
   value = m_enabledFlag ? "true" : "false";
   kwl.addPair( myPrefix, key, value, true );

   // Save the type:
   key = ossimKeywordNames::TYPE_KW;
   value = "geoid_image";
   kwl.addPair( myPrefix, key, value, true );
   
   return true;
}

bool ossimGeoidImage::loadState( const ossimKeywordlist& kwl,
                                 const char* prefix )
{
   bool result = false;
   
   std::string myPrefix = prefix ? prefix : "";
   
   // Check the type name:
   std::string key = ossimKeywordNames::TYPE_KW;
   std::string value = kwl.findKey( myPrefix, key );

   if ( (value == "geoid_image" ) || ( value == "ossimGeoidImage" ) )
   {
      // Get the geoid type name:
      key = "geoid.type";
      m_geoidTypeName = kwl.findKey( myPrefix, key );

      // Get the memory map flag:
      key = "memory_map";
      value = kwl.findKey( myPrefix, key );
      m_memoryMapFlag = ossimString( value ).toBool();

      // Get the enabled flag:
      key = ossimKeywordNames::ENABLED_KW;
      value = kwl.findKey( myPrefix, key );
      m_enabledFlag = ossimString( value ).toBool();
      
      // Get the connection string:
      std::string key = "connection_string";
      m_connectionString = kwl.findKey( myPrefix, key );
      if ( m_connectionString.size() )
      {
         result = open( ossimFilename( m_connectionString ) );
      }
   }
   
   return result;
}


double ossimGeoidImage::offsetFromEllipsoid( const ossimGpt& gpt )
{
   double offset = ossim::nan();

   if ( m_enabledFlag )
   {
      if ( m_geom.valid() &&
           ( m_handler.valid() ||
             ( m_memoryMapFlag && m_cacheTile.valid() )
             )
           )
      {
         switch(m_scalarType)
         {
            case OSSIM_SINT16:
            {
               offset = offsetFromEllipsoidTemplate((ossim_sint16)0, gpt);
               break;
            }
            case OSSIM_FLOAT32:
            {
               offset = offsetFromEllipsoidTemplate((ossim_float32)0, gpt);
               break;
            }
            case OSSIM_FLOAT64:
            {
               offset = offsetFromEllipsoidTemplate((ossim_float64)0, gpt);
               break;
            }
            default:
            {
               ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimGeoidImage::offsetFromEllipsoid ERROR:\n"
                  << "Unhandled scalar type: "
                  << ossimScalarTypeLut::instance()->getEntryString( m_scalarType )
                  << std::endl;
               break;
            }
         }
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimGeoidImage::offsetFromEllipsoid ERROR: Object not initialized!\n"
            << std::endl;
      }
      
   } // Matches: if ( m_enabledFlag )
   
   return offset;
}

template <class T>
double ossimGeoidImage::offsetFromEllipsoidTemplate(T /* dummy */,
                                                    const ossimGpt& gpt)
{
   double geoidOffset = ossim::nan();

   // Change the datum if needed:
   ossimGpt copyGpt = gpt;
   if(ossimDatumFactory::instance()->wgs84())
   {
      copyGpt.changeDatum(ossimDatumFactory::instance()->wgs84());
   }
   
   // Fix wrap conditions:
   copyGpt.wrap();
   
   // Get the local image point for the input ground point.
   ossimDpt imgDpt;
   m_geom->worldToLocal( copyGpt, imgDpt );

   if ( m_imageRect.pointWithin( ossimIpt( imgDpt ) ) )
   {
      ossim_int32 x0 = static_cast<ossim_int32>( imgDpt.x );
      ossim_int32 y0 = static_cast<ossim_int32>( imgDpt.y );
      
      ossim_int32 IW = static_cast<ossim_int32>(m_imageRect.width());
      ossim_int32 IH = static_cast<ossim_int32>(m_imageRect.height());
      
      if ( x0 == (IW-1) )
      {
         --x0;  // Move over one point.
      }
      if ( y0 == (IH-1) )
      {
         --y0; // Move over one point.
      }

      if ( !m_memoryMapFlag )
      {
         // Get the four points from the image handler.
         ossimIrect tileRect(ossimIpt(x0, y0), ossimIpt(x0+1, y0+1));
         m_cacheTile = m_handler->getTile( tileRect, 0 );
      }
      
      if ( m_cacheTile.valid() )
      {
         // Get a pointer to the buffer.
         const T* buf = static_cast<const T*>(m_cacheTile->getBuf());
         
         if ( buf )
         {
            const ossim_float64 NP = m_cacheTile->getNullPix(0);
            const ossim_int64 TW = static_cast<ossim_int64>(m_cacheTile->getWidth());
            ossim_int64 offset = (y0 - m_cacheTile->getOrigin().y) * TW +
               (x0 - m_cacheTile->getOrigin().x);
            ossim_int64 offset2 = offset + TW;
            
            double p00 = buf[offset];
            double p01 = buf[offset+1];
            double p10 = buf[offset2];
            double p11 = buf[offset2+1];
            
            double xt0 = imgDpt.x - x0;
            double yt0 = imgDpt.y - y0;
            double xt1 = 1-xt0;
            double yt1 = 1-yt0;
            
            double w00 = xt1*yt1;
            double w01 = xt0*yt1;
            double w10 = xt1*yt0;
            double w11 = xt0*yt0;
            
            //---
            // Test for null posts and set the corresponding weights to 0:
            //---
            if (p00 == NP) w00 = 0.0;
            if (p01 == NP) w01 = 0.0;
            if (p10 == NP) w10 = 0.0;
            if (p11 == NP) w11 = 0.0;                  
            
            double sum_weights = w00 + w01 + w10 + w11;
            if (sum_weights)
            {
               geoidOffset = (p00*w00 + p01*w01 + p10*w10 + p11*w11) / sum_weights;
            }
         }
         
      } // Matches: if ( m_cacheTile.valid() )
      
   } // Matches: if ( m_imageRect.pointWithin( imgPt ) )

   return geoidOffset;
}

