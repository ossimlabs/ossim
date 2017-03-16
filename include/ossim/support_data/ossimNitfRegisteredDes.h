//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: 
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfRegisteredDes.h 22013 2012-12-19 17:37:20Z dburken $
#ifndef ossimNitfRegisteredDes_HEADER
#define ossimNitfRegisteredDes_HEADER 1

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimPropertyInterface.h>
#include <ossim/base/ossimKeywordlist.h>
#include <string>

class ossimKeywordlist;
class ossimString;

class OSSIM_DLL ossimNitfRegisteredDes : public ossimObject ,
                                         public ossimPropertyInterface
{
public:
   /** @brief default constructor */
   ossimNitfRegisteredDes();
   
   /** @brief Constructor that takes name and size. */
   ossimNitfRegisteredDes(const std::string& desName, ossim_uint32 desLength);

   /** @brief destructor */
   virtual ~ossimNitfRegisteredDes();
   
   /**
    * @brief This will return the name of the registered des for this user
    * defined header.
    *
    * @note Deprecated - Use getDesName()
    */
   virtual std::string getRegisterDesName() const;
   
   /**
    * @brief This will return the name of the registered des for this user
    * defined header.
    */
   virtual const std::string& getDesName() const;

   /**
    * @param desName Name of des.
    *
    * @note Users should set des name as this is an unknown des.
    */
   virtual void setDesName(const std::string& desName);
 
   /**
    * @brief Returns the length in bytes of the des from the CEL or REL field.
    * 
    * @note Depricated use: getDesLength()
    *
    * The entire TRE length is 11 plus this(the size of the CEL or REL field).
    *
    * @return Length of REDATA or CEDATA.
    */
   virtual ossim_uint32 getSizeInBytes()const;

  /**
    * @brief Returns the length in bytes of the des from the CEL or REL field.
    *
    * The entire TRE length is 11 plus this(the size of the CEL or REL field).
    *
    * @return Length of REDATA or CEDATA.
    */
   virtual ossim_uint32 getDesLength()const;

  /**
    * @brief Set the des length.
    *
    * @param length Length of des.
    */
   virtual void setDesLength(ossim_uint32 length);
    
   /**
    * This will allow the user defined data to parse the stream.
    */
   virtual void parseStream(std::istream& in)=0;
   virtual void writeStream(std::ostream& out)=0;

   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;

   /**
    * @brief Default interface to populate keyword list with metadata.
    * expample:  country_code: US
    * This implementation does nothing.  Derived classes should implement as
    * they see fit.
    * 
    * @param kwl Keywordlist to populate with metadata.
    *
    * @param prefix Optional prefix to put in front of key, like: "image1.".
    */
   virtual void getMetadata(ossimKeywordlist& kwl,
                            const char* prefix=0) const;

   /**
    * @brief Pure virtual print method that outputs a key/value type format
    * adding prefix to keys.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   
   virtual bool saveState(ossimKeywordlist& kwl, const ossimString& prefix)const;
   
protected:
   std::string  m_desName;
   ossim_uint32 m_desLength;
   
TYPE_DATA
};

#endif
