//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
//*******************************************************************
//  $Id: ossimImageHandlerFactoryBase.cpp 22632 2014-02-20 00:53:14Z dburken $
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
   std::istream* /* str */,
   std::streamoff /* restartPosition */,
   bool /* youOwnIt */ ) const
{
   return ossimRefPtr<ossimImageHandler>(0);
}
