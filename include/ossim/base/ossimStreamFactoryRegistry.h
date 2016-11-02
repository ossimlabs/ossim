//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*******************************************************************
// $Id$

#ifndef ossimStreamFactoryRegistry_HEADER
#define ossimStreamFactoryRegistry_HEADER 1

#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimStreamFactoryBase.h>

#include <memory>
#include <vector>

class OSSIM_DLL ossimStreamFactoryRegistry : public ossimStreamFactoryBase
{
public:
   static ossimStreamFactoryRegistry* instance();
   virtual ~ossimStreamFactoryRegistry();
   
   void registerFactory(ossimStreamFactoryBase* factory);

   virtual std::shared_ptr<ossim::ifstream>
      createIFStream(const ossimFilename& file,
                     std::ios_base::openmode openMode) const;
   
   virtual ossimRefPtr<ossimIFStream> createNewIFStream(
      const ossimFilename& file, std::ios_base::openmode openMode) const;
   
protected:
   ossimStreamFactoryRegistry();
private:
   /** @brief copy constructor hidden from use */
   ossimStreamFactoryRegistry(const ossimStreamFactoryRegistry&);
   
   std::vector<ossimStreamFactoryBase*> theFactoryList;
   static ossimStreamFactoryRegistry* theInstance;
};

#endif
