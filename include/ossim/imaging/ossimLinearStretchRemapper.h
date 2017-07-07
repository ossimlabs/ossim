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
// $Id: ossimLinearRemapper.h 23616 2015-11-11 19:50:29Z dburken $
#ifndef ossimLinearRemapper_HEADER
#define ossimLinearRemapper_HEADER

#include <ossim/base/ossimRgbVector.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <vector>

class ossimImageData;

/***************************************************************************************************
 *
 * This class provides a linear remap between a min and a max value. Default is to do a linear
 * stretch between the input connection's min and max, but the user can also define specific min
 * and max. In the latter case, the input pixels outside the range will be clamped.
 *
 * If non-default min and max values are to be used, the min and max values must be set for all
 * bands, otherwise the defaults will be used for all bands.
 *
 * The output scalar type is normalized double.
 *
 **************************************************************************************************/
class OSSIM_DLL ossimLinearStretchRemapper : public ossimImageSourceFilter
{
public:

   ossimLinearStretchRemapper();
   virtual ~ossimLinearStretchRemapper();

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& origin,
                                               ossim_uint32 resLevel=0);
   
   void           setMinPixelValue(double value, ossim_uint32 band=0);
   void           setMaxPixelValue(double value, ossim_uint32 band=0);
   
   virtual double getMinPixelValue(ossim_uint32 band=0)const;
   virtual double getMaxPixelValue(ossim_uint32 band=0)const;

   virtual void initialize();

   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=NULL)const;

   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=NULL);

   virtual ossimScalarType getOutputScalarType() const { return OSSIM_NORMALIZED_DOUBLE; }

protected:

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();

   std::vector<double> m_minValues;
   std::vector<double> m_maxValues;
   ossimRefPtr<ossimImageData> m_tile;
   
TYPE_DATA
};

#endif /* #ifndef ossimLinearRemapper_HEADER */
