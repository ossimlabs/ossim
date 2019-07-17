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

#include <ossim/util/ossimTool.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>
#include <ossim/point_cloud/ossimPointCloudUtilityFilter.h>


/**
 * Utility class for generating point-cloud-derived image products
 */
class OSSIMDLLEXPORT ossimPointCloudTool : public ossimTool
{
   friend class ossimPointCloudUtilityFilter;

public:
   ossimPointCloudTool();
   ~ossimPointCloudTool();

   bool initialize(ossimArgumentParser& ap) override;
   void loadJSON(const Json::Value& json_request) override;
   void saveJSON(Json::Value& json) const override;
   virtual bool execute() override;

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:
   bool initialize();
   void setGSD(const double& meters_per_pixel);
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
