//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Generic geoid source which uses an image handler for reading the grid.
// 
//----------------------------------------------------------------------------
#ifndef ossimGeoidImage_HEADER
#define ossimGeoidImage_HEADER 1
   
#include <ossim/base/ossimGeoid.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>

class ossimFilename;

/**
 * @class ossimGeoidImage
 *
 * Generic geoid source which uses a image handler for reading the grid.
 *
 * The settable keyword "geoid.type", e.g. "egm2008" is used to attach a given geoid
 * to an elevation source.
 *
 * The keyword "type" is fixed for this object as "geoid_image".
 */
class OSSIM_DLL ossimGeoidImage : public ossimGeoid
{

public:
   
   /** @brief default constructor */
   ossimGeoidImage();

   /** @brief destructor */
   virtual ~ossimGeoidImage();

   /**
    * @brief open method
    * @param file This can be any image that ossim can open and get geometry
    * info for.
    * @param byteOrder Not used by this object.
    * @return true on success, false on erro.
    */
   virtual bool open( const ossimFilename& file,
                      ossimByteOrder byteOrder=OSSIM_BIG_ENDIAN );

   /**
    * @return The short name which represents the "geoid.type" keyword which is
    * used by ossimGeoidFactory.
    */
   virtual ossimString getShortName() const;
   
   /** @brief Sets the geoid type name string. */
   void setShortName( const std::string& geoidTypeName );

   /**
    * @brief Gets the memory map flag.
    * @return true if geoid image is memory mapped; else, false.
    */
   bool getMemoryMapFlag() const;

   /**
    * @brief Set the memory map flag.
    */
   void setMemoryMapFlag( bool flag );
   
   /**
    * Method to save the state of the object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0) const;
   
   /**
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   
   /**
    *  @return The offset from the ellipsoid to the geoid or ossim::nan()
    *  (IEEE NAN) if grid does not contain the point.
    */
   virtual double offsetFromEllipsoid(const ossimGpt& gpt);

   bool getEnableFlag() const;

   void setEnableFlag(bool flag);

protected:

   template <class T>
   double offsetFromEllipsoidTemplate(T dummy, const ossimGpt& gpt);

   ossimRefPtr<ossimImageGeometry> m_geom;
   ossimRefPtr<ossimImageHandler>  m_handler;
   ossimRefPtr<ossimImageData>     m_cacheTile;
   std::string                     m_connectionString;
   ossimString                     m_geoidTypeName;
   bool                            m_memoryMapFlag;
   bool                            m_enabledFlag;
   ossimIrect                      m_imageRect;
   ossimScalarType                 m_scalarType;
};

#endif /* #define ossimGeoidImage_HEADER 1 */
