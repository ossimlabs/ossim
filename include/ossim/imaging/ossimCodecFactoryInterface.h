//----------------------------------------------------------------------------
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Interface for codec(encoder/decoder) factories.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimCodecFactoryInterface_HEADER
#define ossimCodecFactoryInterface_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageData.h>
#include <vector>

class ossimObject;
class ossimCodecBase;
/**
 * @brief Codec factory interface.
 */
class OSSIM_DLL ossimCodecFactoryInterface // : public ossimObjectFactory
{
public:

   /** default constructor */
   ossimCodecFactoryInterface(){}

   /** virtual destructor */
   virtual ~ossimCodecFactoryInterface(){}
   
   /**
   * This is a bridge to the generic factory list interface that trys to call createObject
   * 
   * We will bridge it in this interface to just call createCodec.  These are not pure virtual.
   *
   */
   virtual ossimObject* createObject(const ossimString& type)const;

  /**
   * This is a bridge to the generic factory list interface that trys to call createObject
   * 
   * We will bridge it in this interface to just call createCodec.  These are not pure virtual.
   *
   */
  virtual ossimObject* createObject(const ossimKeywordlist& kwl, const char* prefix=0)const;
   

   
   virtual ossimCodecBase* createCodec(const ossimString& type)const=0;
   virtual ossimCodecBase* createCodec(const ossimKeywordlist& kwl, const char* prefix=0)const=0;


   virtual void getTypeNameList(std::vector<ossimString>& typeNames)const=0;

   /**
    * @brief Pure virtual decode method. Derived classes must implement to
    * be concrete.
    *
    * @param in Input data to decode.
    * 
    * @param out Output tile.
    *
    * @return true on success, false on failure.
    */
//   virtual bool decode( const std::vector<ossim_uint8>& in,
//                        ossimRefPtr<ossimImageData>& out ) const = 0;

   /**
    * @brief Pure virtual encode method. Derived classes must implement to
    * be concrete.
    *
    * @param in Input data to encode.
    * 
    * @param out Encoded output data.
    *
    * @return true on success, false on failure.
    */

//   virtual bool encode( const ossimKeywordlist& options,
//                        const ossimRefPtr<ossimImageData>& in,
//                        std::vector<ossim_uint8>& out ) const = 0;
};

#endif /* End of "#ifndef ossimCodecFactoryInterface_HEADER" */
