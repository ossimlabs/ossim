//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimErosionFilter_HEADER
#define ossimErosionFilter_HEADER

#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/base/ossimPolygon.h>

/*!
 * class ossimErosionFilter
 *
 * Causes null pixels to erode neighboring valid pixels. This is the opposite of dilation.
 * If a null pixel is detected inside the sampling window, the center pixel is set to null.
 */
class OSSIM_DLL ossimErosionFilter : public ossimImageSourceFilter
{
public:
   ossimErosionFilter(ossimObject* owner=NULL);
   virtual void initialize();
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& rect, ossim_uint32 resLevel=0);

   /**
    * Size of resampling kernel width & height
    */
   void setWindowSize(ossim_uint32 windowSize) { theWindowSize = windowSize; }
   ossim_uint32 getWindowSize() const          { return theWindowSize; }

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
   virtual ~ossimErosionFilter();
   
   ossimRefPtr<ossimImageData> theTile;
   ossimRefPtr<ossimImageData> theTempTile; //!> Used for recursion when recursive fill enabled
   ossim_uint32                theWindowSize;
   ossimPolygon                theValidImagePoly;

   template <class T> void doErosion(T scalarType, ossimRefPtr<ossimImageData>& inputData);

   TYPE_DATA
};

#endif
