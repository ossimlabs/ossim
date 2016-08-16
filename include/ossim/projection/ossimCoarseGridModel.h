//*****************************************************************************
// FILE: ossimCoarseGridModel.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION:
//   Contains declaration of class ossimCoarseGridModel. This is an
//   implementation of an interpolation sensor model.
//
//*****************************************************************************
//  $Id: ossimCoarseGridModel.h 22825 2014-07-07 23:14:52Z dburken $

#ifndef ossimCoarseGridModel_HEADER
#define ossimCoarseGridModel_HEADER 1

#include <ossim/projection/ossimSensorModel.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimDblGrid.h>
#include <ossim/base/ossimFilename.h>

class ossimImageGeometry;

/******************************************************************************
 *
 * CLASS:  ossimCoarseGridModel
 *
 *****************************************************************************/
class OSSIMDLLEXPORT ossimCoarseGridModel : public ossimSensorModel
{
public:
   ossimCoarseGridModel();
   ossimCoarseGridModel(const ossimCoarseGridModel& copy_this);

   /** Accepts name of geometry file. This can be either MET ECG geom file, or
    * OSSIM keywordlist geometry file.*/
   ossimCoarseGridModel(const ossimFilename& geom_file);

   /** Accepts OSSIM keywordlist geometry file. */
   ossimCoarseGridModel(const ossimKeywordlist& geom_kwl);
   
   ~ossimCoarseGridModel();

   /** This method will build a grid from any projector. The accuracy of the grid can be
    * controlled by the static method setInterpolationError().
    * @param imageBounds Must be the image space bounds for the projection.
    * @param proj The projector that will be used to approximate a bilinear grid over. */
   virtual void buildGrid(const ossimDrect& imageBounds,
                          ossimProjection* proj,
                          double heightDelta=500.0,
                          bool enableHeightFlag=false,
                          bool makeAdjustableFlag=true);
   virtual void buildGrid(const ossimDrect& imageBounds,
                          ossimImageGeometry* geom,
                          double heightDelta=500.0,
                          bool enableHeightFlag=false,
                          bool makeAdjustableFlag=true);
   
   /** This is used when building a grid from a projector. You can set the interpolation error.
    * The default is subpixel accuracy (within .1 of a pixel). */
   static void setInterpolationError(double error=.1);
   static void setMinGridSpacing(ossim_int32 minSpacing = 100);

   /** Extends base-class implementation. Dumps contents of object to ostream. */
   virtual std::ostream& print(std::ostream& out) const;
   
   /** Fulfills ossimObject base-class pure virtuals. Saves geometry KWL files.
    * @return Returns true if successful. */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0) const;

   /** Fulfills ossimObject base-class pure virtuals. Loads geometry KWL files.
    * @return Returns true if successful. */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

   /** Writes a template of geometry keywords processed by loadState and
    * saveState to output stream. */
   static void writeGeomTemplate(ostream& os);

   /** Returns pointer to a new instance, copy of this. */
   virtual ossimObject* dup() const { return new ossimCoarseGridModel(*this); }
   
   /** Saves the coarse grid to the specified file.
    * @return Returns true if successful. */
   bool saveCoarseGrid(const ossimFilename& cgFileName) const;

   /** Loads the coarse grid from the specified file.
    * @return Returns true if successful. */
   bool loadCoarseGrid(const ossimFilename& cgFileName);

   virtual void imagingRay(const ossimDpt& image_point, ossimEcefRay& image_ray) const;

   
   virtual void lineSampleToWorld(const ossimDpt& image_point, ossimGpt& gpt) const;

   /** This is the virtual that performs the actual work of projecting the image point
    * to the earth at some specified elevation. */
   virtual void lineSampleHeightToWorld(const ossimDpt& image_point,
                                        const double&   heightEllipsoid,
                                        ossimGpt&       world_pt) const;

   virtual void initAdjustableParameters();

   /*!
    * ossimOptimizableProjection
    */
   inline virtual bool useForward()const {return false;} //!image to ground faster

   /** Overrides base ossimSensorModel::isAffectedByElevation method.
    * @return true if height enabled, false if not. */
   virtual bool isAffectedByElevation() const;
   
protected:

   /** Deletes existing allocated memory and reallocates
    * new space. This may happen if a new grid is loaded over an existing one. */
   void reallocateGrid(const ossimIpt& size);
   
   //! Initializes base class data members after grids have been assigned.
   void initializeModelParams(ossimIrect irect);

   //! Implements its own extrapolation since this can be handled by ossimDblGrid.
   virtual ossimGpt extrapolate (const ossimDpt& imgPt, const double& height=ossim::nan()) const;

   mutable ossimFilename theGridFilename;
   ossimDblGrid  theLatGrid;         // degrees
   ossimDblGrid  theLonGrid;         // degrees
   ossimDblGrid  theDlatDhGrid;      // degrees/meter
   ossimDblGrid  theDlonDhGrid;      // degrees/meter
   ossimDblGrid* theDlatDparamGrid;  // degrees/(units-of-param)
   ossimDblGrid* theDlonDparamGrid;  // degrees/(units-of-param)

   static double       theInterpolationError;
   static ossim_int32  theMinGridSpacing;
   ossimAdjustmentInfo theInitialAdjustment;
   bool                theHeightEnabledFlag;
   
   TYPE_DATA
};

#endif
