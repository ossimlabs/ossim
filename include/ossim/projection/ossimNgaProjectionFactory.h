//*******************************************************************
// Copyright (C) 2014 RadiantBlue, Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
//*******************************************************************
//  $Id$
#ifndef ossimNgaProjectionFactory_HEADER
#define ossimNgaProjectionFactory_HEADER

#include <ossim/projection/ossimProjectionFactoryBase.h>

class OSSIM_DLL ossimNgaProjectionFactory : public ossimProjectionFactoryBase
{
public:
   
   static ossimNgaProjectionFactory* instance();

   /**
    * takes a filename. This filename can be an image file or
    * it can also be a ossim .geom file.  It could be other
    * forms of geom files as well.  The factories job will be to
    * determine what parser to use and return a projection if
    * successful.
    */
   virtual ossimProjection* createProjection(const ossimFilename& filename,
                                             ossim_uint32 entryIdx)const;

   /**
    * Take a projection type name.
    */
   virtual ossimProjection* createProjection(const ossimString& name)const;

   /**
    * Take a keywordlist.
    */
   virtual ossimProjection* createProjection(const ossimKeywordlist& kwl,
                                             const char* prefix)const;
   
   virtual ossimObject* createObject(const ossimString& typeName)const;

   /*!
    * Creates and object given a keyword list.
    */
   virtual ossimObject* createObject(const ossimKeywordlist& kwl,
                                     const char* prefix=0) const;

   /*!
     * This should return the type name of all objects in all factories.
     * This is the name used to construct the objects dynamially and this
     * name must be unique.
     */
    virtual void getTypeNameList(std::vector<ossimString>& typeList)const;

protected:
    //! Constructor loads all SRS CSV files specified in the ossim prefs. Protected as part of
    //! singleton implementation.
    ossimNgaProjectionFactory() {}

TYPE_DATA
};

#endif
