//*******************************************************************
//
// License: MIT
// 
// Author:  David Burken
//
// Description:
//
// Contains class declaration for ossimImageHandlerFactoryMaker.
//*******************************************************************
// $Id$

#ifndef ossimImageHandlerFactory_HEADER
#define ossimImageHandlerFactory_HEADER 1

#include <ossim/imaging/ossimImageHandlerFactoryBase.h>
#include <ossim/base/ossimString.h>

class ossimImageHandler;
class ossimFilename;
class ossimKeywordlist;

//*******************************************************************
// CLASS:  ossimImageHandlerFactory
//*******************************************************************
class OSSIMDLLEXPORT ossimImageHandlerFactory : public ossimImageHandlerFactoryBase
{
public:
   virtual ~ossimImageHandlerFactory();
   static ossimImageHandlerFactory* instance();

   /**
    * @param openOverview If true image handler will attempt to open overview.
    * default = true
    */
   virtual ossimImageHandler* open(const ossimFilename& fileName,
                                   bool openOverview=true)const;
   virtual ossimImageHandler* open(const ossimKeywordlist& kwl,
                                   const char* prefix=0)const;
   
   /**
    * @brief Open method that takes a stream.
    * @param str Open stream to image.
    * @param connectionString
    * @param openOverview If true attempt to open overview file. 
    * @return ossimImageHandler
    */
   virtual ossimRefPtr<ossimImageHandler> open(
      std::shared_ptr<ossim::istream>& str,
      const std::string& connectionString,
      bool openOverview ) const;

   /**
    * @brief Open overview that takes a file name.
    * 
    * @param file File to open.
    * 
    * @return ossimRefPtr to image handler on success or null on failure.
    */
   virtual ossimRefPtr<ossimImageHandler> openOverview(
      const ossimFilename& file ) const;
   
   virtual ossimObject* createObject(const ossimString& typeName)const;
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual ossimObject* createObject(const ossimKeywordlist& kwl,
                                     const char* prefix=0)const;
   
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<ossimString>& typeList)const;
   virtual void getSupportedExtensions(ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const;
   virtual void getImageHandlersBySuffix(ossimImageHandlerFactoryBase::ImageHandlerList& result, const ossimString& ext)const;
   virtual void getImageHandlersByMimeType(ossimImageHandlerFactoryBase::ImageHandlerList& result, const ossimString& mimeType)const;
   
protected:

   /**
    * @brief Open dot.src (ossimSrcRecord).
    *
    * Currently only opens a single image.  Can be used for split base image
    * with overviews and histograms in a separate "support" directory.
    *
    * Input record example:
    * 
    * ossim_src_record_version: 1.0
    * image0.entry: 0
    * image0.file: s3://your_bucket/data1/test/data/public/tif/ls7-ff-fusion.tif
    * image0.ovr: /data1/s3_test/ls7-ff-fusion.ovr
    * image0.hist: /data1/s3_test/ls7-ff-fusion.his
    * image0.support: /data1/s3_test
    *
    * @param str Open stream to src record.
    * @param connectionString Path to src file.
    * @param openOverview If true attempt to open overview file.
    * @return ossimRefPtr to image handler on success or null on failure.
    */
   ossimRefPtr<ossimImageHandler> openSrcRecord(std::shared_ptr<ossim::istream>& str,
                                                const std::string& connectionString,
                                                bool openOverview ) const;

   /**
    * @brief Open method that looks at extension e.g. "tif", "jpg" to select
    * class to open.
    *
    * This is to be called by standard open before just going down the list
    * of classes the brute force hard way.
    * 
    * @param fileName The filename to open.
    * 
    * @return Pointer to image handler or null if nothing opened.
    */
//   virtual ossimImageHandler* openFromExtension(
//      const ossimFilename& fileName) const;
  
   ossimImageHandlerFactory(){}
   ossimImageHandlerFactory(const ossimImageHandlerFactory&){}
   void operator = (const ossimImageHandlerFactory&){}

   static ossimImageHandlerFactory* theInstance;

TYPE_DATA
};

#endif
