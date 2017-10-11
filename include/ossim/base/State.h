//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//*****************************************************************************
#ifndef ossimState_HEADER
#define ossimState_HEADER 1
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>

namespace ossim
{ 
  /**
  * This is the base for all state objects.  We started a new state
  * object for our first step will be using this interface to cache 
  * the state of ImageHandlers.  It is very expensive to keep opening
  * a resource and then parsing all the meta information that describes
  * the image.  We should be able to do this once and then allow for it
  * to be saved and reloaded witout ever having to parse the state again
  * unless the state changes
  *
  */
  class OSSIM_DLL State   
  {
  public:
    State(){}
    virtual ~State(){}
    /**
    * @return the typename of the state that uniquely identifies
    *         the name.  Usually the class name will be used
    */
    virtual const ossimString& getTypeName()const=0;

    /**
    * Allows a state to be loaded from keywordlist.
    */
    virtual bool load(const ossimKeywordlist& kwl,
                           const ossimString& prefix="")=0;

    /**
    * Allows a state to be saved to a keywordlist.
    */
    virtual bool save(ossimKeywordlist& kwl,
                           const ossimString& prefix="")const=0;
  };
}
#endif
