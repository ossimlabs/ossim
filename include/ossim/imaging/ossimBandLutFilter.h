//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
//
//*************************************************************************
// $Id: ossimBandLutFilter.h 23616 2015-11-11 19:50:29Z dburken $
#ifndef ossimBandLutFilter_HEADER
#define ossimBandLutFilter_HEADER

#include <ossim/base/ossimRgbVector.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <vector>

class ossimImageData;

/***************************************************************************************************
 *
 * This class provides a look-up-table remap capability for performing band-separate remapping to
 * an output with the same number of bands. This is similar to ossimIndexToRgbLutFilter except it
 * works independently on the input bands. The number of output bands is necessarily equal to the
 * number of input bands, but the scalar data type between input and output can be different (just
 * like with the ossimScalarRemapper).
 *
 * The LUT is provided as a KWL with several remapping modes supported:
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
 * Here are example KWLs for the three modes, Note that the band identifier is optional if the input
 * image is single band, or if the same LUT is to be applied to all bands.
 *
 * If the input is multi-band, the same mode applies to all bands. The band numbers are 0-based.
 * If band-specific entries are being provided (i.e., the "band" prefix is being used) then at least
 * one entry for each band must be provided, otherwise remaining bands will be ignored.
 *
 * First for the "literal" remap:
 *
 *    type: ossimBandLutFilter
 *    mode: literal
 *    [band0.]entry0.in: 0
 *    [band0.]entry0.out: 1
 *    [band0.]entry1.in: 128
 *    [band0.]entry1.out: 2
 *    [band0.]entry2.in: 255
 *    [band0.]entry2.out: 3
 *
 * The above KWL will map only pixels with input values of 0, 128, and 255. All other indices will
 * map to the null value. This mode is not appropriate for floating-point input scalar types.
 * However, no checks are made so pay attention to what you put in your LUT!
 *
 * Example for arbitrary piecewise linear, a.k.a. "interpolated" mode:
 *
 *    type: ossimBandLutFilter
 *    mode: interpolated
 *    [band0.]entry0.in: 0.1
 *    [band0.]entry0.out: 255
 *    [band0.]entry1.in: 0.5
 *    [band0.]entry1.out: 128
 *    [band0.]entry2.in: 0.90
 *    [band0.]entry2.out: 1
 *
 * The above KWL is a sort of heat map where input values between 0 and 0.5 will linearly map to
 * discrete numbers 255 to 128. Then values between 0.5 to 1.0 will linearly map to
 * values 128 down to 0. Effectively this inverts the colors and performs a scalar remap to UInt8.
 * Note that any input values below 0.1 ad above 0.9 are outside of the remap range and will map to
 * the null pixel. This mode is appropriate for both integer and floating-point inputs.
 *
 * The table can be contained in a separate file from the main KWL state file. In this case, the
 * loadState can accept a file name in place of a complete table:
 *
 *    type: ossimBandLutFilter
 *    lut_file: <path to LUT KWL file>
 * 
 * The format of the LUT KWL file is just as specified above for the inline case.
 *
 * If you want an output scalar type different from the input scalar type, you will need to indicate
 * that either programmatically or via the keyword (specified in the state KWL or LUT KWL file:
 *
 * scalar_type: <ossimScalarType> (see ossimScalarTypeLut for list of enumerated strings)
 *
 **************************************************************************************************/
class OSSIM_DLL ossimBandLutFilter : public ossimImageSourceFilter
{
public:
   enum Mode  { UNKNOWN=0, LITERAL = 1, INTERPOLATED  = 2 };

   ossimBandLutFilter();

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& origin,
                                               ossim_uint32 resLevel=0);
   
   void           setMode(Mode mode) { theMode = mode; }
   Mode           getMode() const { return theMode; }

   virtual ossimScalarType getOutputScalarType() const { return theOutputScalarType; }

   /**
    *  Sets the output scalar type.
    */
   virtual void setOutputScalarType(ossimScalarType scalarType);
   virtual void setOutputScalarType(ossimString scalarType);

   /**
    * This opens the keyword list and initializes lut.
    * @param file Keyword list containing lut.
    */
   void           setLut(const ossimFilename& file);

   virtual void initialize();

   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=NULL)const;

   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=NULL);

protected:
   virtual ~ossimBandLutFilter();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   bool initializeLut(const ossimKeywordlist& kwl, const char* prefix=0);
   
   std::vector< std::map<double, double> > theLut;

   Mode   theMode;
   ossimRefPtr<ossimImageData> theTile;
   ossimFilename   theLutFile;
   ossimScalarType  theOutputScalarType;
   
TYPE_DATA
};

#endif /* #ifndef ossimBandLutFilter_HEADER */
