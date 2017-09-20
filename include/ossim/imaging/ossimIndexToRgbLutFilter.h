//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
//
//*************************************************************************
// $Id: ossimIndexToRgbLutFilter.h 23616 2015-11-11 19:50:29Z dburken $
#ifndef ossimIndexToRgbLutFilter_HEADER
#define ossimIndexToRgbLutFilter_HEADER

#include <ossim/base/ossimRgbVector.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <vector>

class ossimImageData;

/***************************************************************************************************
 *
 * This class provides a look-up-table remap from a single band input (the index), to a 3-band RGB
 * output. The LUT is provided as a KWL with several remapping modes supported:
 *
 * Literal: Only those indices represented in the KWL are remapped to the values indicated. All
 * other pixel values are mapped to the NULL pixel (0,0,0). Note that this is only meaningful for
 * integer input scalar types since a normalized input will likely not find an exact match.
 *
 * Arbitrary piecewise linear ("vertices"): The LUT KWL provides vertices to contiguous line
 * segments. Pixel values falling between specified indices are linearly interpolated between
 * adjacent vertices. Any index pixel values falling outside the range between min and max vertex
 * indices will be mapped to the NULL pixel.
 *
 * Regular piecewise linear ("regular"): The entries in the KWL do not correspond to any specific
 * index (except the first and the last entries that correspond to the min and max pixel values).
 * Remaining intermediate entries are equally spaced in index space so that the line segments are of
 * equal length. The min and max values are queried from the input source, but can be overriden with
 * the "min_value" and "max_value" keywords in the LUT KWL.
 *
 * Here are example KWLs for the three modes, first for the literal remap:
 *
 *    type: ossimIndexToRgbLutFilter
 *    mode: literal
 *    entry0.index: 0
 *    entry0.color: 1 1 1
 *    entry1.index: 128
 *    entry1.color: 255 0 0
 *    entry2.index: 255
 *    entry2.color: 0 255 0
 *
 * The above KWL will map only pixels with input values of 0, 128, and 255. All other indices will
 * map to the null value.
 *
 * Example for arbitrary piecewise linear:
 *
 *    type: ossimIndexToRgbLutFilter
 *    mode: vertices
 *    entry0.index: 1
 *    entry0.color: 0 0 255
 *    entry1.index: 128
 *    entry1.color: 0 255 0
 *    entry2.index: 250
 *    entry2.color: 255 0 0
 *
 * The above KWL is a sort of heat map where indices between 1 and 128 will linearly map from blue
 * to green, and then 128 to 250 will map from green (through yellow) to red. Note that any value
 * above 250 as well as 0 are outside of the remap range and will map to the null pixel (0,0,0).
 *
 * Example of regular piecewise linear. This is the default mode. For backward compatibility, the
 * mode keyword here is optional and if omitted will imply this mode. Also, unlike previous form,
 * the number_of_entries keyword is not required and is ignored if present.
 *
 *    type: ossimIndexToRgbLutFilter
 *    mode: regular
 *    entry0: 0 0 255
 *    entry1: 0 255 0
 *    entry2: 255 0 0
 *    max_value:  250
 *    min_value:  1
 *
 * The above KWL is almost equivalent to the "vertices" example above, i.e., a heat map. However,
 * the individual vertices are not specified. Instead, three entries will define two line segments.
 * The first line segment will commence with index 1 (min_value) and finish at:
 *
 *    [(max_value-min_value)/(number_of_entries-1)] + min_value
 *
 * In this case it will be 125.5. The quantity in square-brackets (124.5) is the interval, or
 * length of each line segment, so the second line segment will go from 125.5 to 250. Input pixel
 * values below the min_value will clamp to the min value (except the null pixel) and those above
 * the max value will clamp to the max value.
 *
 * The table can be contained in a separate file from the main KWL state file. In this case, the
 * loadState can accept a file name in place of a complete table:
 *
 *    type: ossimIndexToRgbLutFilter
 *    lut_file: <path to LUT KWL file>
 * 
 * The format of the LUT KWL file is just as specified above for the inline case.
 *
 **************************************************************************************************/
class OSSIM_DLL ossimIndexToRgbLutFilter : public ossimImageSourceFilter
{
public:
   enum Mode  { LITERAL = 0, VERTICES  = 1, REGULAR = 2 };

   ossimIndexToRgbLutFilter();

   virtual ~ossimIndexToRgbLutFilter();

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& origin,
                                               ossim_uint32 resLevel=0);
   
   ossim_uint32 getNumberOfOutputBands() const;
   
   virtual ossimScalarType getOutputScalarType() const;
   
   void           setMode(Mode mode) { theMode = mode; }
   Mode           getMode() const { return theMode; }

   /**
    * @brief Set lookup table(lut) method.
    *
    * This opens the keyword list and initializes lut.
    *
    * CAUTION: Requires theMinValue, theMaxValue, and theMode to be set.  Typically a
    * connection is made prior to this call.  If not connected to an input, "min",
    * "max" and "mode" keywords should be passed in the keyword list file.
    *
    * @param file Keyword list containing lut.
    */
   void           setLut(const ossimFilename& file);
   
   double         getMinValue()const;
   double         getMaxValue()const;
   
   void           setMinValue(double value);
   void           setMaxValue(double value);
   
   virtual double getNullPixelValue(ossim_uint32 band=0)const;
   virtual double getMinPixelValue(ossim_uint32 band=0)const;
   virtual double getMaxPixelValue(ossim_uint32 band=0)const;

   virtual void initialize();

   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=NULL)const;

   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=NULL);

protected:
   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   bool initializeLut(const ossimKeywordlist* kwl, const char* prefix=0);
   
   std::map<double, ossimRgbVector> theLut;

   double theMinValue;
   double theMaxValue;
   bool   theMinValueOverride;
   bool   theMaxValueOverride;
   Mode   theMode;
   ossimRefPtr<ossimImageData> theTile;
   ossimFilename   theLutFile;
   
TYPE_DATA
};

#endif /* #ifndef ossimIndexToRgbLutFilter_HEADER */
