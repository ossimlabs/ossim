/*****************************************************************************
 *                                                                            *
 *                                 O S S I M                                  *
 *            Open Source, Geospatial Image Processing Project                *
 *          License: MIT, see LICENSE at the top-level directory              *
 *                                                                            *
 ******************************************************************************/

#ifndef ossimHdf5_HEADER
#define ossimHdf5_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/plugin/ossimPluginConstants.h>
#include <ostream>
#include <iosfwd>
#include <string>
#include <vector>
#include <H5Cpp.h>

/**
 * Low-level OSSIM interface to HDF5 libraries. Catches HDF5 exceptions on common operations.
 * Note, all methods return by value as that is the way HDF5 returns objects. This seems very
 * inefficient, particularly for large datasets. Hopefully they are shallow copies.
 */
class OSSIM_DLL ossimHdf5 : public ossimReferenced
{
public:
   ossimHdf5();
   ~ossimHdf5();

   /** Opens specified HDF5 file. */
   bool open(const ossimFilename& hdf5File);

   bool isOpen() const { return (m_h5File != NULL); }

   /** Closes the file and deletes all pointers.
    * @return True if close successful. */
   bool close();

   /** Assigns the root group.
    * @return True if result valid */
   bool getRoot(H5::Group& root) const;


   /** Assigns list of groups under specified group.
    * @param recursive If true, recursively visits all subgroups
    * @return True if result valid */
   static bool getChildGroups(H5::Group group,
                              std::vector<H5::Group>& groupList,
                              bool recursive=false);

   /** Assigns list of datasets under specified group.
    * @param recursive If true, recursively visits all datasets for this group and subgroups
    * @return True if result valid */
   static bool getDatasets(H5::Group group,
                           std::vector<H5::DataSet>& datasetList,
                           bool recursive=false);

   /** Assigns list of all multi-dimensional datasets under current active group.
    * @param recursive If true, recursively visits all datasets for this group and subgroups
    * @return True if result valid */
   static bool getNdimDatasets(H5::Group group,
                               std::vector<H5::DataSet>& datasetList,
                               bool recursive=false);

   /** Assigns map of attributes (key, value) for the specified object.
    * @param objPath Either relative or absolute path in file to object.
    * @return True if result valid */
   static bool getAttributes(const H5::H5Object& obj, std::vector<H5::Attribute>& attrList);

   /** Finds a group by name. The first object with specified name (can be relative path -- a
    * naive string comparison is performed) under the specified parent group is returned.
    * @param group If null, implies root group.
    * @param recursive If true, recursively visits all subgroups.
    * @return result Set to valid dataset object if found (caller assumes ownership), else NULL. */
   H5::Group* findGroupByName(const char* group_name,
                              const H5::Group* parent_group=0,
                              bool recursive=false);

   /** Finds a dataset by name. The first object with specified name (can be relative path -- a
    * naive string comparison is performed) under the specified group is returned.
    * @param group If null, implies root group.
    * @param recursive If true, recursively visits all subgroups.
    * @return result Set to valid dataset object if found (caller assumes ownership), else NULL. */
   H5::DataSet* findDatasetByName(const char* dataset_name,
                                  const H5::Group* group=0,
                                  bool recursive=false);

   static ossimByteOrder getByteOrder( const H5::AbstractDs* obj );

   static std::string getDatatypeClassType( ossim_int32 type );

   static void getExtents( const H5::DataSet& dataset, std::vector<ossim_uint32>& extents );

   static ossimScalarType getScalarType( const H5::DataSet& dataset );
   static ossimScalarType getScalarType( const H5::DataType& datatype );

private:
   ossimFilename m_filename;
   H5::H5File* m_h5File;
};

#endif /* #ifndef ossimHdf5_HEADER */
