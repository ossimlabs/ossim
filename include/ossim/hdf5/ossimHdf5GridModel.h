//*****************************************************************************
// FILE: ossimHdf5GridModel.h
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: David Burken
//
// Copied from Mingjie Su's ossimHdfGridModel.
//
// DESCRIPTION:
//   Contains declaration of class ossimHdfGridModel. This is an
//   implementation of an interpolation sensor model.
//
//*****************************************************************************
//  $Id$

#ifndef ossimHdf5GridModel_HEADER
#define ossimHdf5GridModel_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimPolygon.h>
#include <ossim/projection/ossimCoarseGridModel.h>
#include <ossim/plugin/ossimPluginConstants.h>
#include <ossim/hdf5/ossimHdf5.h>
#include <string>


/******************************************************************************
 *
 * CLASS:  ossimHdf5GridModel
 *
 *****************************************************************************/
class OSSIM_DLL ossimHdf5GridModel : public ossimCoarseGridModel
{
public:

   /** @brief default constructor. */
   ossimHdf5GridModel();
   
   /** @brief virtual destructor */
   virtual ~ossimHdf5GridModel();

   /** Initializes from an open HDF5 file */
   bool initialize(ossimHdf5* hdf5, const ossimString& projDataPath="");

   /** Makes sure that the "type" keyword entry reflects the base class, not this one. */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix) const;

protected:

   bool initCoarseGrid(const char* datasetName, ossimDblGrid& coarseGrid);

   bool crossesDateline();

   // This polygon differs from base "theBoundGndPolygon" in that if the
   // scene crosses the dateline the longitude values are stored between
   // 0 and 360 degress as opposed to -180 to 180.
   ossimPolygon m_boundGndPolygon;

   ossimRefPtr<ossimHdf5> m_hdf5;
   ossimIpt m_imageSize;
   ossimString m_projDataPath;
   
   TYPE_DATA
};

#endif /* Matches: #ifndef ossimHdf5GridModel_HEADER */
