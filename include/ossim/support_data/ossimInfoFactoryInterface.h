//---
//
// License: MIT
//
// Author: David Burken
//
// Description: Interface for info factories.
// 
//---
// $Id$

#ifndef ossimInfoFactoryInterface_HEADER
#define ossimInfoFactoryInterface_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimInfoBase.h>

class ossimFilename;

//---
// Note: Deriving from ossimObjectFactory is being backed out as no
// ossimObjectFactory interfaces were ever called and it introduces warning in
// the header file so anyone who #includes this gets warnings...
//
// We can add back if there is a interface from ossimObjectFactory needed;
// else, please leave out.
// 
// drb 20120518
//---

/**
 * @brief Info factory.
 */
class OSSIM_DLL ossimInfoFactoryInterface
{
public:

   /** default constructor */
   ossimInfoFactoryInterface(){}

   /** virtual destructor */
   virtual ~ossimInfoFactoryInterface(){}

   /**
    * @brief Pure virtual create method.  Derived classes must implement to
    * be concrete.
    *
    * @param file Some file you want info for.
    *
    * @return ossimInfoBase* on success 0 on failure.  Caller is responsible
    * for memory.
    */
    virtual std::shared_ptr<ossimInfoBase> create(const ossimFilename& file) const = 0;

    virtual std::shared_ptr<ossimInfoBase> create(std::shared_ptr<ossim::istream>& str,
                                                  const std::string& connectionString) const = 0;

};

#endif /* End of "#ifndef ossimInfoFactoryInterface_HEADER" */
