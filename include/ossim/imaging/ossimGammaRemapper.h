//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Gamma remapper.
//
//*************************************************************************
// $Id: ossimGammaRemapper.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef ossimGammaRemapper_HEADER
#define ossimGammaRemapper_HEADER

#include <ossim/imaging/ossimImageSourceFilter.h>

class ossimGammaRemapper : public ossimImageSourceFilter
{
public:
   ossimGammaRemapper();

   virtual ossimString getShortName() const;

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel=0);

   virtual void initialize();

   void setGamma(const double& gamma);
   double getGamma()const { return m_gamma; }
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0);

protected:
   virtual ~ossimGammaRemapper();

   void allocate();
   void computeLookup();
   void calculateGamma(ossimRefPtr<ossimImageData> input);
   void calculateGammaWithLookup(ossimRefPtr<ossimImageData> input);
   template<class T>
   void calculateGammaWithLookupTemplate(ossimRefPtr<ossimImageData> input, T /*dummy*/);

   ossim_float64 m_gamma;
   mutable bool m_dirtyFlag;
   ossimRefPtr<ossimImageData> m_tile;
   ossimRefPtr<ossimImageData> m_normalizedTile;
   mutable std::vector<ossim_float32> m_lookupTable;

   static const ossim_float64 MIN_GAMMA;   
   static const ossim_float64 MAX_GAMMA;   
   static const ossim_float64 DEFAULT_GAMMA;   
   TYPE_DATA
};

#endif /* #ifndef ossimGammaRemapper_HEADER */
