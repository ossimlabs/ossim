//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM HDF5 Reader.
//
//----------------------------------------------------------------------------
// $Id: ossimHdf5ImageHandler.cpp 19878 2011-07-28 16:27:26Z dburken $

//#include "ossimH5GridModel.h"

#include <ossim/hdf5/ossimHdf5ImageHandler.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageGeometryRegistry.h>
#include <ossim/imaging/ossimTiffTileSource.h>
#include <ossim/imaging/ossimU8ImageData.h>
#include <ossim/projection/ossimBilinearProjection.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/hdf5/ossimHdf5GridModel.h>
static const ossimTrace traceDebug("ossimHdf5ImageHandler:debug");

RTTI_DEF1(ossimHdf5ImageHandler, "ossimHdf5ImageHandler", ossimImageHandler)

using namespace std;
using namespace H5;

static const string LAYER_KW = "layer";

ossimHdf5ImageHandler::ossimHdf5ImageHandler()
:  ossimImageHandler(),
   m_entries(),
   m_currentEntry(0),
   m_tile(0),
   m_mutex()
{
}

ossimHdf5ImageHandler::~ossimHdf5ImageHandler()
{
   if (isOpen())
   {
      close();
   }
}

void ossimHdf5ImageHandler::allocate()
{
   m_mutex.lock();
   m_tile = ossimImageDataFactory::instance()->create(this, this);
   m_tile->initialize();
   m_mutex.unlock();
}

ossimRefPtr<ossimImageData> ossimHdf5ImageHandler::getTile(const ossimIrect& rect,
                                                           ossim_uint32 resLevel)
{
   if ( m_tile.valid() == false ) // First time through.
   {
      allocate();
   }

   if (m_tile.valid())
   {
      // Image rectangle must be set prior to calling getTile.
      m_mutex.lock();
      m_tile->setImageRectangle(rect);
      m_mutex.unlock();

      if ( getTile( m_tile.get(), resLevel ) == false )
      {
         m_mutex.lock();
         if (m_tile->getDataObjectStatus() != OSSIM_NULL)
         {
            m_tile->makeBlank();
         }
         m_mutex.unlock();
      }
   }

   return m_tile;
}

bool ossimHdf5ImageHandler::getTile(ossimImageData* result, ossim_uint32 resLevel)
{
   bool status = false;

   m_mutex.lock();

   //---
   // Not open, this tile source bypassed, or invalid res level,
   // return a blank tile.
   //---
   if( isOpen() && isSourceEnabled() && isValidRLevel(resLevel) &&
         result && (result->getNumberOfBands() == getNumberOfOutputBands()) )
   {
      result->ref(); // Increment ref count.

      //---
      // Check for overview tile.  Some overviews can contain r0 so always
      // call even if resLevel is 0.  Method returns true on success, false
      // on error.
      //---
      status = getOverviewTile(resLevel, result);

      if (!status) // Did not get an overview tile.
      {
         status = true;

         ossimIrect tile_rect = result->getImageRectangle();

         if ( ! tile_rect.completely_within(getImageRectangle(0)) )
         {
            // We won't fill totally so make blank first.
            result->makeBlank();
         }

         if (getImageRectangle(0).intersects(tile_rect))
         {
            // Make a clip rect.
            ossimIrect clipRect = tile_rect.clipToRect(getImageRectangle(0));

            if (tile_rect.completely_within( clipRect) == false)
            {
               // Not filling whole tile so blank it out first.
               result->makeBlank();
            }

            // Create buffer to hold the clip rect for a single band.
            ossim_uint32 clipRectSizeInBytes = clipRect.area() *
                  ossim::scalarSizeInBytes( m_entries[m_currentEntry]->getScalarType() );
            vector<char> dataBuffer(clipRectSizeInBytes);

            // Get the data.
            for (ossim_uint32 band = 0; band < getNumberOfInputBands(); ++band)
            {
               // Hdf5 file to buffer:
               m_entries[m_currentEntry]->getTileBuf(&dataBuffer.front(), clipRect, band);
#if 0
               // Scan and fix non-standard null value:
               if ( m_entries[m_currentEntry]->getScalarType() == OSSIM_FLOAT32 )
               {
                  const ossim_float32 NP = getNullPixelValue(band);
                  const ossim_uint32 COUNT = clipRect.area();
                  ossim_float32* float_buffer = (ossim_float32*)&dataBuffer.front();
                  for ( ossim_uint32 i = 0; i < COUNT; ++i )
                  {
                     if ( float_buffer[i] < -999.0 )
                        float_buffer[i] = NP;
                  }
               }
#endif
               // Buffer to tile:
               result->loadBand((void*)&dataBuffer.front(), clipRect, band);
            }

            // Validate the tile, i.e. full, partial, empty.
            result->validate();
         }
         else // No intersection...
         {
            result->makeBlank();
         }
      }

      result->unref();  // Decrement ref count.
   }

   m_mutex.unlock();

   return status;
}

ossimIrect
ossimHdf5ImageHandler::getImageRectangle(ossim_uint32 reduced_res_level) const
{
   return ossimIrect(0,
                     0,
                     getNumberOfSamples(reduced_res_level) - 1,
                     getNumberOfLines(reduced_res_level)   - 1);
}

bool ossimHdf5ImageHandler::saveState(ossimKeywordlist& kwl,
                                      const char* prefix) const
{
   return ossimImageHandler::saveState(kwl, prefix);
}

bool ossimHdf5ImageHandler::loadState(const ossimKeywordlist& kwl,
                                      const char* prefix)
{
   if (ossimImageHandler::loadState(kwl, prefix))
   {
      return open();
   }

   return false;
}

void ossimHdf5ImageHandler::loadMetaData()
{
   m_tile = 0;
   ossimImageHandler::loadMetaData();
}

bool ossimHdf5ImageHandler::open()
{
   static const char* M = "ossimHdf5ImageHandler::open(filename) -- ";
   if(traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << M <<" Entering..." << std::endl;

   // Start with a clean slate.
   if (isOpen())
   {
      close();
   }

   // Check for empty filename.
   if (theImageFile.empty())
      return false;

   theImageFile = theImageFile.expand();
   m_hdf5 = new ossimHdf5;
   if (!m_hdf5->open(theImageFile))
   {
      if(traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << M <<" Unable to open image Leaving..." << std::endl;
      m_hdf5 = 0;
      return false;
   }

   if(traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << M <<" Opened Image..." << std::endl;

   vector<H5::DataSet> datasetList;
   H5::Group root;
   m_hdf5->getRoot(root);
   m_hdf5->getNdimDatasets(root, datasetList, true);

   if ( datasetList.empty() )
   {
      return false;
      m_hdf5->close();
      m_hdf5 = 0;
   }
   // Filter for specified renderable datasets:
   if (m_renderableNames.size())
   {
      std::vector<H5::DataSet>::iterator dataset = datasetList.begin();
      while (dataset != datasetList.end())
      {
         bool found=false;
         ossimString dsName = dataset->getObjName();
         std::vector<ossimString>::iterator name = m_renderableNames.begin();
         while (name != m_renderableNames.end())
         {
            if (dsName.contains(*name))
            {
               found = true;
               break;
            }
            ++name;
         }
         if (!found)
            datasetList.erase(dataset);
         else
            ++dataset;
      }
   }
#if 0
   ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimHdf5ImageHandler:"<<__LINE__ << " DEBUG\nDataset names:\n";
   for ( ossim_uint32 i = 0; i < datasetList.size(); ++i )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "dataset[" << i << "]: "
            << datasetList[i].getObjName() << "\n";
   }
#endif
   if ( datasetList.empty() )
   {
      m_hdf5 = 0;
      return false;
   }

   // Add the image dataset entries.
   std::vector<H5::DataSet>::iterator dataset = datasetList.begin();
   while (dataset != datasetList.end())
   {
      ossimRefPtr<ossimHdf5ImageDataset> oimgset = new ossimHdf5ImageDataset(this);
      oimgset->initialize(*dataset);
      m_entries.push_back(oimgset);
      ++dataset;
   }

   // Initialize the current entry to be 0:
   m_currentEntry = 999; // Forces init on change of index
   setCurrentEntry(0);

   // Establish a common geometry for all entries. TODO: Need to verify if this is a kosher thing
   // to do. It may be that multiple entries have different geometries associated.
   getImageGeometry();
   if (!theGeometry.valid())
      return false;

#if 0 /* Please leave for debug. (drb) */
   std::vector<ossimHdf5ImageDataset>::const_iterator i = m_entries.begin();
   while ( i != m_entries.end() )
   {
      std::cout << (*i) << endl;
      ++i;
   }
#endif
   if(traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << M <<" Leaving..." << std::endl;

   return true;
}


ossim_uint32 ossimHdf5ImageHandler::getNumberOfLines(ossim_uint32 reduced_res_level) const
{
   ossim_uint32 r = 0;
   if (reduced_res_level == 0)
   {
      if (m_currentEntry < m_entries.size())
         r = m_entries[m_currentEntry]->getNumberOfLines();
   }
   else if ( theOverview.valid() )
   {
      r = theOverview->getNumberOfLines(reduced_res_level);
   }

   return r;
}

ossim_uint32 ossimHdf5ImageHandler::getNumberOfSamples(ossim_uint32 reduced_res_level) const
{
   ossim_uint32 r = 0;
   if (reduced_res_level == 0)
   {
      if (m_currentEntry < m_entries.size())
         r = m_entries[m_currentEntry]->getNumberOfSamples();
   }
   else if ( theOverview.valid() )
   {
      r = theOverview->getNumberOfSamples(reduced_res_level);
   }

   return r;
}

ossim_uint32 ossimHdf5ImageHandler::getImageTileWidth() const
{
   return 0; // Not tiled format.
}

ossim_uint32 ossimHdf5ImageHandler::getImageTileHeight() const
{
   return 0; // Not tiled format.
}

ossimString ossimHdf5ImageHandler::getShortName()const
{
   return ossimString("ossim_hdf5_reader");
}

ossimString ossimHdf5ImageHandler::getLongName()const
{
   return ossimString("ossim hdf5 reader");
}

ossimString  ossimHdf5ImageHandler::getClassName()const
{
   return ossimString("ossimHdf5ImageHandler");
}

ossim_uint32 ossimHdf5ImageHandler::getNumberOfInputBands() const
{
   ossim_uint32 result = 1;

   if ( m_currentEntry < m_entries.size() )
      result = m_entries[m_currentEntry]->getNumberOfBands();

   return result;
}

ossim_uint32 ossimHdf5ImageHandler::getNumberOfOutputBands()const
{
   // Currently not band selectable.
   return getNumberOfInputBands();
}

ossimScalarType ossimHdf5ImageHandler::getOutputScalarType() const
{
   ossimScalarType result = OSSIM_SCALAR_UNKNOWN;

   if ( m_currentEntry < m_entries.size() )
   {
      result = m_entries[m_currentEntry]->getScalarType();
   }

   return result;
}

bool ossimHdf5ImageHandler::isOpen()const
{
   return ( m_hdf5.valid() &&  m_entries.size() && (m_currentEntry < m_entries.size()));
}

void ossimHdf5ImageHandler::close()
{
   // Close the datasets.
   m_entries.clear();

   // Then the file.
   if ( m_hdf5.valid() )
      m_hdf5 = 0;

   // ossimRefPtr so assign to 0(unreferencing) will handle memory.
   m_tile = 0;

   ossimImageHandler::close();
}

ossim_uint32 ossimHdf5ImageHandler::getNumberOfEntries() const
{
   return (ossim_uint32)m_entries.size();
}

void ossimHdf5ImageHandler::getEntryNames(std::vector<ossimString>& entryNames) const
{
   entryNames.clear();
   for (ossim_uint32 i=0; i<m_entries.size(); ++i )
   {
      entryNames.push_back(m_entries[i]->getName());
   }
}

void ossimHdf5ImageHandler::getEntryList(std::vector<ossim_uint32>& entryList) const
{
   const ossim_uint32 SIZE = m_entries.size();
   entryList.resize( SIZE );
   for ( ossim_uint32 i = 0; i < SIZE; ++i )
   {
      entryList[i] = i; 
   }
}

bool ossimHdf5ImageHandler::setCurrentEntry( ossim_uint32 entryIdx)
{
   bool result = true;
   if (m_currentEntry != entryIdx)
   {
      // Entries always start at zero and increment sequentially..
      if ( entryIdx < m_entries.size() )
      {
         theOverviewFile.clear();
         m_currentEntry = entryIdx;
         m_tile = 0;
         ossimIrect validRect = m_entries[entryIdx]->getValidImageRect();
         theValidImageVertices.clear();
         theValidImageVertices.push_back(validRect.ul());
         theValidImageVertices.push_back(validRect.ur());
         theValidImageVertices.push_back(validRect.lr());
         theValidImageVertices.push_back(validRect.ll());

         if ( isOpen() )
            completeOpen();
      }
      else
      {
         result = false; // Entry index out of range.
      }
   }

   return result;
}

ossim_uint32 ossimHdf5ImageHandler::getCurrentEntry() const
{
   return m_currentEntry;
}

ossimRefPtr<ossimHdf5ImageDataset> ossimHdf5ImageHandler::getCurrentDataset()
{
   if ( m_currentEntry >= m_entries.size() )
      return 0;

   return m_entries[m_currentEntry];
}

double ossimHdf5ImageHandler::getNullPixelValue( ossim_uint32 band ) const
{
   return ossimImageHandler::getNullPixelValue( band );
}

double ossimHdf5ImageHandler::getMaxPixelValue( ossim_uint32 band ) const
{
   if ( m_currentEntry >= m_entries.size() )
   {
      if(m_entries[m_currentEntry]->isMaxPixelSet())
      {
         return m_entries[m_currentEntry]->getMaxPixelValue(band);
      }
   }
   return ossimImageHandler::getMaxPixelValue( band );
}

double ossimHdf5ImageHandler::getMinPixelValue( ossim_uint32 band ) const
{
   if ( m_currentEntry >= m_entries.size() )
   {
      if(m_entries[m_currentEntry]->isMinPixelSet())
      {
         return m_entries[m_currentEntry]->getMinPixelValue(band);
      }
   }
   return ossimImageHandler::getMinPixelValue( band );
}

void ossimHdf5ImageHandler::setProperty(ossimRefPtr<ossimProperty> property)
{
   if ( !property.valid() )
      return;

   if ( property->getName().string() == LAYER_KW )
   {
      ossimString s;
      property->valueToString(s);
      ossim_uint32 SIZE = (ossim_uint32)m_entries.size();
      for ( ossim_uint32 i = 0; i < SIZE; ++i )
      {
         if ( m_entries[i]->getName() == s.string() )
         {
            setCurrentEntry( i );
         }
      }
   }
   else
   {
      ossimImageHandler::setProperty(property);
   }
}

ossimRefPtr<ossimProperty> ossimHdf5ImageHandler::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> prop = 0;
   if ( name.string() == LAYER_KW )
   {
      if ( m_currentEntry < m_entries.size() )
      {
         ossimString value = m_entries[m_currentEntry]->getName();
         prop = new ossimStringProperty(name, value);
      }
   }
   else
   {
      prop = ossimImageHandler::getProperty(name);
   }
   return prop;
}

void ossimHdf5ImageHandler::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   propertyNames.push_back( ossimString("layer") );
   ossimImageHandler::getPropertyNames(propertyNames);
}

