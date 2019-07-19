//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/reg/GroundControlPoint.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimGpt.h>

using namespace std;

namespace ossim
{

GroundControlPoint::GroundControlPoint()
{

}

GroundControlPoint::GroundControlPoint(const Json::Value& json_node)
:  m_covariance (3)
{
   m_covariance.ReSize(3);
   loadJSON(json_node);
}


GroundControlPoint::~GroundControlPoint()
{

}

void GroundControlPoint::loadJSON(const Json::Value& json_node)
{
   ostringstream xmsg;
   xmsg<<__FILE__<<": ";

   m_id = json_node["gcpId"].asString();
   if (m_id.empty())
      m_id = json_node["pointId"].asString();

   if (json_node.isMember("lat") && json_node.isMember("lon") && json_node.isMember("hgt"))
   {
      ossimGpt refGpt;
      refGpt.lat = json_node["lat"].asDouble();
      refGpt.lon = json_node["lon"].asDouble();
      refGpt.hgt = json_node["hgt"].asDouble();
      m_gcp = ossimEcefPoint(refGpt);
   }
   else if (json_node.isMember("x") && json_node.isMember("y") && json_node.isMember("z"))
   {
      m_gcp.x() = json_node["x"].asDouble();
      m_gcp.y() = json_node["y"].asDouble();
      m_gcp.z() = json_node["z"].asDouble();
   }
   else
   {
      xmsg<<"Ground point coordinates JSON not correct. JSON: \n"<<json_node.toStyledString()<<endl;
      throw ossimException(xmsg.str());
   }

   const Json::Value& covariance = json_node["covariance"];
   unsigned int covSize = covariance.size();
   if ((covSize != 6) || (covSize != 9))
   {
      xmsg<<"No covariance information in JSON or not correctly formatted (must be 6 or 9 element array)";
      throw ossimException(xmsg.str());
   }

   // TODO: Covariance in proper coordinate system. ENU -> ECF conversion needed here?
   if (covSize == 6)
   {
      m_covariance(0,0) = covariance[0].asDouble();
      m_covariance(1,1) = covariance[1].asDouble();
      m_covariance(2,2) = covariance[2].asDouble();
      m_covariance(0,1) = covariance[3].asDouble();
      m_covariance(0,2) = covariance[4].asDouble();
      m_covariance(1,2) = covariance[5].asDouble();
   }
   else
   {
      m_covariance(0,0) = covariance[0].asDouble();
      m_covariance(0,1) = covariance[1].asDouble();
      m_covariance(0,2) = covariance[2].asDouble();
      m_covariance(1,0) = covariance[3].asDouble();
      m_covariance(1,1) = covariance[4].asDouble();
      m_covariance(1,2) = covariance[5].asDouble();
      m_covariance(2,0) = covariance[6].asDouble();
      m_covariance(2,1) = covariance[7].asDouble();
      m_covariance(2,2) = covariance[8].asDouble();
   }

   // TODO: Implement GCP cross-correlation
   if (json_node.isMember("crossCovariances") || json_node.isMember("gpCrossCovList"))
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"GCP cross covariances are specified in the JSON, but the"
         " capability is not yet implemented!"<<endl;
   }
}

void GroundControlPoint::saveJSON(Json::Value& json_node) const
{
   // ID
   json_node["gcpId"] = m_id;

   // ECF
   json_node["X"] = m_gcp.x();
   json_node["Y"] = m_gcp.y();
   json_node["Z"] = m_gcp.z();

   Json::Value covJson (Json::arrayValue);
   covJson[0] = m_covariance(1,1);
   covJson[1] = m_covariance(2,2);
   covJson[2] = m_covariance(3,3);
   covJson[3] = m_covariance(1,2);
   covJson[4] = m_covariance(1,3);
   covJson[5] = m_covariance(2,3);

   json_node["covariance"] = covJson;
}
} // end namespace ossimMsp
