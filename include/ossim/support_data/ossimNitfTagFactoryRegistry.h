//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfTagFactoryRegistry.h 22875 2014-08-27 13:52:03Z dburken $
#ifndef ossimNitfTagFactoryRegistry_HEADER
#define ossimNitfTagFactoryRegistry_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <vector>

class ossimString;
class ossimNitfTagFactory;

class OSSIM_DLL ossimNitfTagFactoryRegistry
{
public:
   virtual ~ossimNitfTagFactoryRegistry();
   void registerFactory(ossimNitfTagFactory* aFactory);
   void unregisterFactory(ossimNitfTagFactory* aFactory);
   
   static ossimNitfTagFactoryRegistry* instance();
   
   ossimRefPtr<ossimNitfRegisteredTag> create(const ossimString &tagName)const;
   bool exists(ossimNitfTagFactory* factory)const;
   
protected:
   ossimNitfTagFactoryRegistry();

private:
   /** hidden copy constructory */
   ossimNitfTagFactoryRegistry(const ossimNitfTagFactoryRegistry& factory);

   /** hidden operator= */
   const ossimNitfTagFactoryRegistry& operator=(
      const ossimNitfTagFactoryRegistry& factory);

   void initializeDefaults();
   
   std::vector<ossimNitfTagFactory*> theFactoryList;
};

#endif
