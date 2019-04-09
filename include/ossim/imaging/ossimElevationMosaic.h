//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: 
//
//*************************************************************************
// $Id: ossimElevationMosaic.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef ossimElevationMosaic_HEADER
#define ossimElevationMosaic_HEADER
#include <vector>
#include <ossim/imaging/ossimImageCombiner.h>


/**
 * An image mosaic is a simple combiner that will
 * just do a simple mosaic.  It just checks NULL pix values until it finds a
 * pixel that is not empty and copies it out to the output.  The list will
 * have same size tiles and have the same number of bands.
 */
class OSSIMDLLEXPORT ossimElevationMosaic : public ossimImageCombiner
{
public:
   ossimElevationMosaic();
   ossimElevationMosaic(ossimConnectableObject::ConnectableObjectList& inputSources);
   
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& origin,
                                               ossim_uint32 resLevel=0);
   
   virtual void initialize();
   
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

protected:
   virtual ~ossimElevationMosaic();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
   ossimRefPtr<ossimImageData> theTile;

   template <class T> ossimRefPtr<ossimImageData> combine(
      T, // dummy template variable not used
      const ossimIrect& tileRect,
      ossim_uint32 resLevel=0);
   template <class T> ossimRefPtr<ossimImageData> combineNorm(
      T, // dummy template variable not used
      const ossimIrect& tileRect,
      ossim_uint32 resLevel=0);

TYPE_DATA
};

#endif /* #ifndef ossimElevationMosaic_HEADER */
