//*******************************************************************
//
// License: MIT
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: 
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id$
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
    * @note Deprecated - Use get_desid()
    */
   virtual std::string getRegisterDesName() const;
   
   /** @return m_desid */
   virtual const std::string& get_desid() const;

   /**
    * @param desid of des.
    * @note Users should set des name as this is an unknown des.
    */
   virtual void set_desid(const std::string& desid);

   /** @return m_desver */
   virtual const std::string& get_desver() const;

   /** @return m_desver as an int */
   virtual ossim_uint32 getDesVersionNumber() const;

   /**
    * @param desid of des.
    * @note Users should set des name as this is an unknown des.
    */
   virtual void set_desver(const std::string& desver);

   /** @return desshl as an int. */
   virtual ossim_uint32 getDesSubHeaderLength() const;

   /**
    * @brief Sets the desshl.
    * Derived from 4 byte field so max is 9999.
    * @param desshl as an int
    * @return true on success, false if greater than 9999.
    */
   virtual bool setDesSubHeaderLength(ossim_uint32 length);
 
   const std::vector<ossim_int8>& getDesDataBuffer()const;
   
   virtual ossim_uint32 getDesDataLength() const;

   /**
    * @brief Set the des data length.
    *
    * @param length Length of des data portion.
    */
   virtual void setDesDataLength(ossim_uint32 length);
    
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
   // These variables are parsed outside of this class.

   // parsed by ossimNitfDesInformation
   std::string  m_desid;

   // parsed by ossimNitfDesInformation
   std::string  m_desver;
   
   // parsed by ossimNitfDesInformation and or
   // ossimNitfFileHeaderV2_1::readDataExtSegInfoRecords
   ossim_uint32 m_desshl; // parsed and set by ossimNitfDesInformation

   // parsed by ossimNitfFileHeaderV2_1::readDataExtSegInfoRecords
   ossim_uint64 m_desLength;

   /**
    * Can hold the raw des data content if not explicitly parsed by derived
    * ossimNitfRegisteredDes class into separate fields, in which case this
    * will be empty.
    */
   std::vector<ossim_int8> m_desData;
   
TYPE_DATA
};

#endif
