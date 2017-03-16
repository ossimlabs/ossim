//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  RP
//
// Description:
//
// 
//*******************************************************************
//  $Id$
#ifndef ossimElevRemapper_HEADER
#define ossimElevRemapper_HEADER
#include <ossim/imaging/ossimImageSourceFilter.h>

class OSSIM_DLL ossimElevRemapper : public ossimImageSourceFilter
{
public:
   static const char REMAP_MODE_KW[];

   enum ReplacementType
   {
      ReplacementType_ELLIPSOID = 0,
      ReplacementType_GEOID = 1
   };
   
   ossimElevRemapper();
   virtual void initialize();
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tile_rect,
                                               ossim_uint32 resLevel=0);
   
protected:
   virtual ~ossimElevRemapper();
   template <class T> void elevRemap(T dummy,
                                      ossimImageData* inpuTile,
                                      ossim_uint32 resLevel);
   
   ReplacementType m_replacementType;
   bool saveState(ossimKeywordlist& kwl, const char* prefix)const;
   bool loadState(const ossimKeywordlist& kwl, const char* prefix);
   ossimRefPtr<ossimImageGeometry> m_imageGeometry;

TYPE_DATA   
};
#endif
