//*******************************************************************
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: ossimImageHistogramSource.cpp 22737 2014-04-16 18:53:57Z gpotts $

#include <ossim/imaging/ossimImageHistogramSource.h>
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <ossim/base/ossimMultiBandHistogram.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>

using namespace std;

static ossimTrace traceDebug("ossimImageHistogramSource:debug");

RTTI_DEF3(ossimImageHistogramSource, "ossimImageHistogramSource", ossimHistogramSource, ossimConnectableObjectListener, ossimProcessInterface);

ossimImageHistogramSource::ossimImageHistogramSource(ossimObject* owner)
   :ossimHistogramSource(owner,
                         1,     // one input
                         0,     // no outputs
                         true,  // input list is fixed
                         false),// output can still grow though
    theHistogramRecomputeFlag(true),
    theMaxNumberOfResLevels(1),
    theComputationMode(OSSIM_HISTO_MODE_NORMAL)
    // theNumberOfTilesToUseInFastMode(100)
{
   theAreaOfInterest.makeNan();
   addListener((ossimConnectableObjectListener*)this);
	
   theMinValueOverride     = ossim::nan();
   theMaxValueOverride     = ossim::nan();
   theNumberOfBinsOverride = -1;
}

ossimImageHistogramSource::~ossimImageHistogramSource()
{
   removeListener((ossimConnectableObjectListener*)this);
}

ossimObject* ossimImageHistogramSource::getObject()
{
   return this;
}

const ossimObject* ossimImageHistogramSource::getObject()const
{
   return this;
}

void ossimImageHistogramSource::setAreaOfInterest(const ossimIrect& rect)
{
   if(rect != theAreaOfInterest)
   {
      theHistogramRecomputeFlag = true;
   }
   theAreaOfInterest = rect;
}

ossimIrect ossimImageHistogramSource::getAreaOfInterest()const
{
   return theAreaOfInterest;
}

void ossimImageHistogramSource::getAreaOfInterest(ossimIrect& rect)const
{
   rect = theAreaOfInterest;
}

ossim_uint32 ossimImageHistogramSource::getMaxNumberOfRLevels()const
{
   return theMaxNumberOfResLevels;
}

void ossimImageHistogramSource::setMaxNumberOfRLevels(ossim_uint32 number)
{
   if(number != theMaxNumberOfResLevels)
   {
      theHistogramRecomputeFlag = true;
   }
   theMaxNumberOfResLevels = number;
}

ossimRefPtr<ossimMultiResLevelHistogram> ossimImageHistogramSource::getHistogram(const ossimIrect& rect)
{
   if((theAreaOfInterest != rect)||
      (theAreaOfInterest.hasNans()))
   {
      theAreaOfInterest = rect;
      theHistogramRecomputeFlag = true;
   }
	
   return getHistogram();
}

bool ossimImageHistogramSource::execute()
{
   if(!isSourceEnabled())
   {
      return theHistogram.valid();
   }
   
   setProcessStatus(ossimProcessInterface::PROCESS_STATUS_EXECUTING);
   if(theHistogramRecomputeFlag)
   {
      if(theAreaOfInterest.hasNans())
      {
         ossimImageSource* interface = PTR_CAST(ossimImageSource, getInput(0));
         if(interface)
         {
            theAreaOfInterest = interface->getBoundingRect();
         }
      }

      switch(theComputationMode)
      {
         case OSSIM_HISTO_MODE_FAST:
         {
            computeFastModeHistogram();
            break;
         }
         case OSSIM_HISTO_MODE_NORMAL:
         default:
         {
            computeNormalModeHistogram();
            break;
         }
      }
   }
   
   if (needsAborting())
   {
      setProcessStatus(ossimProcessInterface::PROCESS_STATUS_ABORTED);
      theHistogramRecomputeFlag = false;
   }
   else
   {
      setProcessStatus(ossimProcessInterface::PROCESS_STATUS_NOT_EXECUTING);
   }
   
   return true;
}

bool ossimImageHistogramSource::canConnectMyInputTo(ossim_int32 myInputIndex,
                                                    const ossimConnectableObject* object)const
{
   return ((myInputIndex==0)&&PTR_CAST(ossimImageSource,
                                       object));
}

void ossimImageHistogramSource::setNumberOfBinsOverride(ossim_int32 numberOfBinsOverride)
{
   theNumberOfBinsOverride = numberOfBinsOverride;
}

void ossimImageHistogramSource::setMinValueOverride(ossim_float32 minValueOverride)
{
   theMinValueOverride = minValueOverride;
}
void ossimImageHistogramSource::setMaxValueOverride(ossim_float32 maxValueOverride)
{
   theMaxValueOverride = maxValueOverride;
}

ossimHistogramMode ossimImageHistogramSource::getComputationMode()const
{
   return theComputationMode;
}

void ossimImageHistogramSource::setComputationMode(ossimHistogramMode mode)
{
   theComputationMode = mode;
}

void ossimImageHistogramSource::propertyEvent(ossimPropertyEvent& /* event */)
{
   theHistogramRecomputeFlag = true;
}

void ossimImageHistogramSource::connectInputEvent(ossimConnectionEvent& /* event */)
{
   theHistogramRecomputeFlag = true;
}

ossimRefPtr<ossimMultiResLevelHistogram> ossimImageHistogramSource::getHistogram()
{
   execute();
   return theHistogram;
}

bool ossimImageHistogramSource::getBinInformation(ossim_uint32& numberOfBins,
                                                  ossim_float32& minValue,
                                                  ossim_float32& maxValue,
                                                  ossim_float32& nullValue,
                                                  ossim_uint32 band)const
{
   bool result = false;
   ossimImageSource* input = PTR_CAST(ossimImageSource, getInput(0));
   if(input)
   {
      result = ossim::getBinInformation( input, band, numberOfBins,
                                         minValue, maxValue, nullValue );
      if ( result )
      {
         if(ossim::isnan(theMinValueOverride) == false)
         {
            minValue = (float)theMinValueOverride;
         }
         if(ossim::isnan(theMaxValueOverride) == false)
         {
            maxValue = (float)theMaxValueOverride;
         }
         if(theNumberOfBinsOverride > 0)
         {
            numberOfBins = theNumberOfBinsOverride;
         }
      }
   }
   return result;
}

void ossimImageHistogramSource::computeNormalModeHistogram()
{
   // ref ptr, not a leak.
   theHistogram = new ossimMultiResLevelHistogram;
   ossimImageSource *input = PTR_CAST(ossimImageSource, getInput(0));
   if ( input )
   {
      // sum up all tiles needing processing.  We will use the sequencer.
      //      ossim_uint32 numberOfResLevels = input->getNumberOfDecimationLevels();
      ossim_uint32 index = 0;
      double tileCount   = 0.0;
      double totalTiles  = 0.0;
      ossim_uint32 numberOfBands = input->getNumberOfOutputBands();
      ossim_uint32 numberOfBins  = 0;
      ossim_float32 minValue     = 0;
      ossim_float32 maxValue     = 0;
      ossim_float32 nullValue    = 0;
      if ( getBinInformation(numberOfBins, minValue, maxValue, nullValue, 0) )
      {
         ossimRefPtr<ossimImageSourceSequencer> sequencer = new ossimImageSourceSequencer();
         
         // If the input is tiled use that tile size:
         ossimIpt inputTileSize;
         inputTileSize.x = input->getTileWidth();
         inputTileSize.y = input->getTileHeight();
         if ( (inputTileSize.x != 0) && (inputTileSize.y != 0) )
         {
            sequencer->setTileSize( inputTileSize );
         }
         
         sequencer->connectMyInputTo(0, getInput(0));
         sequencer->initialize();

         vector<ossimDpt> decimationFactors;
         input->getDecimationFactors(decimationFactors);
         if ( !decimationFactors.size() )
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "ossimImageHistogramSource::computeNormalModeHistogram WARNING:"
               << "\nNo decimation factors from input.  returning..." << std::endl;
            return;
         }

         ossim_uint32 resLevelsToCompute = ossim::min((ossim_uint32)theMaxNumberOfResLevels,
                                                      (ossim_uint32)decimationFactors.size());

         if( decimationFactors.size() < theMaxNumberOfResLevels)
         {
            ossimNotify(ossimNotifyLevel_WARN) << "Number Decimations is smaller than the request number of r-levels defaulting to the smallest of the 2 numbers" << endl;
         }

         theHistogram->create(resLevelsToCompute);
         for(index = 0; index < resLevelsToCompute; ++index)
         {
            sequencer->setAreaOfInterest(theAreaOfInterest*decimationFactors[index]);
            
            totalTiles += sequencer->getNumberOfTiles();
         }

         if(numberOfBins > 0)
         {
            setPercentComplete(0.0);
            for(index = 0; (index < resLevelsToCompute); ++index)
            {
               // Check for abort request.
               if (needsAborting())
               {
                  setPercentComplete(100);
                  break;
               }
               
               //sequencer->setAreaOfInterest(input->getBoundingRect(index));
               sequencer->setAreaOfInterest(theAreaOfInterest*decimationFactors[index]);

               sequencer->setToStartOfSequence();

               theHistogram->getMultiBandHistogram(index)->create(
                  numberOfBands, numberOfBins, minValue, maxValue, nullValue,
                  input->getOutputScalarType() );
               
               ossimRefPtr<ossimImageData> data = sequencer->getNextTile(index);
               ++tileCount;
               setPercentComplete((100.0*(tileCount/totalTiles)));

               ossim_uint32 resLevelTotalTiles = sequencer->getNumberOfTiles();
               for (ossim_uint32 resLevelTileCount = 0;
                    resLevelTileCount < resLevelTotalTiles;
                    ++resLevelTileCount)
               {
                  //---
                  // Counting nulls now so the check for OSSIM_EMPTY status
                  // removed. drb - 20190227
                  //---
                  if( data.valid() )
                  {
                     data->populateHistogram(
                        theHistogram->getMultiBandHistogram(index), theAreaOfInterest );
                  }

                  // Check for abort request.
                  if (needsAborting())
                  {
                     setPercentComplete(100);
                     break;
                  }

                  data = sequencer->getNextTile(index);
                  ++tileCount;
                  setPercentComplete((100.0*(tileCount/totalTiles)));
               }
            }
         }
         sequencer->disconnect();
         sequencer = 0;
      }
   }
   else
   {
      setPercentComplete(100.0);
   }
}

void ossimImageHistogramSource::computeFastModeHistogram()
{
   // Compute at most 9 x 9 tiles of 16 x 16 tile size. 

   ossim_uint32 resLevelsToCompute = 1;
	
   // ref ptr, not a leak.
   theHistogram = new ossimMultiResLevelHistogram;
   theHistogram->create(resLevelsToCompute);
   ossimImageSource* input = PTR_CAST(ossimImageSource, getInput(0));
   if(!input)
   {
      setPercentComplete(100.0);
      return;
   }
   // sum up all tiles needing processing.  We will use the sequencer.
   //      ossim_uint32 numberOfResLevels = input->getNumberOfDecimationLevels();
   double tileCount   = 0.0;
   double totalTiles  = 0.0;
   ossim_uint32 numberOfBands = input->getNumberOfOutputBands();
   ossim_uint32 numberOfBins  = 0;
   ossim_float32 minValue     = 0;
   ossim_float32 maxValue     = 0;
   ossim_float32 nullValue    = 0;

   // Assuming all bands have the same min, max, null as band 0:
   if ( getBinInformation(numberOfBins, minValue, maxValue, nullValue, 0) )
   {
      // Fixed 32 x 32 tile size:
      ossimIpt tileSize( 32, 32 );

      ossimIrect tileBoundary = theAreaOfInterest;
      tileBoundary.stretchToTileBoundary(tileSize);

      // Max of 11 x 11 tiles accross the image.
      const ossim_uint32 MAX_TILES_WIDE = 11;

      ossim_uint32 tilesWide = ossim::min( (ossim_uint32)(tileBoundary.width()/tileSize.x),
                                           MAX_TILES_WIDE);
      ossim_uint32 tilesHigh = ossim::min( (ossim_uint32)(tileBoundary.height()/tileSize.y),
                                           MAX_TILES_WIDE);
      totalTiles = tilesWide*tilesHigh;

      if(numberOfBins > 0)
      {
         ossimIpt origin = theAreaOfInterest.ul();
         
         ossim_uint32 xTileOffset = tileBoundary.width()  / tilesWide;
         ossim_uint32 yTileOffset = tileBoundary.height() / tilesHigh;
         
         theHistogram->getMultiBandHistogram(0)->create(
            numberOfBands, numberOfBins, minValue, maxValue, nullValue,
            input->getOutputScalarType() );
         ossim_uint32 x = 0;
         ossim_uint32 y = 0;
         tileCount = 0;
         totalTiles = tilesWide*tilesHigh;
         
         for(y = 0; y < tilesHigh; ++y)
         {
            for(x = 0; x < tilesWide; ++x)
            {
               ossimIpt ul( origin.x + (x*xTileOffset), origin.y + (y*yTileOffset) );
               ossimIrect tileRect(ul.x, ul.y, ul.x + tileSize.x-1, ul.y + tileSize.y-1);

               ossimRefPtr<ossimImageData> data = input->getTile(tileRect);
               if(data.valid()&&data->getBuf()&&(data->getDataObjectStatus() != OSSIM_EMPTY))
               {
                  data->populateHistogram(
                     theHistogram->getMultiBandHistogram(0), theAreaOfInterest );
               }

               // Check for abort request.
               if (needsAborting())
               {
                  break;
               }

               ++tileCount;
            }

            // Check for abort request.
            if (needsAborting())
            {
               setPercentComplete(100);
               break;
            }

            setPercentComplete((100.0*(tileCount/totalTiles)));
         }
      }
   }
}

bool ossimImageHistogramSource::loadState(const ossimKeywordlist& kwl,
                                          const char* prefix)
{
   bool result = ossimHistogramSource::loadState(kwl, prefix);

   if ( result )
   {
      std::string myPrefix = (prefix?prefix:"");
      std::string key = "area_of_interest";
      ossimString value;
      value.string() = kwl.findKey( myPrefix, key);
      if ( value.size() )
      {
         theAreaOfInterest.toRect( value );
      }

      key = "mode";
      value.string() = kwl.findKey( myPrefix, key);
      if ( value.size() )
      {
         if(value == "normal")
         {
            theComputationMode = OSSIM_HISTO_MODE_NORMAL;
         }
         else if(value == "fast")
         {
            theComputationMode = OSSIM_HISTO_MODE_FAST;
         }
      }
   }
#if 0 /* old loadState drb - 20181114 */
   // setNumberOfInputs(2);
   // ossimString rect = kwl.find(prefix, "rect");

   if(!rect.empty())
   {
      loadState(kwl, prefix);
   }
   else 
   {
      ossimString newPrefix = ossimString(prefix) + "area_of_interest.";
      theAreaOfInterest.loadState(kwl, newPrefix);
   }
   
   ossimString mode = kwl.find(prefix, "mode");
   mode = mode.downcase();
   if(mode == "normal")
   {
      theComputationMode = OSSIM_HISTO_MODE_NORMAL;
   }
   else if(mode == "fast")
   {
      theComputationMode = OSSIM_HISTO_MODE_FAST;
   }
   if(getNumberOfInputs()!=1)
   {
      setNumberOfInputs(1);
   }
   
   // ossimString numberOfTiles = kwl.find(prefix, "number_of_tiles");
   // if(!numberOfTiles.empty())
   // {
   //    theNumberOfTilesToUseInFastMode = numberOfTiles.toUInt32();
   // }

   theInputListIsFixedFlag = true;
   theOutputListIsFixedFlag = false;
#endif
   
   return result;
}

bool ossimImageHistogramSource::saveState(ossimKeywordlist& kwl,
                                          const char* prefix)const
{
   bool result = ossimHistogramSource::saveState(kwl, prefix);
   if(result)
   {
      std::string myPrefix = (prefix?prefix:"");
      std::string key = "area_of_interest";
      ossimString value = theAreaOfInterest.toString();
      kwl.addPair( myPrefix, key, value.string() );

      key = "mode";
      if ( theComputationMode == OSSIM_HISTO_MODE_NORMAL )
      {
         value = "normal";
      }
      else if ( theComputationMode == OSSIM_HISTO_MODE_FAST )
      {
         value = "fast";
      }
      else
      {
         value = "unknown";
      }
      kwl.addPair( myPrefix, key, value.string() );
   }
   return result;
}
