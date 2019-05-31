//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: ossimImageSharpenFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef ossimImageSharpenFilter_HEADER
#define ossimImageSharpenFilter_HEADER
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/imaging/ossimConvolutionSource.h>
class OSSIMDLLEXPORT ossimImageSharpenFilter : public ossimImageSourceFilter
{
public:
   ossimImageSharpenFilter(ossimObject* owner=NULL);

   virtual ossimString getShortName()const;
   virtual ossimString getLongName()const;
   
   /**
    * @param percent Sets the percentage of sharpen.  A value of 0 is no sharpen and a
    *                value of 1 is fully sharpened.  The range is between and including
    *                0 and 1
    */
   void setSharpenPercent(ossim_float64 percent);

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel=0);
   
   virtual void initialize();
   
   virtual void connectInputEvent(ossimConnectionEvent &event);
   virtual void disconnectInputEvent(ossimConnectionEvent &event);
   
   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;
   
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;
protected:
   virtual ~ossimImageSharpenFilter();
   template <class T>
   void sharpenLut(T,
                   const ossimRefPtr<ossimImageData> &inputData,
                   ossimRefPtr<ossimImageData> &outputData);
   template <class T>
   void sharpenLutRemap(T,
                   const ossimRefPtr<ossimImageData> &inputData,
                   ossimRefPtr<ossimImageData> &outputData);

   void buildConvolutionLuts();
   void allocate();

   std::vector<ossim_float64>    m_posLut;
   std::vector<ossim_float64>    m_posNegLut;
   ossim_float64                 m_sharpenPercent;
   ossimRefPtr<ossimImageData>   m_tile;

   /**
    * This is a flag to indicate whether or not
    * to remap values.  This means we do not have a
    * way to directly map values to a lut and so 
    * need to get remapped. to a sampled LUT. 
    */
   bool                          m_remapValue;

   TYPE_DATA
};

#endif /* #ifndef ossimImageSharpenFilter_HEADER */
