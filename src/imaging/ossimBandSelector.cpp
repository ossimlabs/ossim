//*******************************************************************
//
// License: MIT
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for ossimBandSelector.
// 
//*******************************************************************
//  $Id$

#include <ossim/imaging/ossimBandSelector.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimStringProperty.h>
#include <iostream>
#include <algorithm>

static ossimTrace traceDebug("ossimBandSelector:debug");

RTTI_DEF1(ossimBandSelector,"ossimBandSelector", ossimImageSourceFilter)

ossimBandSelector::ossimBandSelector()
   :
      ossimImageSourceFilter(),
      m_tile(0),
      m_outputBandList(0),
      m_withinRangeFlag(ossimBandSelectorWithinRangeFlagState_NOT_SET),
      m_passThroughFlag(false),
      m_delayLoadRgbFlag(false),
      m_inputIsSelectable(false)
{
//   theEnableFlag = false; // Start off disabled.
   theEnableFlag = true; 
}

ossimBandSelector::~ossimBandSelector()
{
   m_tile = 0;
}

ossimRefPtr<ossimImageData> ossimBandSelector::getTile(
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   if (!theInputConnection)
   {
      return ossimRefPtr<ossimImageData>();
   }

   // Get the tile from the source.
   ossimRefPtr<ossimImageData> t = theInputConnection->getTile(tileRect, resLevel);


   if (!isSourceEnabled()||m_passThroughFlag)
   {
      return t;  // This tile source bypassed, return the input tile source.
   }

   if(!m_tile.valid()) // First time through, might not be initialized...
   {
      allocate();
      if (!m_tile.valid())
      {
         // Should never happen...
         return t; // initialize failed.
      }
   }

   m_tile->setImageRectangle(tileRect);
   if(m_withinRangeFlag == ossimBandSelectorWithinRangeFlagState_NOT_SET)
   {
      m_withinRangeFlag = ((outputBandsWithinInputRange() == true) ?
                            ossimBandSelectorWithinRangeFlagState_IN_RANGE:
                            ossimBandSelectorWithinRangeFlagState_OUT_OF_RANGE);
   }
   if(m_withinRangeFlag == ossimBandSelectorWithinRangeFlagState_OUT_OF_RANGE)
   {
      m_tile->makeBlank();
      return m_tile;
   }

   if ( !t.valid() ||
        (t->getDataObjectStatus() == OSSIM_EMPTY) ||
        (t->getDataObjectStatus() == OSSIM_NULL))
   {
      //---
      // Since we're enabled, we must return our tile not "t" so the
      // correct number of bands goes through the chain.
      //---
      m_tile->makeBlank();
      return m_tile;
   }

   // Copy selected bands to our tile.
   for ( ossim_uint32 i = 0; i < m_outputBandList.size(); ++i)
   {
      m_tile->assignBand(t.get(), m_outputBandList[i], i);
   }
   
   m_tile->validate();

   return m_tile;
}

void ossimBandSelector::setThreeBandRgb()
{
   m_outputBandList.clear();
   m_delayLoadRgbFlag = true;
   initialize();
}

void ossimBandSelector::setOutputBandList( const vector<ossim_uint32>& outputBandList,
                                           bool disablePassThru)
{
   if ( outputBandList.size() )
   {
      m_outputBandList = outputBandList;  // Assign the new list.
      if ( !disablePassThru )
      {
         bool setBands = false;
         ossimRefPtr<ossimImageHandler> ih = getBandSelectableImageHandler();
         if ( ih.valid() )
         {
            // Our input is a single image chain that can do band selection.
            if ( ih->setOutputBandList( outputBandList ) )
            {
               m_passThroughFlag = true;
               setBands = true;
               m_tile = 0; // Don't need.
            }
         }

         if ( setBands == false )
         {
            if ( m_tile.valid() )
            {
               // Check the tile band size and scalar.
               if ( ( m_tile->getNumberOfBands() != outputBandList.size() ) ||
                    ( m_tile->getScalarType() != theInputConnection->getOutputScalarType() ) )
               {
                  m_tile = 0;       // Force an allocate call next getTile.
               }
            }
            m_withinRangeFlag = ossimBandSelectorWithinRangeFlagState_NOT_SET;
            checkPassThrough();
            //theOrderedCorrectlyFlag = isOrderedCorrectly();
         }
      }
   }
}

ossim_uint32 ossimBandSelector::getNumberOfOutputBands() const
{
   ossim_uint32 bands;
   
   if(isSourceEnabled())
   {
      bands = static_cast<ossim_uint32>(m_outputBandList.size());
   }
   else
   {
      //---
      // Note:
      // This returns theInputConnection->getNumberOfOutputBands() which is our
      // input.  Calling ossimBandSelector::getNumberOfInputBands() will produce
      // an error if we are bypassed due to a band selectable image handler.
      //---
      bands = ossimImageSourceFilter::getNumberOfInputBands();
   }

   return bands;
}

ossim_uint32 ossimBandSelector::getNumberOfInputBands() const
{
   ossim_uint32 bands;

   // See if we have a single image chain with band selectable image handler.
   ossimRefPtr<ossimImageHandler> ih = getBandSelectableImageHandler();
   if ( ih.valid() )
   {
      bands = ih->getNumberOfInputBands();
   }
   else
   {
      bands = ossimImageSourceFilter::getNumberOfInputBands();
   }
   
   return bands;
}


void ossimBandSelector::initialize()
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_WARN) << "ossimBandSelector::initialize() entered...\n";
   }
      
   // Base class will recapture "theInputConnection".
   ossimImageSourceFilter::initialize();

   m_withinRangeFlag = ossimBandSelectorWithinRangeFlagState_NOT_SET;

   if(theInputConnection)
   {
      if ( !m_outputBandList.size() ) 
      {
         // First time through...
         if ( m_delayLoadRgbFlag )
         {
            //---
            // "bands" key was set to "rgb" or "default" in loadState but there
            // was not a connection to derive rgb bands from image handler.
            //---
            if ( getRgbBandList( m_outputBandList ) == false )
            {
               //---
               // Could not derive from input. Assuming caller wanted three bands.
               // Call to checkPassThrough()->outputBandsWithinInputRange()
               // will check the range of output band list.
               //---
               m_outputBandList.resize(3);
               if( theInputConnection->getNumberOfOutputBands() > 2 )
               {
                  m_outputBandList[0] = 0;
                  m_outputBandList[1] = 1;
                  m_outputBandList[2] = 2;
               }
               else
               {
                 m_outputBandList[0] = 0;
                 m_outputBandList[1] = 0;
                 m_outputBandList[2] = 0;                  
               }
            }
            
            m_delayLoadRgbFlag = false; // clear flag.
         }

         if ( !m_outputBandList.size() )
         {
            // Set the output band list to input if not set above.
            theInputConnection->getOutputBandList( m_outputBandList );
         }
         
         if( m_outputBandList.size() )
         {
            // Call to setOutputBandList configures band selector.
            setOutputBandList(m_outputBandList); 
         } 
         
      } // if ( !m_outputBandList.size() )
      
   } // if(theInputConnection)

   if ( !isSourceEnabled() )
   {
      m_tile = 0;
   }

   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_WARN) << "ossimBandSelector::initialize() exited...\n";
   }
}

void ossimBandSelector::allocate()
{
   //initialize(); // Update the connection.
   //theOrderedCorrectlyFlag = isOrderedCorrectly();
   m_tile = ossimImageDataFactory::instance()->create(this, this);
   m_tile->initialize();
}

bool ossimBandSelector::isSourceEnabled()const
{
   bool result = ossimImageSourceFilter::isSourceEnabled();
   if(result)
   {
      // if I am not marked to pass information on through then enable me
      result = !m_passThroughFlag;
   }
   
   return result;
}

double ossimBandSelector::getMinPixelValue(ossim_uint32 band)const
{
   if(theInputConnection)
   {
      if (isSourceEnabled())
      {
         if(band < m_outputBandList.size())
         {
            return theInputConnection->
               getMinPixelValue(m_outputBandList[band]);
         }
         else
         {
            return theInputConnection->getMinPixelValue(band);
         }
      }
      else
      {
         return theInputConnection->getMinPixelValue(band);
      }
   }
   return 0;
}

double ossimBandSelector::getNullPixelValue(ossim_uint32 band)const
{
   if(theInputConnection)
   {
      if (isSourceEnabled())
      {
         if(band < m_outputBandList.size())
         {
            return theInputConnection->
               getNullPixelValue(m_outputBandList[band]);
         }
         else
         {
            return theInputConnection->getNullPixelValue(band);
         }
      }
      else
      {
         return theInputConnection->getNullPixelValue(band);
      }
   }
   
   return 0;
}

double ossimBandSelector::getMaxPixelValue(ossim_uint32 band)const
{
   if(theInputConnection)
   {
      if (isSourceEnabled())
      {
         if(band < m_outputBandList.size())
         {
            return theInputConnection->
               getMaxPixelValue(m_outputBandList[band]);
         }
         else
         {
            return theInputConnection->getMaxPixelValue(band);
         }
      }
      else
      {
        return theInputConnection->getMaxPixelValue(band);
      }
   }
   return 1.0/DBL_EPSILON;
}

bool ossimBandSelector::saveState(ossimKeywordlist& kwl,
                                  const char* prefix)const
{
   ossimString temp;

   kwl.add(prefix,
           ossimKeywordNames::NUMBER_OUTPUT_BANDS_KW,
           static_cast<int>(m_outputBandList.size()),
           true);
   
   ossimString bandsString;
   ossim::toSimpleStringList(bandsString,
                             m_outputBandList);
   kwl.add(prefix,
           ossimKeywordNames::BANDS_KW,
           bandsString,
           true);
/*   
   for(ossim_uint32 counter = 0; counter < m_outputBandList.size();counter++)
   {
      temp  = ossimKeywordNames::BAND_KW;
      temp += ossimString::toString(counter+1);
      
      kwl.add(prefix,
              temp.c_str(),
              ossimString::toString(m_outputBandList[counter]+1).c_str());
   }
*/   
   return ossimImageSourceFilter::saveState(kwl, prefix);
}

bool ossimBandSelector::loadState(const ossimKeywordlist& kwl,
                                  const char* prefix)
{
   bool result = false;
   
   ossimImageSourceFilter::loadState(kwl, prefix);

   m_outputBandList.clear();
   
   ossimString copyPrefix = prefix;
   
   ossimString bands = kwl.find(prefix, ossimKeywordNames::BANDS_KW);
   if(!bands.empty())
   {
      ossimString bs = bands;
      bs.downcase();
      if ( bs == "rgb" )
      {
         // Flag initialize() to set the band list on first connection.
         m_delayLoadRgbFlag = true;
      }
      else
      {
         // Load from key:value, e.g. bands:(2,1,0)
         ossim::toSimpleVector(m_outputBandList, bands);
         result = true;
      }
   }
   else
   {
      ossimString regExpression =  ossimString("^(") + copyPrefix + "band[0-9]+)";
      
      vector<ossimString> keys = kwl.getSubstringKeyList( regExpression );
      long numberOfBands = (long)keys.size();
      ossim_uint32 offset = (ossim_uint32)(copyPrefix+"band").size();
      std::vector<int>::size_type idx = 0;
      std::vector<int> numberList(numberOfBands);
      for(idx = 0; idx < keys.size();++idx)
      {
         ossimString numberStr(keys[idx].begin() + offset,
                               keys[idx].end());
         numberList[idx] = numberStr.toInt();
      }
      std::sort(numberList.begin(), numberList.end());
      for(idx=0;idx < numberList.size();++idx)
      {
         const char* bandValue =
         kwl.find(copyPrefix,
                  ("band"+ossimString::toString(numberList[idx])).c_str());
         m_outputBandList.push_back( ossimString(bandValue).toLong()-1);
      }
      result = true;
   }

   initialize();
   
   return result;
}

void ossimBandSelector::checkPassThrough()
{
   m_passThroughFlag = ((theInputConnection == 0)||!outputBandsWithinInputRange());
   
   // check if marked with improper bands
   if(m_passThroughFlag) return;
   
   if(theInputConnection)
   {
      std::vector<ossim_uint32> inputList;
      theInputConnection->getOutputBandList(inputList);

      if ( inputList.size() == m_outputBandList.size() )
      {
         const std::vector<ossim_uint32>::size_type SIZE =
            m_outputBandList.size();
      
         std::vector<ossim_uint32>::size_type i = 0;
         while (i < SIZE)
         {
            if ( inputList[i] != m_outputBandList[i] )
            {
               break;
            }
            ++i;
         }
         if (i == SIZE)
         {
            m_passThroughFlag = true;
         }
      }
   }
   else
   {
      if(traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
         << "ossimBandSelector::isOrderedCorrectly() ERROR:"
         << "Method called prior to initialization!\n";
      }
   }

}

bool ossimBandSelector::outputBandsWithinInputRange() const
{
   bool result = false;
   
   if(theInputConnection)
   {
      result = true;
      const ossim_uint32 HIGHEST_BAND = getNumberOfInputBands() - 1;
      const ossim_uint32 OUTPUT_BANDS = (ossim_uint32)m_outputBandList.size();
      for (ossim_uint32 i=0; i<OUTPUT_BANDS; ++i)
      {
         if (m_outputBandList[i] > HIGHEST_BAND)
         {
            if(traceDebug())
            {
               ossimNotify(ossimNotifyLevel_WARN)
               << "ossimBandSelector::outputBandsWithinInputRange() ERROR:"
               << "Output band greater than highest input band. "
               << m_outputBandList[i] << " > " << HIGHEST_BAND << "."
               << std::endl;
            }
            result = false;
            break;
         }
      }
   }
   else
   {
      if(traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
         << "ossimBandSelector::outputBandsWithinInputRange() ERROR:"
         << "Method called prior to initialization!" << std::endl;
      }
   }
   return result;
}

void ossimBandSelector::getOutputBandList(std::vector<ossim_uint32>& bandList) const
{
   if ( isSourceEnabled()&&m_outputBandList.size() )
   {
      bandList = m_outputBandList;
   }
   else if (theInputConnection)
   {
      theInputConnection->getOutputBandList(bandList);
   }
   else
   {
      bandList.clear();
   }
}

ossimString ossimBandSelector::getLongName()const
{
   return ossimString("Band Selector, maps an input band to the output band.");
}

ossimString ossimBandSelector::getShortName()const
{
   return ossimString("Band Selector");
}

void ossimBandSelector::setProperty(ossimRefPtr<ossimProperty> property)
{
   if( property.valid() )
   {
      if(property->getName() == "bands")
      {
         std::vector<ossim_uint32> selection;
         if(ossim::toSimpleVector(selection, property->valueToString()))
         {
            m_outputBandList = selection;
            setOutputBandList( m_outputBandList );
         }
      }
      else if(property->getName() == "bandSelection")
      {
         ossimString str = property->valueToString();
         std::vector<ossimString> str_vec;
         std::vector<ossim_uint32> int_vec;
         
         str.split( str_vec, " " );
         
         for ( ossim_uint32 i = 0; i < str_vec.size(); ++i )
         {
            if(!str_vec[i].empty())
            {
               int_vec.push_back( str_vec[i].toUInt32() );
            }
         }
         setOutputBandList( int_vec );
      }
      else
      {
         ossimImageSourceFilter::setProperty(property);
      }
   }
}

ossimRefPtr<ossimProperty> ossimBandSelector::getProperty(const ossimString& name)const
{
   if( name == "bands" )
   {
      ossimString bandsString;
      ossim::toSimpleStringList(bandsString,
                                m_outputBandList);
      ossimStringProperty* stringProp = new ossimStringProperty(name, bandsString);
      
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
   }
   else if( name == "bandSelection" )
   {
      std::vector<ossim_uint32> bands;
	  
      getOutputBandList( bands );
      
      std::vector<ossimString> bandNames;
      
      for(ossim_uint32 i = 0; i < bands.size(); i++)
      {
         bandNames.push_back( ossimString::toString( bands[i] ) );
      }
      
      ossimString str;
      
      str.join( bandNames, " " );
      
      ossimStringProperty* stringProp = new ossimStringProperty(name, str);
      
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
   }

   return ossimImageSourceFilter::getProperty(name);
}

void ossimBandSelector::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   ossimImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back("bands");
}

ossimRefPtr<ossimImageHandler> ossimBandSelector::getBandSelectableImageHandler() const
{
   ossimRefPtr<ossimImageHandler> ih = 0;

   if ( theInputConnection )
   {
      /**
       * GP: We will only allow the immediate input check for an image handler
       * and if one is present then check if selectable
       */
      ossimTypeNameVisitor visitor(ossimString("ossimImageHandler"),
                                   true,
                                   ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
      ossimTypeNameVisitor bandSelectorVisitor(ossimString("ossimBandSelector"),
                                   true,
                                   ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
      
      theInputConnection->accept(visitor);
      theInputConnection->accept(bandSelectorVisitor);
      // if there is a band selector before us then do not 
      // set the image handler
      if(bandSelectorVisitor.getObjects().size() < 1)
      {
         // If there are multiple image handlers, e.g. a mosaic do not uses.
         if ( visitor.getObjects().size() == 1 )
         {
            ih = visitor.getObjectAs<ossimImageHandler>( 0 );
            if ( ih.valid() )
            {
               if ( ih->isBandSelector() == false )
               {
                  ih = 0;
               }
            }
         }

      }
      
   } // Matches: if ( theInputConnection )
   return ih;
   
} // End: ossimBandSelector::getBandSelectableImageHandler()

bool ossimBandSelector::getRgbBandList(std::vector<ossim_uint32>& bandList) const
{
   bool result = false;
   
   if ( theInputConnection )
   {
      ossimTypeNameVisitor visitor(ossimString("ossimImageHandler"),
                                   true,
                                   ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
      
      theInputConnection->accept(visitor);

      // If there are multiple image handlers, e.g. a mosaic do not uses.
      if ( visitor.getObjects().size() == 1 )
      {
         ossimRefPtr<ossimImageHandler> ih = visitor.getObjectAs<ossimImageHandler>( 0 );
         if ( ih.valid() )
         {
            result = ih->getRgbBandList( bandList );
         }
      }
      
   } // Matches: if ( theInputConnection )

   return result;
   
} // End: ossimBandSelector::getRgbBandList( ... )
