//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Oscar Kramer
//
//*************************************************************************
// $Id$

#ifndef ossimPointCloudUtil_HEADER
#define ossimPointCloudUtil_HEADER

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimListenerManager.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>
#include <ossim/point_cloud/ossimPointCloudUtilityFilter.h>


/**
 * Utility class for generating point-cloud-derived image products
 */
class OSSIMDLLEXPORT ossimPointCloudUtil : public ossimObject,
                                           public ossimProcessInterface,
                                           public ossimListenerManager
{
   friend class ossimPointCloudUtilityFilter;

public:
   ossimPointCloudUtil();
   virtual ~ossimPointCloudUtil();

   /**
    * Initializes from command line arguments.
    */
   bool initialize(ossimArgumentParser& ap);

   /*
    * Initializes after parameter set-methods have been called (in lieu of argument parser init)
    */
   bool initialize();

   /**
    * Returns true if successful
    */
   virtual bool execute();

   /**
    * Sets the nominal output resolution in meters
    */
   void setGSD(const double& meters_per_pixel);

   virtual ossimObject* getObject() { return this; }
   virtual const ossimObject* getObject() const  { return this; }
   virtual ossimListenerManager* getManager()  { return this; };

protected:
   void usage(ossimArgumentParser& ap);
   void addArguments(ossimArgumentParser& ap);
   bool loadPC();
   bool loadDem();

   enum Operation { HIGHEST_DEM, LOWEST_DEM, HIGHEST_LOWEST } m_operation;
   ossimRefPtr<ossimImageGeometry> m_prodGeom;
   ossimRefPtr<ossimPointCloudHandler> m_pcHandler;
   ossimRefPtr<ossimPointCloudImageHandler> m_pciHandler;
   ossimRefPtr<ossimPointCloudUtilityFilter> m_pcuFilter;
   double m_gsd;
   ossimFilename m_lutFile;
   ossimFilename m_prodFile;
   ossimFilename m_demFile;
   ossimFilename m_pcFile;

};

#endif /* ossimPointCloudUtil_HEADER */
