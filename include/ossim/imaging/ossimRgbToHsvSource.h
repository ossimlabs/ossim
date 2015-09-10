//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: ossimRgbToHsvSource.h 22724 2014-04-10 16:27:46Z gpotts $
#ifndef ossimRgbToHsvSource_HEADER
#define ossimRgbToHsvSource_HEADER
#include <ossim/imaging/ossimImageSourceFilter.h>

class ossimRgbToHsvSource : public ossimImageSourceFilter
{
public:
   ossimRgbToHsvSource();
   ossimRgbToHsvSource(ossimImageSource* inputSource);
   
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel=0);
   
   virtual ossimScalarType getOutputScalarType() const;
   virtual ossim_uint32    getNumberOfOutputBands()const;
   virtual double getNullPixelValue()const;
   virtual double getMinPixelValue(ossim_uint32 band=0)const;
   virtual double getMaxPixelValue(ossim_uint32 band=0)const;

   virtual void initialize();
       
protected:
   virtual ~ossimRgbToHsvSource();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
   ossimRefPtr<ossimImageData> theBlankTile;
   ossimRefPtr<ossimImageData> theTile;

TYPE_DATA
};

#endif
