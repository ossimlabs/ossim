//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Table remapper class declaration.
// 
// Takes input tile, remaps it through a table, then output tile in the desired
// scalar type.
//
// Two modes one that works on native remap tables, that being of the same
// scalar type (like ossim_uint8) of the input connection, and another that
// uses a normalized remap table (more scalar independent).
//
//*************************************************************************
// $Id: ossimTableRemapper.h 22479 2013-11-12 02:18:55Z dburken $
#ifndef ossimTableRemapper_HEADER
#define ossimTableRemapper_HEADER

#include <ossim/imaging/ossimImageSourceFilter.h>

class OSSIMDLLEXPORT ossimTableRemapper : public ossimImageSourceFilter
{
public:
   enum RemapTableType
   {
      UKNOWN     = 0,
      NATIVE     = 1,
      NORMALIZED = 2
   };

   /** default constructor */
   ossimTableRemapper();

   virtual ossimScalarType getOutputScalarType() const;
    
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tile_rect,
                                               ossim_uint32 resLevel=0);

   virtual void initialize();



   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   
   
   virtual ostream& print(ostream& os) const;

   friend ostream& operator << (ostream& os,  const ossimTableRemapper& tr);

protected:
   /** virtual destructor */
   virtual ~ossimTableRemapper();

   ossimRefPtr<ossimImageData> theTile;
   ossimRefPtr<ossimImageData> theTmpTile;
   std::vector<ossim_uint8>    theTable;
   ossim_float64*  theNormBuf;
   ossim_uint32    theTableBinCount;
   ossim_uint32    theTableBandCount;
   RemapTableType  theTableType;
   ossimScalarType theInputScalarType;
   ossimScalarType theOutputScalarType;
   
   void allocate(const ossimIrect& rect);
   void destroy();

   void remapFromNativeTable(ossimRefPtr<ossimImageData>& inputTile);

   template <class T> void remapFromNativeTable(
      T dummy,
      ossimRefPtr<ossimImageData>& inputTile);

   void remapFromNormalizedTable(ossimRefPtr<ossimImageData>& inputTile);

   template <class T> void dumpTable(T dummy, ostream& os) const;

   // Do not allow copy constructor, operator=.
   ossimTableRemapper(const ossimTableRemapper& tr);
   ossimTableRemapper& operator=(const ossimTableRemapper& tr);
   
   TYPE_DATA
};

#endif  /* #ifndef ossimTableRemapper_HEADER */
