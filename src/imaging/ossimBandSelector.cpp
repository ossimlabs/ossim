//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for ossimBandSelector.
// 
//*******************************************************************
//  $Id: ossimBandSelector.cpp 23426 2015-07-15 14:51:23Z okramer $

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
      m_passThroughFlag(false)
   

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

//   if (theOrderedCorrectlyFlag)
//   {
//      return t; // Input band order same as output band order.
//   }

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
   for (ossim_uint32 i=0; i<m_outputBandList.size(); i++)
   {
      m_tile->assignBand(t.get(), m_outputBandList[i], i);
   }
   
   m_tile->validate();

   return m_tile;
}

void ossimBandSelector::setOutputBandList( const vector<ossim_uint32>& outputBandList,
                                           bool disablePassThru)
{
   if (outputBandList.size() == 0)
      return;

   m_outputBandList = outputBandList;  // Assign the new list.
   if (disablePassThru)
      return;

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
      if ( m_tile.valid() && ( m_tile->getNumberOfBands() != outputBandList.size() ) )
      {
         m_tile = 0;       // Force an allocate call next getTile.
      }
      m_withinRangeFlag = ossimBandSelectorWithinRangeFlagState_NOT_SET;
      checkPassThrough();
      //theOrderedCorrectlyFlag = isOrderedCorrectly();
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
         ossimNotify(ossimNotifyLevel_WARN) << "ossimBandSelector::initialize(): Entered...............\n";
   }
   // Base class will recapture "theInputConnection".
   ossimImageSourceFilter::initialize();
   std::vector<ossim_uint32> outputBandList;
   m_withinRangeFlag = ossimBandSelectorWithinRangeFlagState_NOT_SET;

//std::cout << "OUTPUT BAND LIST VALUES===== ";
//   std::copy(m_outputBandList.begin(), m_outputBandList.end(),
//      std::ostream_iterator<ossim_int32>(std::cout, " "));
//std::cout << "\n";

   if(theInputConnection)
   {
      if ( !m_outputBandList.size() ) 
      {
         // First time through...
         if ( getRgbBandList( outputBandList ) == false )
         {
            theInputConnection->getOutputBandList(outputBandList);
         }
      }

#if 0
         if ( theDelayLoadRgbFlag )
         {
            //---
            // "bands" key was set to "rgb" in loadState but there was not a
            // connection to derive rgb bands from image handler.
            //---
            if ( getRgbBandList( m_outputBandList ) == false )
            {
               ossim_uint32 nOutputBands = theInputConnection->getNumberOfOutputBands();

               //---
               // Could not derive from input. Assuming caller wanted three bands.
               // Call to checkPassThrough()->outputBandsWithinInputRange()
               // will check the range of output band list.
               //---

               if(nOutputBands >2)
               {
                 outputBandList.resize(3);
                 outputBandList[0] = 0;
                 outputBandList[1] = 1;
                 outputBandList[2] = 2;
               }
               else if(nOutputBands > 0)
               {
                 outputBandList.resize(3);
                 outputBandList[0] = 0;
                 outputBandList[1] = 0;
                 outputBandList[2] = 0;                  
               }

            }
            else
            {

            }
            theDelayLoadRgbFlag = false; // clear flag.
         }

         if ( !outputBandList.size() )
         {
            std::cout << "GETTING REGULAR OUTPUT BAND LIST!!!!\n";
            // Set the output band list to input if not set above.
            theInputConnection->getOutputBandList(m_outputBandList);
         }
      }
#endif
      // NOTE: The following code was already being done in setOutputBandList(), so ignored here
      // until someone complains. Need the ability to force this selector to be active even if
      // the input handler is a band-selectable variety. (OLK Jul/2015)
#if 0
      // See if we have a single image chain with band selectable image handler.
      ossimRefPtr<ossimImageHandler> ih = getBandSelectableImageHandler();
      if ( ih.valid() )
      {
         if ( m_outputBandList.size() )
         {
            ih->setOutputBandList( m_outputBandList );
         }
         m_passThroughFlag = true;
      }
      else
      {
         checkPassThrough();
      }
#endif

      if ( isSourceEnabled() )
      {
         // theOrderedCorrectlyFlag = isOrderedCorrectly();
         
         if ( m_tile.valid() )
         {
            //---
            // Check for:
            // - ordered correctly
            // - band change
            // - scalar change
            //---
            if( ( m_tile->getNumberOfBands() != m_outputBandList.size() ) ||
                ( m_tile->getScalarType() !=
                  theInputConnection->getOutputScalarType() ) )
            {
               m_tile = 0; // Don't need it.
            }
         }
         if(outputBandList.size())
         {
           setOutputBandList(outputBandList); 
         } 
      }
   }

   if ( !isSourceEnabled() )
   {
      m_tile = 0;
   }
   if(traceDebug())
   {
         ossimNotify(ossimNotifyLevel_WARN) << "ossimBandSelector::initialize(): Leaving...............\n";
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
      if ( (bs != "rgb" ) && (bs != "default"))
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
   if(!property) return;

   if(property->getName() == "bandSelection")
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
   else if(property->getName() == "bands")
   {
      std::vector<ossim_uint32> selection;
      if(ossim::toSimpleVector(selection, property->valueToString()))
      {
         m_outputBandList = selection;
      }
   }
   else
   {
      ossimImageSourceFilter::setProperty(property);
   }
}

ossimRefPtr<ossimProperty> ossimBandSelector::getProperty(const ossimString& name)const
{
   if(name == "bandSelection")
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
   else if(name == "bands")
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
//      ih = dynamic_cast<ossimImageHandler*>(theInputConnection);
      
//      if(ih.valid())
//      {
//         if(!ih->isBandSelector())
//         {
//            ih = 0;
//         }
//      }
      /**
      * GP: commenting out for there could be two band selectors in the chain
      * we will only allow the immediate input check for an image handler
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
