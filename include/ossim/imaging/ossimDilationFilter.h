//**************************************************************************************************
//
//                                            OSSIM
//
//       License: MIT. See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
//  $Id$
#ifndef ossimDilationFilter_HEADER
#define ossimDilationFilter_HEADER

#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/base/ossimPolygon.h>

/*!
 * class ossimDilationFilter
 *
 * Allows you to change between a median or mean filter.  You can
 * also specify a window size which the median or mean is computed and
 * the center pixel is replaced.
 *
 */
class OSSIM_DLL ossimDilationFilter : public ossimImageSourceFilter
{
public:
   ossimDilationFilter(ossimObject* owner=NULL);
   virtual void initialize();
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& rect, ossim_uint32 resLevel=0);

   /**
    * Size of resampling kernel width & height
    */
   void setWindowSize(ossim_uint32 windowSize) { theWindowSize = windowSize; }
   ossim_uint32 getWindowSize() const          { return theWindowSize; }

   /**
    * Set if getTile is to be called recursively until all pixels have been assigned non-null.
    * This is only valid for "fill null" modes.
    */
   void setRecursiveFlag(bool flag=true) { theRecursiveFlag = flag; }
   bool getRecursiveFlag() const         { return theRecursiveFlag; }

   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;
   
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

protected:
   virtual ~ossimDilationFilter();
   
   ossimRefPtr<ossimImageData> theTile;
   ossimRefPtr<ossimImageData> theTempTile; //!> Used for recursion when recursive fill enabled
   ossim_uint32                theWindowSize;
   bool                        theRecursiveFlag;
   ossimPolygon                theValidImagePoly;
   bool                        theNullFoundFlag;

   template <class T> void doDilation(T scalarType, ossimRefPtr<ossimImageData>& inputData);

   TYPE_DATA
};

#endif
