//---
//
// License: MIT
//
// Author: David Burken
//
// Description: Class declaration of Registry (singleton) for info factories.
//
//---
// $Id$

#ifndef ossimInfoFactoryRegistry_HEADER
#define ossimInfoFactoryRegistry_HEADER 1

#include <ossim/base/ossimConstants.h> /* for OSSIM_DLL macro */
#include <ossim/base/ossimIosFwd.h> 
#include <ossim/support_data/ossimInfoBase.h> 
#include <memory>
#include <vector>
#include <mutex>

// Forward class declarations.
class ossimInfoFactoryInterface;
class ossimInfoBase;
class ossimFilename;

class OSSIM_DLL ossimInfoFactoryRegistry
{
public:
   
   /** destructor */
   ~ossimInfoFactoryRegistry();

   /**
    * instance method
    *
    * @return Pointer to the instance of the registry.
    */
   static ossimInfoFactoryRegistry* instance();

   /**
    * @brief Method to add factory to registry.
    * @param factory Factory to register.
    */
   void registerFactory(ossimInfoFactoryInterface* factory);

   /**
    * @brief Adds factory to the front of the registry.
    * @param factory Factory to register.
    */
   void registerFactoryToFront( ossimInfoFactoryInterface* factory );
                               
   /**
    * Method to remove a factory from the registry.  Used by plugins when they
    * are unloaded.
    * @param factory Factory to remove.
    */
   void unregisterFactory(ossimInfoFactoryInterface* factory);

   /**
    * @brief Create method.
    *
    * @param file Some file you want info for.
    *
    * @return ossimInfoBase* on success 0 on failure.  Caller is responsible
    * for memory.
    */
   std::shared_ptr<ossimInfoBase> create(const ossimFilename& file) const;

   /**
    * @brief Create method.
    *
    * @param file Some file you want info for.
    *
    * @return ossimInfoBase* on success 0 on failure.  Caller is responsible
    * for memory.
    */
   std::shared_ptr<ossimInfoBase> create( std::shared_ptr<ossim::istream>& str,
                                          const std::string& connectionString) const;

protected:

   /**
    * @brief Hidden from use default constructor.
    * @note Adds ossimInfoFactory to registry.
    */
   ossimInfoFactoryRegistry();

   /** @brief hidden from use copy constructor */
   ossimInfoFactoryRegistry(const ossimInfoFactoryRegistry& obj);

   /** @brief hidden from use assignment operator */
   const ossimInfoFactoryRegistry& operator=(
      const ossimInfoFactoryRegistry& rhs);
   
   std::vector<ossimInfoFactoryInterface*> m_factoryList;

   std::mutex m_mutex;

   static ossimInfoFactoryRegistry* m_instance;
};

#endif /* #ifndef ossimInfoFactoryRegistry_HEADER */
