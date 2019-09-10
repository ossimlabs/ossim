//**************************************************************************************************
//
//     OSSIM Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimBandMergeTool.h>
#include <ossim/util/ossimHillshadeTool.h>
#include <ossim/util/ossimHlzTool.h>
#include <ossim/util/ossimInfo.h>
#include <ossim/util/ossimOrthoTool.h>
#include <ossim/util/ossimShorelineTool.h>
#include <ossim/util/ossimSlopeTool.h>
#include <ossim/util/ossimToolFactory.h>
#include <ossim/util/ossimToolRegistry.h>
#include <ossim/util/ossimVerticesFinderTool.h>
#include <ossim/util/ossimViewshedTool.h>
#include <ossim/util/ossimSubImageTool.h>
#include <ossim/util/ossimPointCloudTool.h>
#if OSSIM_HAS_HDF5
#include <ossim/hdf5/ossimHdf5Tool.h>

#endif

using namespace std;

ossimToolFactory* ossimToolFactory::s_Instance = 0;

ossimToolFactory* ossimToolFactory::instance()
{
   if (!s_Instance)
      s_Instance = new ossimToolFactory;
   return s_Instance;
}

ossimToolFactory::ossimToolFactory()
{
}

ossimToolFactory::~ossimToolFactory()
{
   ossimToolRegistry::instance()->unregisterFactory(this);
}

ossimTool* ossimToolFactory::createTool(const std::string& argName) const
{
   ossimString utilName (argName);
   utilName.downcase();

   if ((utilName == "info") || (argName == "ossimInfo"))
      return new ossimInfo;

   if ((utilName == "hillshade") || (argName == "ossimHillshadeTool"))
      return new ossimHillshadeTool;

   if ((utilName == "viewshed") || (argName == "ossimViewshedTool"))
      return new ossimViewshedTool;

   if ((utilName == "slope") || (argName == "ossimSlopeTool"))
      return new ossimSlopeTool;

   if ((utilName == "hlz") || (argName == "ossimHlzTool"))
      return new ossimHlzTool;

   if ((utilName == "shoreline") || (argName == "ossimShorelineTool"))
      return new ossimShorelineTool;

   if ((utilName == "ortho") || (argName == "ossimOrthoTool"))
      return new ossimOrthoTool;

   if ((utilName == "vertices") || (argName == "ossimVerticesFinderTool"))
      return new ossimVerticesFinderTool;

   if ((utilName == "bandmerge") || (argName == "ossimBandMergeTool"))
      return new ossimBandMergeTool;

   if ((utilName == "subimage") || (argName == "ossimSubImageTool"))
      return new ossimSubImageTool;

   if ((utilName == "pointcloud") || (argName == "ossimPointCloudTool"))
      return new ossimPointCloudTool;

#if OSSIM_HAS_HDF5
   if ((utilName == "hdf5") || (argName == "ossimHdf5Tool"))
      return new ossimHdf5Tool;
#endif

   return 0;
}

void ossimToolFactory::getCapabilities(std::map<std::string, std::string>& capabilities) const
{
   capabilities.insert(pair<string, string>("info", ossimInfo::DESCRIPTION));
   capabilities.insert(pair<string, string>("hillshade", ossimHillshadeTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("viewshed", ossimViewshedTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("slope", ossimSlopeTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("hlz", ossimHlzTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("shoreline", ossimShorelineTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("ortho", ossimOrthoTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("vertices", ossimVerticesFinderTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("bandmerge", ossimBandMergeTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("subimage", ossimSubImageTool::DESCRIPTION));
   capabilities.insert(pair<string, string>("pointcloud", ossimPointCloudTool::DESCRIPTION));
#if OSSIM_HAS_HDF5
   capabilities.insert(pair<string, string>("hdf5", ossimHdf5Tool::DESCRIPTION));
#endif
}

std::map<std::string, std::string> ossimToolFactory::getCapabilities() const
{
   std::map<std::string, std::string> result;
   getCapabilities(result);
   return result;
}

void ossimToolFactory::getTypeNameList(vector<ossimString>& typeList) const
{
   typeList.push_back("ossimInfo");
   typeList.push_back("ossimHillshadeUtil");
   typeList.push_back("ossimViewshedUtil");
   typeList.push_back("ossimSlopeUtil");
   typeList.push_back("ossimHLZUtil");
   typeList.push_back("ossimShorelineUtil");
   typeList.push_back("ossimOrthoUtil");
   typeList.push_back("ossimVerticesFinderUtil");
   typeList.push_back("ossimBandMergeUtil");
   typeList.push_back("ossimSubImageTool");
   typeList.push_back("ossimPointCloudTool");
#if OSSIM_HAS_HDF5
   typeList.push_back("ossimHdf5Tool");
#endif
}

