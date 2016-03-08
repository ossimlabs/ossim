//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
//
//*************************************************************************
// $Id: ossimIndexToRgbLutFilter.cpp 23242 2015-04-08 17:31:35Z dburken $
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/base/ossimRgbVector.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimFilenameProperty.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimNumericProperty.h>

static const ossimTrace traceDebug("ossimIndexToRgbLutFilter:debug");

RTTI_DEF1(ossimIndexToRgbLutFilter, "ossimIndexToRgbLutFilter", ossimImageSourceFilter);

static const char* MIN_VALUE_KW = "min_value";
static const char* MAX_VALUE_KW = "max_value";
static const char* MODE_KW = "mode";
static const char* LUT_FILE_KW = "lut_file";

ossimIndexToRgbLutFilter::ossimIndexToRgbLutFilter()
:ossimImageSourceFilter(),
 theMinValue(ossim::nan()),
 theMaxValue(ossim::nan()),
 theMinValueOverride(false),
 theMaxValueOverride(false),
 theMode(REGULAR),
 theTile(0),
 theLutFile("")
{
   setDescription("Look-up-table remapper from single-band index image to 24-bit RGB.");
}

ossimIndexToRgbLutFilter::~ossimIndexToRgbLutFilter()
{
}

ossimRefPtr<ossimImageData> ossimIndexToRgbLutFilter::getTile(const ossimIrect& tileRect,
                                                              ossim_uint32 resLevel)
{
   if(!theInputConnection || (theLut.size() == 0))
      return 0;

   ossimRefPtr<ossimImageData> tile = theInputConnection->getTile(tileRect, resLevel);
   if (!tile || !tile->getBuf())
      return 0;
   if(!theTile)
   {
      allocate();
      if (!theTile)
         return 0;
   }

   theTile->setImageRectangle(tileRect);
   theTile->makeBlank();

   // Quick handling special case of empty input tile:
   if (tile->getDataObjectStatus() == OSSIM_EMPTY)
      return theTile;

   ossim_uint8* outBuf[3];
   outBuf[0] = (ossim_uint8*)(theTile->getBuf(0));
   outBuf[1] = (ossim_uint8*)(theTile->getBuf(1));
   outBuf[2] = (ossim_uint8*)(theTile->getBuf(2));

   ossim_uint32 maxLength = tile->getWidth()*tile->getHeight();
   ossimRgbVector color;
   const ossimRgbVector null_color (theTile->getNullPix(0), theTile->getNullPix(1), theTile->getNullPix(2));
   double index;
   std::map<double, ossimRgbVector>::const_iterator lut_entry;
   double null_index = theInputConnection->getNullPixelValue();

   for (ossim_uint32 pixel=0; pixel<maxLength; ++pixel)
   {
      // Convert input pixel to a double index value:
      switch(tile->getScalarType())
      {
      case OSSIM_DOUBLE:
         index = ((double*) tile->getBuf())[pixel];
         break;
      case OSSIM_SSHORT16:
         index = (double)(((ossim_sint16*) tile->getBuf())[pixel]);
         break;
      case OSSIM_FLOAT:
      case OSSIM_NORMALIZED_FLOAT:
         index = (double)(((float*) tile->getBuf())[pixel]);
         break;
      case OSSIM_UCHAR:
         index = (double)(((ossim_uint8*) tile->getBuf())[pixel]);
         break;
      case OSSIM_USHORT16:
      case OSSIM_USHORT11:
         index = (double)(((ossim_uint16*) tile->getBuf())[pixel]);
         break;
      default:
         break;
      }

      // Do not remap null pixels, leave the output pixel "blank" which is null-pixel value:
      if (index == null_index)
         continue;

      // REGULAR mode needs to clamp the indices to min max for non-null pixels:
      if (theMode == REGULAR)
      {
         if (index < theMinValue)
            index = theMinValue;
         else if (index > theMaxValue)
            index = theMaxValue;
      }

      // Now perform look-up depending on mode:
      if (theMode == LITERAL)
      {
         lut_entry = theLut.find(index);
         if (lut_entry == theLut.end())
            color = null_color;
         else
            color = lut_entry->second;
      }
      else
      {
         // Vertices and Regular mode perform same interpolation here between the line segments
         // vertices:
         lut_entry = theLut.find(index);
         if (lut_entry != theLut.end())
         {
            // Got exact match, no interpolation needed:
            color = lut_entry->second;
         }
         else
         {
            lut_entry = theLut.upper_bound(index);
            if ((lut_entry == theLut.end()) || (lut_entry == theLut.begin()))
               color = null_color;
            else
            {
               // Need to linearly interpolate:
               double index_hi = lut_entry->first;
               ossimRgbVector color_hi (lut_entry->second);
               --lut_entry;
               double index_lo = lut_entry->first;
               ossimRgbVector color_lo (lut_entry->second);
               double w_lo = (index_hi - index)/(index_hi - index_lo);
               double w_hi = 1.0 - w_lo;
               color.setR(ossim::round<ossim_uint8, double>( color_hi.getR()*w_hi + color_lo.getR()*w_lo ));
               color.setG(ossim::round<ossim_uint8, double>( color_hi.getG()*w_hi + color_lo.getG()*w_lo ));
               color.setB(ossim::round<ossim_uint8, double>( color_hi.getB()*w_hi + color_lo.getB()*w_lo ));
            }
         }
      }

      // Assign this output pixel:
      outBuf[0][pixel]  = color.getR();
      outBuf[1][pixel]  = color.getG();
      outBuf[2][pixel]  = color.getB();

   } // end loop over tile's pixels

   theTile->validate();
   return theTile;
}

void ossimIndexToRgbLutFilter::allocate()
{
   if(!theInputConnection) return;

   theTile = ossimImageDataFactory::instance()->create(this, 3, this);
   if(theTile.valid())
   {
      theTile->initialize();
   }
}

void ossimIndexToRgbLutFilter::initialize()
{
   // This assigns theInputConnection if one is there.
   ossimImageSourceFilter::initialize();

   // theTile will get allocated on first getTile call.
   theTile = 0;

   if ( theInputConnection )
   {
      // Initialize the chain on the left hand side of us.
      theInputConnection->initialize();
      if(!theMinValueOverride)
         theMinValue = theInputConnection->getMinPixelValue(0);
      if(!theMaxValueOverride)
         theMaxValue = theInputConnection->getMaxPixelValue(0);
      if(theMinValue > theMaxValue)
         swap(theMinValue, theMaxValue);
   }
}

void ossimIndexToRgbLutFilter::setProperty(ossimRefPtr<ossimProperty> property)
{
   if(property.valid())
   {
      ossimString value = property->valueToString();
      value = value.trim();

      if(property->getName() == "LUT file")
      {
         setLut(ossimFilename(property->valueToString()));
      }
      else
      {
         ossimImageSourceFilter::setProperty(property);
      }
   }
}

ossimRefPtr<ossimProperty> ossimIndexToRgbLutFilter::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> property = 0;
   if(name == "LUT file")
   {
      ossimFilenameProperty* filenameProperty = new ossimFilenameProperty(name, theLutFile);
      filenameProperty->setIoType(ossimFilenameProperty::ossimFilenamePropertyIoType_INPUT);
      filenameProperty->clearChangeType();
      filenameProperty->setCacheRefreshBit();
      filenameProperty->setReadOnlyFlag(false);
      property = filenameProperty;
   }
   else
   {
      property = ossimImageSourceFilter::getProperty(name);
   }
   return property;
}

void ossimIndexToRgbLutFilter::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   ossimImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back("LUT file");
}

bool ossimIndexToRgbLutFilter::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   if (theMinValueOverride)
      kwl.add(prefix, MIN_VALUE_KW, theMinValue, true);

   if (theMaxValueOverride)
      kwl.add(prefix, MAX_VALUE_KW, theMaxValue, true);

   ossimString value;
   switch (theMode)
   {
   case LITERAL:
      value = "literal";
      break;
   case VERTICES:
      value = "vertices";
      break;
   default:
      value = "regular";
   }
   kwl.add(prefix, MODE_KW,  value.c_str(), true);

   bool rtn_stat = true;

   // Save the actual LUT:
   const ossimString entry_kw ("entry");
   ossimString color_keyword, base_keyword;
   ossimRgbVector rgbVector;
   ossimString blank(" ");

   ossim_uint32 entry = 0;
   std::map<double, ossimRgbVector>::const_iterator iter =  theLut.begin();
   while (iter != theLut.end())
   {
      base_keyword = entry_kw + ossimString::toString(entry);
      if ((theMode == LITERAL) || (theMode == VERTICES))
      {
         // Index and color are sub-entries for these modes:
         kwl.add(prefix, (base_keyword+".index").chars(), iter->first);
         color_keyword = base_keyword + ".color";
      }
      else
      {
         color_keyword = base_keyword;
      }

      rgbVector = iter->second;
      value = ossimString::toString((int)rgbVector.getR()) + blank +
            ossimString::toString((int)rgbVector.getG()) + blank +
            ossimString::toString((int)rgbVector.getB());
      kwl.add(prefix, color_keyword.chars(), value.chars());
      ++iter;
      ++entry;
   }

   rtn_stat &= ossimImageSourceFilter::saveState(kwl, prefix);
   return rtn_stat;
}

bool ossimIndexToRgbLutFilter::loadState(const ossimKeywordlist& orig_kwl, const char* prefix)
{
   bool return_state = true;
   ossimKeywordlist* kwl = new ossimKeywordlist(orig_kwl); // need non-const copy

   // First look for a LUT filename, and add its contents to the original KWL:
   theLutFile = kwl->find(prefix, LUT_FILE_KW);
   if (!theLutFile.empty())
   {
      // Need new (non const) KWL to hold merged contents, maintaining proper prefix if any:
      ossimKeywordlist lut_kwl;
      if (lut_kwl.addFile(theLutFile))
         kwl->add(prefix, lut_kwl, false); // appends all entries of lut_kwl with prefix before adding
   }

   theMinValueOverride = false;
   ossimString lookup = kwl->find(prefix, MIN_VALUE_KW);
   if(!lookup.empty())
   {
      theMinValue =lookup.toDouble();
      theMinValueOverride = true;
   }

   theMaxValueOverride = false;
   lookup = kwl->find(prefix, MAX_VALUE_KW);
   if(!lookup.empty())
   {
      theMaxValue = lookup.toDouble();
      theMaxValueOverride = true;
   }

   lookup =  kwl->find(prefix, MODE_KW);
   if (lookup.contains("literal"))
      theMode = LITERAL;
   else if (lookup.contains("vertices"))
      theMode = VERTICES;
   else
      theMode = REGULAR;

   return_state &= initializeLut(kwl, prefix);
   return_state &= ossimImageSourceFilter::loadState(orig_kwl, prefix);

   delete kwl;
   return return_state;
}

bool ossimIndexToRgbLutFilter::initializeLut(const ossimKeywordlist* kwl, const char* prefix)
{
   theLut.clear();

   const ossimString entry_kw ("entry");
   ossimString keyword, base_keyword;
   ossimString indexStr, rgbStr;
   ossimString blank(" ");
   std::vector<ossimString> rgbList;
   double index;
   ossimRgbVector rgbVector (0,0,0);
   bool rtn_state = true;

   ossim_uint32 numEntries=0;
   while (true)
   {
      base_keyword = entry_kw + ossimString::toString(numEntries);
      if ((theMode == LITERAL) || (theMode == VERTICES))
      {
         // Index and color are subentries for this mode:
         keyword = base_keyword + ".index";
         indexStr = kwl->find(prefix, keyword.chars());
         if (indexStr.empty())
            break;

         index = indexStr.toDouble();
         keyword = base_keyword + ".color";
         rgbStr = kwl->find(prefix, keyword.chars());
      }
      else
      {
         // REGULAR mode: index is computed later to arrive at equally-spaced vertices. For now,
         // just store entry number as index:
         index = (double) numEntries;
         keyword = base_keyword;
         rgbStr = kwl->find(prefix, keyword.chars());
         if (rgbStr.empty())
         {
            // Perhaps old bloated form with separate keywords for R, G, B
            rgbStr = kwl->find(prefix, (keyword + ".r").chars());
            rgbStr += " ";
            rgbStr += kwl->find(prefix, (keyword + ".g").chars());
            rgbStr += " ";
            rgbStr += kwl->find(prefix, (keyword + ".b").chars());
            if (rgbStr.length() < 5)
               break;
         }
      }

      rgbStr.split(rgbList, blank, true);
      if (rgbList.size() != 3)
      {
         ossimNotify(ossimNotifyLevel_WARN)<<"ossimIndexToRgbLutFilter::initializeLut() -- "
               "Bad color specification in LUT KWL. LUT is not properly initialized."<<endl;
         return false;
      }

      rgbVector.setR(rgbList[0].toUInt8());
      rgbVector.setG(rgbList[1].toUInt8());
      rgbVector.setB(rgbList[2].toUInt8());
      theLut.insert(std::pair<double, ossimRgbVector>(index, rgbVector));
      rgbList.clear();
      ++numEntries;
   }

   // For REGULAR mode, need to adjust the indices to reflect a piecewise linear LUT with equally
   // spaced vertices:
   if (theMode == REGULAR)
   {
      std::map<double, ossimRgbVector> orig_lut = theLut;
      std::map<double, ossimRgbVector>::iterator iter =  orig_lut.begin();
      theLut.clear();

      if (numEntries == 1)
      {
         // Insert the implied start index at black and endpoint at specified color:
         theLut.insert(std::pair<double, ossimRgbVector>(theMinValue, ossimRgbVector (1, 1, 1)));
         theLut.insert(std::pair<double, ossimRgbVector>(theMaxValue, iter->second));
      }
      else
      {
         // Loop to create equally-spaced vertices between min and max index values:
         double interval = (theMaxValue - theMinValue) / (numEntries - 1);
         while (iter != orig_lut.end())
         {
            index = theMinValue + iter->first*interval;
            theLut.insert(std::pair<double, ossimRgbVector>(index, iter->second));
            ++iter;
         }
      }
   }

   return rtn_state;
}

ossim_uint32 ossimIndexToRgbLutFilter::getNumberOfOutputBands() const
{
   if(isSourceEnabled())
   {
      return 3;
   }
   return ossimImageSourceFilter::getNumberOfOutputBands();
}

ossimScalarType ossimIndexToRgbLutFilter::getOutputScalarType() const
{
   if(isSourceEnabled())
   {
      return OSSIM_UCHAR;
   }

   return ossimImageSourceFilter::getOutputScalarType();
}

void ossimIndexToRgbLutFilter::setLut(const ossimFilename& file)
{
   theLutFile = file;
   if(file.exists())
   {
      ossimKeywordlist kwl(theLutFile);
      loadState(kwl);
   }
}

double ossimIndexToRgbLutFilter::getMinValue()const
{
   return theMinValue;
}

double ossimIndexToRgbLutFilter::getMaxValue()const
{
   return theMaxValue;
}

void ossimIndexToRgbLutFilter::setMinValue(double value)
{
   theMinValue = value;
}

void ossimIndexToRgbLutFilter::setMaxValue(double value)
{
   theMaxValue = value;
}

double ossimIndexToRgbLutFilter::getNullPixelValue(ossim_uint32 /* band */ )const
{
   return 0.0;
}

double ossimIndexToRgbLutFilter::getMinPixelValue(ossim_uint32 /* band */)const
{
   return 1.0;
}

double ossimIndexToRgbLutFilter::getMaxPixelValue(ossim_uint32 /* band */)const
{
   return 255.0;
}


