//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
//*******************************************************************
//  $Id: ossimQuickbirdNitfTileSource.h 22836 2014-07-25 15:13:47Z dburken $
#ifndef ossimQuickbirdNitfTileSource_HEADER
#define ossimQuickbirdNitfTileSource_HEADER
#include <ossim/imaging/ossimNitfTileSource.h>
#include <ossim/base/ossim2dTo2dTransform.h>

class ossimQuickbirdNitfTileSource : public ossimNitfTileSource
{
public:
   virtual bool open();

   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry();

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
   ossimRefPtr<ossim2dTo2dTransform> m_transform;
TYPE_DATA   
};
#endif
