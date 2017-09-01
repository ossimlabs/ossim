//----------------------------------------------------------------------------
//
// File: ossimImageElevationHandler.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  See class desciption in header file.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/elevation/ossimImageElevationHandler.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <OpenThreads/ScopedLock>

#include <iostream> // tmp drb
using namespace std;

#include <cmath>

RTTI_DEF1(ossimImageElevationHandler, "ossimImageElevationHandler" , ossimElevCellHandler)

// Define Trace flags for use within this file:
static ossimTrace traceDebug ("ossimImageElevationHandler:debug");
static const ossim_uint32 MAX_TILE_CACHE_SIZE = 32;

ossimImageElevationHandler::ossimImageElevationHandler()
   :
   ossimElevCellHandler(),
   m_rect(),
   m_tileSize(256,256),
   m_numTilesPerRow(0),
   m_mutex()
{
   ossim::defaultTileSize(m_tileSize);
}

ossimImageElevationHandler::ossimImageElevationHandler(const ossimFilename& file)
   :
   ossimElevCellHandler(),
   m_rect(),
   m_tileSize(256,256),
   m_numTilesPerRow(0),
   m_mutex()
{
   ossim::defaultTileSize(m_tileSize);
   open(file);
}

bool ossimImageElevationHandler::open(const ossimFilename& file)
{
   static const char M[] = "ossimImageElevationHandler::open";
   
   bool result = false;

   if ( isOpen() ) close();
   
   if ( file.size() )
   {
      //---
      // NOTE: The false passed to open is flag to NOT open overviews. If code is ever changed
      // to go between reduced resolution levels this should be changed.
      //---
      m_ih = ossimImageHandlerRegistry::instance()->open(file, true, false);
      if ( m_ih.valid() )
      {
         m_geom = m_ih->getImageGeometry();
         if ( m_geom.valid() )
         {
            result = true;

            // Image rect stored as a drect for ossimImageElevationHandler::pointHasCoverage
            // method.
            m_rect = ossimDrect(0.0, 0.0, m_ih->getNumberOfSamples()-1, m_ih->getNumberOfLines()-1);
            m_numTilesPerRow = m_rect.width()/m_tileSize.x;
            if (m_numTilesPerRow * m_tileSize.x != m_rect.width())
               ++m_numTilesPerRow;

            // Initialize base class stuff.
            theFilename = file;

            theMeanSpacing = (m_geom->getMetersPerPixel().x + m_geom->getMetersPerPixel().y) / 2.0;

            // Set the ossimElevSource::theGroundRect
            std::vector<ossimGpt> corner(4);
            if ( m_geom->getCornerGpts(corner[0], corner[1], corner[2], corner[3]) )
            {
               ossimGpt ulGpt(corner[0]);
               ossimGpt lrGpt(corner[0]);
               for ( ossim_uint32 i = 1; i < 4; ++i )
               {
                  if ( corner[i].lon < ulGpt.lon ) ulGpt.lon = corner[i].lon;
                  if ( corner[i].lat > ulGpt.lat ) ulGpt.lat = corner[i].lat;
                  if ( corner[i].lon > lrGpt.lon ) lrGpt.lon = corner[i].lon;
                  if ( corner[i].lat < lrGpt.lat ) lrGpt.lat = corner[i].lat;
               }
               theGroundRect = ossimGrect(ulGpt, lrGpt);
            }
            else
            {
               theGroundRect.makeNan();
            }
         }
         // theAbsLE90 ???
         // theAbsCE90 ???
      }
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " DEBUG:"
         << "\nmean spacing(meters): " << theMeanSpacing
         << "\nunding rect: " << theGroundRect
         << "\nreturn status: " << (result?"true\n":"false\n");
   }
         
   return result;
}


ossimImageElevationHandler::~ossimImageElevationHandler()
{
   close();
}

double ossimImageElevationHandler::getHeightAboveMSL(const ossimGpt& gpt)
{
   double height = ossim::nan();

   // Get the image point for the world point.
   ossimDpt dpt;
   m_geom->worldToLocal(gpt, dpt);

   if  ( !m_rect.pointWithin(dpt) )
      return height;

   // Cast it to an int which will shift to nearest upper left post.
   ossim_uint32 x0 = static_cast<ossim_uint32>(dpt.x);
   ossim_uint32 y0 = static_cast<ossim_uint32>(dpt.y);

   ossimRefPtr<ossimImageData> data = getTile(x0, y0);

   // Check if we are at the bottom or right edge of the tile to move in one post:
   ossimIpt data_lr = data->getImageRectangle().lr();
   if (x0 == static_cast<ossim_uint32>(data_lr.x))
      --x0;
   if (y0 == static_cast<ossim_uint32>(data_lr.y))
      --y0;

   double p00 = data->getPix( ossimIpt(x0,   y0)   );
   double p01 = data->getPix( ossimIpt(x0+1, y0)   );
   double p10 = data->getPix( ossimIpt(x0,   y0+1) );
   double p11 = data->getPix( ossimIpt(x0+1, y0+1) );

   double xt0 = dpt.x - x0;
   double yt0 = dpt.y - y0;
   double xt1 = 1-xt0;
   double yt1 = 1-yt0;

   double w00 = xt1*yt1;
   double w01 = xt0*yt1;
   double w10 = xt1*yt0;
   double w11 = xt0*yt0;

   // Test for null posts and set the corresponding weights to 0:
   const double NP = data->getNullPix(0);

   if (p00 == NP)
      w00 = 0.0;
   if (p01 == NP)
      w01 = 0.0;
   if (p10 == NP)
      w10 = 0.0;
   if (p11 == NP)
      w11 = 0.0;

#if 0 /* Serious debug only... */
   cout << "\np00:  " << p00
         << "\np01:  " << p01
         << "\np10:  " << p10
         << "\np11:  " << p11
         << "\nw00:  " << w00
         << "\nw01:  " << w01
         << "\nw10:  " << w10
         << "\nw11:  " << w11
         << endl;
#endif

   double sum_weights = w00 + w01 + w10 + w11;
   if (sum_weights)
      height = (p00*w00 + p01*w01 + p10*w10 + p11*w11) / sum_weights;

   return height;
}

ossimImageData* ossimImageElevationHandler::getTile(ossim_uint32 x, ossim_uint32 y) const
{
   // Establish the tile ID that this post belongs to:
   ossim_uint32 u = x/m_tileSize.x;
   ossim_uint32 v = y/m_tileSize.y;
   ossim_uint32 tile_id = v*m_numTilesPerRow + u;

   ossimRefPtr<ossimImageData> data = 0;

   // Search for this tile in the cache:
   OpenThreads::ScopedWriteLock lock (m_mutex);
   vector<TileCacheEntry>::iterator iter = m_tileCache.begin();
   while ((iter != m_tileCache.end()) && (iter->id != tile_id))
      ++iter;

   if (iter != m_tileCache.end())
   {
      // Found an entry in the cache:
      data = iter->data;
      if (iter != m_tileCache.begin())
      {
         // Need to move the entry to the top of the list (current most popular)
         TileCacheEntry temp = *iter;
         m_tileCache.erase(iter);
         m_tileCache.insert(m_tileCache.begin(), temp);
      }
   }
   else
   {
      // Didn't find the tile in the cache, read it from the handler and insert into the cache:
      ossimIpt ul (u*m_tileSize.x, v*m_tileSize.y);
      ossimIpt lr (ul.x + m_tileSize.x - 1, ul.y + m_tileSize.y - 1);
      ossimIrect tileRect (ul, lr);
      ossimRefPtr<ossimImageData> inData = m_ih->getTile(tileRect, 0 );
      if (inData.valid())
      {
         // Always insert at beginning. Need to check for overflow:
         data = (ossimImageData*) inData->dup();
         data->assign(inData.get());
         TileCacheEntry entry (tile_id, data.get());
         if (m_tileCache.size() == MAX_TILE_CACHE_SIZE)
            m_tileCache.pop_back();
         m_tileCache.insert(m_tileCache.begin(), entry);
      }
   }

   return data.get();
}

ossimIpt ossimImageElevationHandler::getSizeOfElevCell() const
{
   ossimIpt sz(0,0);
   if ( m_ih.valid() )
   {
      sz.x = static_cast<ossim_int32>(m_ih->getNumberOfSamples(0));
      sz.y = static_cast<ossim_int32>(m_ih->getNumberOfLines(0));
   }
   return sz;
}

double ossimImageElevationHandler::getPostValue(const ossimIpt& gridPt ) const
{
   double height = ossim::nan();
   if ( m_rect.pointWithin(ossimDpt(gridPt)) )
   {
      ossimRefPtr<ossimImageData> data = getTile(gridPt.x, gridPt.y);
      if ( data.valid() )
      {
         height = data->getPix(0, 0);
         if (height == data->getNullPix(0))
            height = ossim::nan();
      }
   }
   return height;
}

ossimImageElevationHandler::ossimImageElevationHandler(const ossimImageElevationHandler& copy)
{
   *this = copy;
}

const ossimImageElevationHandler&
ossimImageElevationHandler::operator=(const ossimImageElevationHandler& rhs)
{
   open(rhs.theFilename);
   return *this;
}
