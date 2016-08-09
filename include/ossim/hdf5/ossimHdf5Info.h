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
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/plugin/ossimPluginConstants.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/hdf5/ossimHdf5.h>
#include <H5Tpublic.h>

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

   /** virtual destructor */
   virtual ~ossimHdf5Info();

   /**
    * @brief open method.
    *
    * @param file File name to open.
    *
    * @return true on success false on error.
    */
   virtual bool open(const ossimFilename& file);
   
   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

private:
   // Methods for printing collections:
   std::ostream& printSubGroups(std::ostream& out, const H5::Group& obj, const ossimString& lm) const;
   std::ostream& printAttributes(std::ostream& out, const H5::H5Object& obj, const ossimString& lm) const;
   std::ostream& printDatasets(std::ostream& out, const H5::Group& obj, const ossimString& lm) const;

   // Methods for printing individual objects:
   std::ostream& print(std::ostream& out, const H5::Group&     obj, const ossimString& lm) const;
   std::ostream& print(std::ostream& out, const H5::DataSet&   obj, const ossimString& lm) const;
   std::ostream& print(std::ostream& out, const H5::DataType&  obj, const ossimString& lm) const;
   std::ostream& print(std::ostream& out, const H5::DataSpace& obj, const ossimString& lm) const;
   std::ostream& print(std::ostream& out, const H5::Attribute& obj, const ossimString& lm) const;

   ossimRefPtr<ossimHdf5>  m_hdf5;
};

#endif /* End of "#ifndef ossimH5Info_HEADER" */
