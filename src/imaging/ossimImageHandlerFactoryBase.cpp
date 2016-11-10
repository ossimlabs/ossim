//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
//*******************************************************************
// $Id$

#include <ossim/imaging/ossimImageHandlerFactoryBase.h>

RTTI_DEF1(ossimImageHandlerFactoryBase, "ossimImageHandlerFactoryBase", ossimObjectFactory);

void ossimImageHandlerFactoryBase::getImageHandlersBySuffix(ImageHandlerList& /*result*/,
                                                            const ossimString& /*ext*/)const
{
}

void ossimImageHandlerFactoryBase::getImageHandlersByMimeType(ImageHandlerList& /*result*/,
                                                              const ossimString& /*mimeType*/)const
{
}

ossimRefPtr<ossimImageHandler> ossimImageHandlerFactoryBase::openOverview(
   const ossimFilename& /* file */ ) const
{
   return ossimRefPtr<ossimImageHandler>(0);
}

ossimRefPtr<ossimImageHandler> ossimImageHandlerFactoryBase::open(
   ossim::istream* /* str */,
   std::streamoff /* restartPosition */,
   bool /* youOwnIt */ ) const
{
   return ossimRefPtr<ossimImageHandler>(0);
}


ossimRefPtr<ossimImageHandler> ossimImageHandlerFactoryBase::open(
   std::shared_ptr<ossim::istream>& /*str */,
   const ossimString& /* connectionString */,
   bool /* openOverview */ ) const
{
   return ossimRefPtr<ossimImageHandler>(0);
}
ossimRefPtr<ossimImageHandler> ossimImageHandlerFactoryBase::openOverview(
   std::shared_ptr<ossim::istream>& /* str */,
   const ossimString& /* connectionString */ ) const
{
   return ossimRefPtr<ossimImageHandler>(0);
}
