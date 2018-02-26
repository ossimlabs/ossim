//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/reg/Image.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimString.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>

using namespace std;

namespace ossim
{

Image::Image(const std::string& imageId,
             const std::string& filename,
             const std::string& modelName,
             unsigned int entryIndex,
             unsigned int band)
:  m_imageId (imageId),
   m_filename (filename),
   m_entryIndex (entryIndex),
   m_activeBand (band),
   m_modelName (modelName)
{

}

Image::Image(const Json::Value& json_node)
:  m_entryIndex (0),
   m_activeBand (1)
{
   loadJSON(json_node);
}

Image::~Image()
{
   //m_handler.reset();
}

void Image::getAvailableModels(std::vector< pair<string, string> >& availableModels) const
{
   ossimProjection* proj = ossimProjectionFactoryRegistry::instance()->
         createProjection(m_filename, m_entryIndex);
   if (proj)
   {
      availableModels.push_back(pair<string, string>("OSSIM", proj->getLongName().string()));
      delete proj;
   }
}

void Image::loadJSON(const Json::Value& json_node)
{
   ostringstream xmsg;
   xmsg<<__FILE__<<": loadJSON(JSON) -- ";

   // Parse JSON. Filename is required:
   if (json_node.isMember("filename"))
   {
      ossimFilename imageFile = json_node["filename"].asString();
      m_filename = imageFile.expand(); // allow embedded environment variables
   }
   else
   {
      xmsg<<"JSON node missing required field: \"filename\".";
      throw ossimException(xmsg.str());
   }

   // Entry index defaults to 0 if not present:
   if (json_node["entryIndex"].isUInt())
      m_entryIndex = json_node["entryIndex"].asUInt();

   // Band defaults to 1 if not present:
   if (json_node["band"].isUInt())
      m_activeBand = json_node["band"].asUInt();

   // Sensor model defaults to most accurate available if not provided (indicated by blank name):
   if (json_node.isMember("sensorModel"))
      m_modelName = json_node["sensorModel"].asString();

   // Sensor model defaults to most accurate available if not provided (indicated by blank name):
   if (json_node.isMember("imageId"))
      m_imageId = json_node["imageId"].asString();

   // Establish the sensor model. This also sets the official image ID, which will be overwritten
   // if JSON field provided
   string modelState = json_node["modelState"].asString();
   ossimKeywordlist kwl;
   kwl.parseString(modelState);
   ossimProjection* proj =  ossimProjectionFactoryRegistry::instance()->createProjection(kwl);
   m_sensorModel = dynamic_cast<ossimSensorModel*>(proj);
}

void Image::saveJSON(Json::Value& json_node) const
{
   json_node.clear();
   json_node["imageId"] = m_imageId;
   json_node["filename"] = m_filename.string();
   json_node["entryIndex"] = m_entryIndex;

   if (m_modelName.size())
      json_node["sensorModel"] = m_modelName;

   if (m_sensorModel.valid())
   {
      ossimKeywordlist kwl;
      m_sensorModel->saveState(kwl);
      json_node["modelState"] = kwl.toString().string();
   }
}


} // end namespace 
