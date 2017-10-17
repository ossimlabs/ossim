//----------------------------------------------------------------------------
//
// File: ossimTieMeasurementGeneratorInterface.h
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David hicks
//
// Description: OSSIM interface for tie measurement generator plugin.
//----------------------------------------------------------------------------
// $Id

#ifndef ossimTieMeasurementGeneratorInterface_HEADER
#define ossimTieMeasurementGeneratorInterface_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimIrect.h>
#include <iosfwd>


class ossimImageSource;

class ossimTieMeasurementGeneratorInterface
{
public:

   /** @brief Virtual destructor defined to squash compiler errors. */
   virtual ~ossimTieMeasurementGeneratorInterface(){}

   // Initializer
   virtual bool init(std::ostream& report = std::cout) = 0;

   virtual bool setImageList(std::vector<ossimImageSource*> src) = 0;

   // Define collection ROIs, coordinate system depends on derived class use. List of AOIs
   // must correspond to the list of images.
   virtual bool setROIs(std::vector<ossimIrect> roi) = 0;

   virtual bool isValidCollectionBox() const = 0;

   // Measurement collection
   virtual bool run() = 0;

   // Report run parameters
   virtual void summarizeRun() const = 0;
   
   // Patch grid configuration accessors
   virtual void setUseGrid(const bool useGrid) = 0;
   virtual bool getUseGrid() const = 0;
   virtual bool setGridSize(const ossimIpt& gridDimensions) = 0;
   virtual ossimIpt getGridSize() const = 0;

   // Max matches in patch accessors
   virtual bool setMaxMatches(const int& maxMatches) = 0;
   virtual int getMaxMatches() const = 0;

   // Set the feature detector
   virtual bool setFeatureDetector(const ossimString& name) = 0;
   virtual ossimString getFeatureDetector() const  = 0;
   
   // Set the descriptor-extractor
   virtual bool setDescriptorExtractor(const ossimString& name) = 0;
   virtual ossimString getDescriptorExtractor() const  = 0;
   
   // Set the matcher
   virtual bool setDescriptorMatcher(const ossimString& name) = 0;
   virtual ossimString getDescriptorMatcher() const  = 0;
   
   // Measured point access
   virtual int numMeasurements() const = 0;
   virtual ossimDpt pointIndexedAt(const ossim_uint32 imgIdx,
                                   const ossim_uint32 measIdx) = 0;

   // OpenCV drawMatches window
   virtual void closeCvWindow(const bool waitKeyPress = false) = 0;
   virtual void setShowCvWindow(const bool showCvWindow) = 0;
   virtual bool getShowCvWindow() const = 0;

};

#endif /* #ifndef ossimTieMeasurementGeneratorInterface_HEADER */
