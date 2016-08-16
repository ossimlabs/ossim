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
class OSSIMDLLEXPORT ossimHdf5 : public ossimReferenced
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
   bool getChildGroups(H5::Group group,
                       std::vector<H5::Group>& groupList,
                       bool recursive=false) const;

   /** Assigns list of datasets under current active group.
    * @param recursive If true, recursively visits all datasets for this group and subgroups
    * @return True if result valid */
   bool getDatasets(H5::Group group,
                    std::vector<H5::DataSet>& datasetList,
                    bool recursive=false) const;

   /** Assigns list of all multi-dimensional datasets under current active group.
    * @param recursive If true, recursively visits all datasets for this group and subgroups
    * @return True if result valid */
   bool getNdimDatasets(H5::Group group,
                        std::vector<H5::DataSet>& datasetList,
                        bool recursive=false) const;

   /** Assigns map of attributes (key, value) for the specified object.
    * @param objPath Either relative or absolute path in file to object.
    * @return True if result valid */
   bool getAttributes(const H5::H5Object* obj, std::vector<H5::Attribute>& attrList) const;

   /** Finds a dataset by name. The first object with specified name (can be relative path -- a
    * naive string comparison is performed) under the specified group is returned.
    * @param group If null, implies root group.
    * @param recursive If true, recursively visits all subgroups.
    * @return result Set to valid dataset object if found (caller assumes ownership), else NULL. */
   H5::DataSet* findDatasetByName(const char* dataset_name,
                                  const H5::Group* group=0,
                                  bool recursive=false);

private:
   ossimFilename m_filename;
   H5::H5File* m_h5File;
};

#endif /* #ifndef ossimHdf5_HEADER */
