//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: HDF5 Info class.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimH5Info_HEADER
#define ossimH5Info_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/plugin/ossimPluginConstants.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/hdf5/ossimHdf5.h>
#include <H5Cpp.h>

class ossimEndian;

/**
 * @brief TIFF info class.
 *
 * Encapsulates the listgeo functionality.
 */
class OSSIMDLLEXPORT ossimHdf5Info : public ossimInfoBase
{
public:
   
   /** default constructor */
   ossimHdf5Info();

   /** Accepts already opened HDF5 file object. */
   ossimHdf5Info(ossimHdf5* hdf5);

   /** virtual destructor */
   virtual ~ossimHdf5Info();

   /**
    * @param file File name to open.
    * @return true on success false on error.
    */
   virtual bool open(const ossimFilename& file);
   
   /**
    * @param out Stream to print to.
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

   virtual bool getKeywordlist(ossimKeywordlist& kwl) const;

   // Methods for printing collections. These are public to enable dumping debug information:
   std::ostream& printSubGroups (std::ostream& out, const H5::Group& obj,    const ossimString& lm=ossimString()) const;
   std::ostream& printAttributes(std::ostream& out, const H5::H5Object& obj, const ossimString& lm=ossimString()) const;
   std::ostream& printDatasets  (std::ostream& out, const H5::Group& obj,    const ossimString& lm=ossimString()) const;

   // Methods for printing individual objects:
   std::ostream& print(std::ostream& out, const H5::Group&     obj, const ossimString& lm=ossimString()) const;
   std::ostream& print(std::ostream& out, const H5::DataSet&   obj, const ossimString& lm=ossimString()) const;
   std::ostream& print(std::ostream& out, const H5::DataType&  obj, const ossimString& lm=ossimString()) const;
   std::ostream& print(std::ostream& out, const H5::DataSpace& obj, const ossimString& lm=ossimString()) const;
   std::ostream& print(std::ostream& out, const H5::Attribute& obj, const ossimString& lm=ossimString()) const;


private:
   void dumpGroup(const H5::Group& group,
                  const std::string& prefix,
                  ossim_uint32& recursedCount) const;

   void dumpDataset(const H5::DataSet& dataset, const std::string& prefix) const;

   void dumpCompoundTypeInfo(const H5::DataSet& dataset, const std::string& prefix) const;
   void dumpEnumTypeInfo(H5::EnumType datatype, const std::string& prefix) const;
   void dumpArrayTypeInfo(H5::ArrayType datatype, const std::string& prefix) const;
   void dumpNumericalTypeInfo(const H5::DataType& datatype,
                              ossimByteOrder order,
                              const std::string& prefix) const;

   void dumpNumerical(const H5::DataSet& dataset,
                      const char* dataPtr,
                      const std::string& prefix) const;

   void dumpStr(const H5::DataSet& dataset,
                const H5::StrType& dataType,
                const char* dataPtr,
                const std::string& prefix) const;

   void dumpAttribute(const H5::Attribute& attr, const std::string& prefix) const;

   bool getGroupAttributeValue(const std::string& group,
                               const std::string& key,
                               std::string& value ) const;

   bool getDatasetAttributeValue(const std::string& objectName,
                                const std::string& key,
                                std::string& value ) const;

   ossimString getObjectName(const ossimString& fullPathName) const;

   ossimRefPtr<ossimHdf5>  m_hdf5;
   mutable ossimKeywordlist m_kwl;
};

#endif
