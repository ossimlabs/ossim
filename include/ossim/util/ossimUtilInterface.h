//---
//
// License: MIT
// 
// Author: David Burken
// 
// Description: Interface for utility class with initialize and execute.
// Here for plugin objects, so they can cast to this interface and call
// methods.
// 
// 
//---
// $Id$

#ifndef ossimUtilInterface_HEADER
#define ossimUtilInterface_HEADER 1

#include <ossim/base/ossimConstants.h>

class ossimArgumentParser;
class ossimKeywordlist;

/**
 * @class ossimUtilInterface
 *
 * Interface for utility class with initialize and execute.
 * Here for plugin objects, so they can cast to this interface and call
 * methods.
 */
class OSSIM_DLL ossimUtilInterface
{
public:
   
   /**
    * @brief Initial method to be ran prior to execute.
    * @param ap Arg parser to initialize from.
    * @return true on success, false on error or usage.
    * @note Can throw ossimException on error.
    */
   virtual bool initialize(ossimArgumentParser& ap) = 0;

   /**
    * @brief Initialize method to be ran prior to execute.
    * @param Keyword list containing all options.
    * @note Can throw ossimException on error.
    */
   virtual void initialize(const ossimKeywordlist& kwl) = 0;
   
   /**
    * @brief Execute method.
    * @return true on success, false on error.
    * @note Can throw an ossimException on error.
    */
   virtual bool execute() = 0;
};

#endif /* End of "#ifndef ossimUtilInterface_H" */
