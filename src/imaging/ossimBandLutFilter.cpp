//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
//
//*************************************************************************
// $Id: ossimBandLutFilter.cpp 23242 2015-04-08 17:31:35Z dburken $
#include <ossim/imaging/ossimBandLutFilter.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimFilenameProperty.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimNumericProperty.h>

RTTI_DEF1(ossimBandLutFilter, "ossimBandLutFilter", ossimImageSourceFilter);

static const char* MODE_KW = "mode";
static const char* LUT_FILE_KW = "lut_file";

ossimBandLutFilter::ossimBandLutFilter()
:ossimImageSourceFilter(),
 theMode(INTERPOLATED),
 theTile(0),
 theLutFile(""),
 theOutputScalarType(OSSIM_SCALAR_UNKNOWN)
{
   setDescription("Look-up-table remapper for band-independent remapping.");
}

ossimBandLutFilter::~ossimBandLutFilter()
{
}

ossimRefPtr<ossimImageData> ossimBandLutFilter::getTile(const ossimIrect& tileRect,
                                                              ossim_uint32 resLevel)
{
   if(!theInputConnection || (theLut.size() == 0))
      return 0;

   ossimRefPtr<ossimImageData> inputTile = theInputConnection->getTile(tileRect, resLevel);
   if (!inputTile || !inputTile->getBuf())
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
   if (inputTile->getDataObjectStatus() == OSSIM_EMPTY)
      return theTile;

   const double null_index = theInputConnection->getNullPixelValue();
   const double null_value = theTile->getNullPix(0); // assuming same null for all bands
   ossim_uint32 maxLength = inputTile->getWidth()*inputTile->getHeight();

   double index, value;
   double min_value=OSSIM_DEFAULT_MAX_PIX_DOUBLE;
   double max_value=OSSIM_DEFAULT_MIN_PIX_DOUBLE;

   ossim_uint32 numBands = getNumberOfInputBands();
   for (ossim_uint32 band = 0; band < numBands; ++band)
   {
      map<double, double>& bandMap = theLut[band];
      std::map<double, double>::const_iterator lut_entry;
      for (ossim_uint32 pixel=0; pixel<maxLength; ++pixel)
      {
         // Do not remap null pixels, leave the output pixel "blank" which is null-pixel value:
         index = inputTile->getPix(pixel, band);
         if (index == null_index)
            continue;

         // Now perform look-up depending on mode:
         if (theMode == LITERAL)
         {
            lut_entry = bandMap.find(index);
            if (lut_entry == bandMap.end())
               value = null_value;
            else
               value = lut_entry->second;
         }
         else
         {
            // Interpolated mode performs interpolation here between the line segments vertices:
            lut_entry = bandMap.find(index);
            if (lut_entry != bandMap.end())
            {
               // Got exact match, no interpolation needed:
               value = lut_entry->second;
            }
            else
            {
               lut_entry = bandMap.upper_bound(index);
               if ((lut_entry == bandMap.end()) || (lut_entry == bandMap.begin()))
                  value = null_value;
               else
               {
                  // Need to linearly interpolate:
                  double index_hi = lut_entry->first;
                  double value_hi (lut_entry->second);
                  --lut_entry;
                  double index_lo = lut_entry->first;
                  double value_lo (lut_entry->second);
                  double w_lo = (index_hi - index)/(index_hi - index_lo);
                  double w_hi = 1.0 - w_lo;
                  value = value_hi*w_hi + value_lo*w_lo;
               }
            }
         }

         if (value < min_value)
            min_value = value;
         if (value > max_value)
            max_value = value;

         // Assign this output pixel according to desired scalar type:
         switch(theOutputScalarType)
         {
         case OSSIM_DOUBLE:
            theTile->getDoubleBuf(band)[pixel] = value;
            theTile->setMinPix(min_value);
            theTile->setMaxPix(max_value);
            break;
         case OSSIM_SSHORT16:
            theTile->getSshortBuf(band)[pixel] = value;
            break;
         case OSSIM_FLOAT:
            theTile->setMinPix(min_value);
            theTile->setMaxPix(max_value);
            theTile->getFloatBuf(band)[pixel] = value;
            break;
         case OSSIM_NORMALIZED_FLOAT:
            theTile->getFloatBuf(band)[pixel] = value;
            break;
         case OSSIM_UCHAR:
            theTile->getUcharBuf(band)[pixel] = value;
            break;
         case OSSIM_USHORT16:
            theTile->getUshortBuf(band)[pixel] = value;
            break;
         case OSSIM_USHORT11:
         case OSSIM_USHORT12:
         case OSSIM_USHORT13:
         case OSSIM_USHORT14:
         case OSSIM_USHORT15:
            theTile->getUshortBuf(band)[pixel] = value;
            break;
         default:
            break;
         }
      } // end loop over band's pixels
   }  // end loop over tile's bands

   theTile->validate();
   return theTile;
}

void ossimBandLutFilter::allocate()
{
   if(!theInputConnection)
      return;

   theTile = ossimImageDataFactory::instance()->create(this, getNumberOfInputBands(), this);
   if(!theTile.valid())
      return;

   switch(theOutputScalarType)
   {
   case OSSIM_SSHORT16:
      theTile->setMinPix(-32768);
      theTile->setMaxPix(32767);
      break;
   case OSSIM_NORMALIZED_FLOAT:
      theTile->setMinPix(0.0);
      theTile->setMaxPix(1.0);
      break;
   case OSSIM_UCHAR:
      theTile->setMinPix(0);
      theTile->setMaxPix(255);
      break;
   case OSSIM_USHORT16:
      theTile->setMinPix(0);
      theTile->setMaxPix(65535);
      break;
   case OSSIM_USHORT11:
      theTile->setMinPix(0);
      theTile->setMaxPix(2047);
      break;
   case OSSIM_USHORT12:
      theTile->setMinPix(0);
      theTile->setMaxPix(4095);
      break;
   case OSSIM_USHORT13:
      theTile->setMinPix(0);
      theTile->setMaxPix(8191);
      break;
   case OSSIM_USHORT14:
      theTile->setMinPix(0);
      theTile->setMaxPix(16383);
      break;
   case OSSIM_USHORT15:
      theTile->setMinPix(0);
      theTile->setMaxPix(32767);
      break;
   default:
      break;
   }

   theTile->initialize();
}

void ossimBandLutFilter::initialize()
{
   // This assigns theInputConnection if one is there.
   ossimImageSourceFilter::initialize();

   // theTile will get allocated on first getTile call.
   theTile = 0;
   if ( theInputConnection )
   {
      // Initialize the chain on the left hand side of us.
      theInputConnection->initialize();
   }
}

bool ossimBandLutFilter::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   if (theLut.empty())
      return true;

   ossimString value;
   switch (theMode)
   {
   case LITERAL:
      value = "literal";
      break;
   case INTERPOLATED:
   default:
      value = "interpolated";
      break;
   }
   kwl.add(prefix, MODE_KW,  value.c_str(), true);

   bool rtn_stat = true;

   // Save the actual LUT:
   int numBands = theLut.size();
   for (int band=0; band<numBands; ++band)
   {
      ostringstream base_keyword;
      if (numBands > 1)
         base_keyword << "band" << band << ".";

      std::map<double, double>::const_iterator iter =  theLut[band].begin();
      ossim_uint32 entry = 0;
      while (iter != theLut[band].end())
      {
         ostringstream inKey, outKey;
         inKey  << base_keyword.str() << "entry" << entry <<".in";
         outKey << base_keyword.str() << "entry" << entry <<".out";

         ostringstream inVal, outVal;
         inVal  << iter->first;
         outVal << iter->second;

         kwl.add(prefix,  inKey.str().c_str(),  inVal.str().c_str());
         kwl.add(prefix, outKey.str().c_str(), outVal.str().c_str());
         ++iter;
         ++entry;
      }
   }
   rtn_stat &= ossimImageSourceFilter::saveState(kwl, prefix);
   return rtn_stat;
}

bool ossimBandLutFilter::loadState(const ossimKeywordlist& orig_kwl, const char* prefix)
{
   bool return_state = true;
   ossimKeywordlist kwl (orig_kwl); // need non-const copy

   // First look for a LUT filename, and add its contents to the original KWL:
   theLutFile = kwl.find(prefix, LUT_FILE_KW);
   if (!theLutFile.empty())
   {
      // Need new (non const) KWL to hold merged contents, maintaining proper prefix if any:
      ossimKeywordlist lut_kwl;
      if (lut_kwl.addFile(theLutFile))
         kwl.add(prefix, lut_kwl, false); // appends all entries of lut_kwl with prefix before adding
   }

   theMode = INTERPOLATED;
   ossimString lookup =  kwl.find(prefix, MODE_KW);
   if (lookup.contains("literal"))
      theMode = LITERAL;

   int scalar = ossimScalarTypeLut::instance()->getEntryNumber(kwl, prefix);
   if (scalar != ossimLookUpTable::NOT_FOUND)
      setOutputScalarType(static_cast<ossimScalarType>(scalar));

   return_state &= initializeLut(kwl, prefix);
   return_state &= ossimImageSourceFilter::loadState(orig_kwl, prefix);

   return return_state;
}

bool ossimBandLutFilter::initializeLut(const ossimKeywordlist& kwl, const char* prefix)
{
   theLut.clear();
   ossim_uint32 numBands = getNumberOfInputBands();
   bool usingBandPrefix = true;
   if (numBands <= 1)
   {
      ossim_uint32 numEntries = kwl.numberOf(prefix, "band0.in");
      if (numEntries == 0)
      {
         usingBandPrefix = false;
         numBands = 1;
      }
   }

   ossim_uint32 band = 0;
   while (true)
   {
      ossim_uint32 entry = 0;
      map<double,double> bandMap;
      while (true)
      {
         ostringstream inKey, outKey;
         if (usingBandPrefix)
         {
            inKey  << "band" << band << ".entry" << entry <<".in";
            outKey << "band" << band << ".entry" << entry <<".out";
         }
         else
         {
            inKey  << "entry" << entry <<".in";
            outKey << "entry" << entry <<".out";
         }

         ossimString inVal = kwl.find(prefix, inKey.str().c_str());
         if (inVal.empty())
            break;
         ossimString outVal = kwl.find(prefix, outKey.str().c_str());
         if (outVal.empty())
            break;

         bandMap.insert(std::pair<double,double>(inVal.toDouble(), outVal.toDouble()));
         ++entry;
      }

      if (bandMap.empty())
         break;

      theLut.push_back(bandMap);

      ++band;
      if ((numBands == 1) || !usingBandPrefix)
         break;

   }

   // Band still 0 would indicate a failure reading the first band entry:
   if (band == 0)
      return false;

   // Check that the correct number of bands are represented:
   if (theInputConnection && (numBands > band))
   {
      if (!usingBandPrefix)
      {
         // Use the same map for all bands
         for (ossim_uint32 i=1; i<numBands; ++i)
            theLut.push_back(theLut[0]);
      }
      else
      {
         // Not enough bands were specified in the KWL to represent all input bands. Set remaining
         // bands to no entries:
         map<double,double> nullBandMap;
         for (; band<numBands; ++band)
            theLut.push_back(nullBandMap);
      }
   }
   return true;
}

void ossimBandLutFilter::setLut(const ossimFilename& file)
{
   theLutFile = file;
   if(file.exists())
   {
      ossimKeywordlist kwl(theLutFile);
      loadState(kwl);
   }
}

void ossimBandLutFilter::setOutputScalarType(ossimScalarType scalarType)
{
   if (scalarType == OSSIM_SCALAR_UNKNOWN)
   {
      ossimNotify(ossimNotifyLevel_WARN)
               << "ossimScalarRemapper::setOutputScalarType WARN:\n"
               << "OSSIM_SCALAR_UNKNOWN passed to method.  No action taken..."
               << std::endl;
      return;
   }

   theOutputScalarType = scalarType;
   allocate();
}

void ossimBandLutFilter::setOutputScalarType(ossimString scalarType)
{
   int scalar = ossimScalarTypeLut::instance()->getEntryNumber(scalarType.c_str());

   if (scalar != ossimLookUpTable::NOT_FOUND)
   {
      setOutputScalarType(static_cast<ossimScalarType>(scalar));
   }
   else
   {
      ossimNotify(ossimNotifyLevel_WARN)
               << "ossimScalarRemapper ERROR:"
               << "\nUnknown scalar type:  " << scalarType.c_str() << std::endl;
   }
}

