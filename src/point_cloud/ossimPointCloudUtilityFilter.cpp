//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
//*******************************************************************
//  $Id$

#include <ossim/point_cloud/ossimPointCloudUtilityFilter.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/util/ossimPointCloudTool.h>

RTTI_DEF1(ossimPointCloudUtilityFilter, "ossimPointCloudUtilityFilter", ossimImageSourceFilter); // NOLINT


ossimPointCloudUtilityFilter::ossimPointCloudUtilityFilter( ossimPointCloudTool* pc_util)
:  m_util (pc_util)
{
}

bool ossimPointCloudUtilityFilter::getTile(ossimImageData* result, ossim_uint32 resLevel)
{
   if (!result)
      return false;

   ossimIrect irect (result->getImageRectangle());
   ossimIpt ipt;
   ossimGpt gpt;
   double h, dh;
   ossimElevManager* elevation = ossimElevManager::instance();
   ossimRefPtr<ossimImageData> highest = 0;
   ossimRefPtr<ossimImageData> lowest = 0;

   // Fetch tile from inputs as needed:
   if ((m_util->m_operation == ossimPointCloudTool::HIGHEST_DEM) ||
         (m_util->m_operation == ossimPointCloudTool::HIGHEST_LOWEST))
   {
      m_util->m_pciHandler->setCurrentEntry(ossimPointCloudImageHandler::HIGHEST);
      highest = m_util->m_pciHandler->getTile(irect, resLevel);
   }
   if ((m_util->m_operation == ossimPointCloudTool::LOWEST_DEM) ||
         (m_util->m_operation == ossimPointCloudTool::HIGHEST_LOWEST))
   {
      m_util->m_pciHandler->setCurrentEntry(ossimPointCloudImageHandler::LOWEST);
      lowest = m_util->m_pciHandler->getTile(irect, resLevel);
   }

   // Now loop over all pixels in tile and perform operations as needed:
   ossimIpt pt_l0;
   for (ipt.y=irect.ul().y; ipt.y<=irect.lr().y; ++ipt.y)
   {
      for (ipt.x=irect.ul().x; ipt.x<=irect.lr().x; ++ipt.x)
      {
         pt_l0 = ipt * (resLevel + 1);
         switch (m_util->m_operation)
         {
         case ossimPointCloudTool::HIGHEST_DEM:
            m_util->m_prodGeom->localToWorld(pt_l0, gpt);
            h = elevation->getHeightAboveEllipsoid(gpt);
            dh = highest->getPix(ipt) - h;
            break;

         case ossimPointCloudTool::HIGHEST_LOWEST:
            dh = highest->getPix(ipt) - lowest->getPix(ipt);
            break;

         default: // LOWEST_DEM
            m_util->m_prodGeom->localToWorld(pt_l0, gpt);
            h = elevation->getHeightAboveEllipsoid(gpt);
            dh = lowest->getPix(ipt) - h;
            break;

         }
         result->setValue(ipt.x, ipt.y, dh);
      }
   }

   result->validate();
   return true;
}

ossimRefPtr<ossimImageGeometry> ossimPointCloudUtilityFilter::getImageGeometry()
{
   if (!m_util.valid())
      return 0;
   return m_util->m_prodGeom;
}

