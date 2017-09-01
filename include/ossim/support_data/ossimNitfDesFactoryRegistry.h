//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfDesFactoryRegistry.h 22875 2014-08-27 13:52:03Z dburken $
#ifndef ossimNitfDesFactoryRegistry_HEADER
#define ossimNitfDesFactoryRegistry_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimNitfRegisteredDes.h>
#include <vector>

class ossimString;
class ossimNitfDesFactory;

class OSSIM_DLL ossimNitfDesFactoryRegistry
{
public:
   virtual ~ossimNitfDesFactoryRegistry();
   void registerFactory(ossimNitfDesFactory* aFactory);
   void unregisterFactory(ossimNitfDesFactory* aFactory);
   
   static ossimNitfDesFactoryRegistry* instance();
   
   ossimRefPtr<ossimNitfRegisteredDes> create(const ossimString &desName)const;
   bool exists(ossimNitfDesFactory* factory)const;
   
protected:
   ossimNitfDesFactoryRegistry();

private:
   /** hidden copy constructory */
   ossimNitfDesFactoryRegistry(const ossimNitfDesFactoryRegistry& factory);

   /** hidden operator= */
   const ossimNitfDesFactoryRegistry& operator=(
      const ossimNitfDesFactoryRegistry& factory);

   void initializeDefaults();
   
   std::vector<ossimNitfDesFactory*> theFactoryList;
};

#endif
