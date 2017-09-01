//----------------------------------------------------------------------------
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimH5ProjectionFactory_HEADER
#define ossimH5ProjectionFactory_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/projection/ossimProjectionFactoryBase.h>

class OSSIM_DLL ossimHdf5ProjectionFactory : public ossimProjectionFactoryBase
{
public:

   static ossimHdf5ProjectionFactory* instance();

   /**
    * @bief Takes a filename. This filename can be an image file or
    * it can also be a ossim .geom file.  It could be other
    * forms of geom files as well.  The factories job will be to
    * determine what parser to use and return a projection if
    * successful.
    */
   virtual ossimProjection* createProjection(const ossimFilename& filename,
                                             ossim_uint32 entryIdx)const;
   
   /** @brief Take a projection type name. */
   virtual ossimProjection* createProjection(const ossimString& name)const;

   /** @brief Take a keywordlist. */
   virtual ossimProjection* createProjection(const ossimKeywordlist& kwl,
                                             const char* prefix=0)const;
   
   virtual ossimObject* createObject(const ossimString& typeName)const;
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual ossimObject* createObject(const ossimKeywordlist& kwl,
                                     const char* prefix=0)const;
   
   virtual void getTypeNameList(std::vector<ossimString>& typeList)const;

   /**
    * Specifies which HDF5 internal paths to search through to locate geometry information. This
    * can be either explicit dataset names, or group names containing known datasets, such as
    * "Latitude" and "longitude". The plugins depending on HDF5 should register their geometry
    * groups here in the same order as image entries.
    */
   void addProjDataPath(const ossimString& projPath) { m_projDataPaths.push_back(projPath); }

protected:
   
   ossimHdf5ProjectionFactory();
   
   std::vector<ossimString> m_projDataPaths; // List of paths (groups or datasets) containing proj info

}; // End: class ossimH5ProjectionFactory{ ... }

#endif /* #ifndef ossimH5ProjectionFactory_HEADER */
