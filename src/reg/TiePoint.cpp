//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/reg/TiePoint.h>
#include <ossim/base/ossimException.h>
#include <ctime>

using namespace std;

namespace ossim
{
int TiePoint::s_runningId = 1;

TiePoint::TiePoint()
:  m_type (UNASSIGNED),
   m_gsd (0.0)
{

}

TiePoint::TiePoint(const TiePoint& copy)
:  m_type (copy.m_type),
   m_gsd (copy.m_gsd),
   m_gcpId (copy.m_gcpId)
{
   for (size_t i=0; i<copy.m_images.size(); ++i)
      m_images.push_back(copy.m_images[i]);

   for (size_t i=0; i<copy.m_imagePoints.size(); ++i)
      m_imagePoints.push_back(copy.m_imagePoints[i]);

   for (size_t i=0; i<copy.m_covariances.size(); ++i)
      m_covariances.push_back(copy.m_covariances[i]);
}

TiePoint::TiePoint(const Json::Value& json_node)
:  m_type (UNASSIGNED),
   m_gsd (0.0)
{
   loadJSON(json_node);
}

TiePoint::~TiePoint()
{
   m_images.clear();
   m_imagePoints.clear();
   m_covariances.clear();
}

void TiePoint::setTiePointId(const string& id)
{
   if (id.empty())
   {
      // Generate ID based on date/time:
      ostringstream tp1;
      tp1<<"TP"<<s_runningId++;
      m_tiePointId = tp1.str();
   }
   else
   {
      // Only accept if there is no ID existing:
      m_tiePointId = id;
   }
}

void TiePoint::getImagePoint(unsigned int index,
                             std::string& imageId,
                             ossimDpt& imagePoint,
                             NEWMAT::SymmetricMatrix& cov) const
{
   // Search the PB index list for entry:
   imagePoint.makeNan();
   imageId.clear();

   if (index >= m_images.size())
      return;

   imageId = m_images[index]->getImageId();
   imagePoint = m_imagePoints[index];
   cov = m_covariances[index];
}

void TiePoint::setImagePoint(std::shared_ptr<Image> image,
                             const ossimDpt& imagePoint,
                             const NEWMAT::SymmetricMatrix& cov)
{
   bool found = false;

   // Possible edit of existing point?
   for (size_t i=0; i<m_images.size(); ++i)
   {
      if (m_images[i]->getImageId().compare(image->getImageId()) == 0)
      {
         found = true;
         if (i >= m_imagePoints.size())
         {
            // This shouldn't happen, but go ahead and resize for this image:
            m_imagePoints.resize(i+1);
            m_covariances.resize(i+1);
         }
         m_imagePoints[i] = imagePoint;
         m_covariances[i] = cov;
         break;
      }
   }

   if (!found)
   {
      m_images.push_back(image);
      m_imagePoints.push_back(imagePoint);
      m_covariances.push_back(cov);
   }
}

void TiePoint::setGcpId(const std::string& id)
{
   m_gcpId = id;
   m_type = GCP;
}

void TiePoint::loadJSON(const Json::Value& json_node)
{
   ostringstream xmsg;
   xmsg<<__FILE__<<": TiePoint(JSON) --";

   // ID
   setTiePointId(json_node["id"].asString());

   // Type
   string tpType = json_node["type"].asString();
   switch (tpType[0])
   {
   case 'M':
      m_type = TiePoint::MANUAL;
      break;
   case 'A':
      m_type = TiePoint::AUTO;
      break;
   case 'G':
      m_type = TiePoint::GCP;
      break;
   default:
      xmsg<<"Tiepoint JSON field \"type\" must be specified as 'M', 'A', or 'G'.";
      throw ossimException(xmsg.str());
   }

   // Read the GCP ID if present:
   if (m_type == TiePoint::GCP)
      m_gcpId = json_node["gcpId"].asString();

   // Image points
   const Json::Value& imagePoints = json_node["imagePoints"];
   if (!imagePoints || (imagePoints.size() < 1))
   {
      xmsg<<"Tiepoint JSON field \"imagePoints\" not found or is empty!";
      throw ossimException(xmsg.str());
   }

   // Loop over points on each image:
   for (int i=0; i<imagePoints.size(); ++i)
   {
      const Json::Value& p = imagePoints[i];
      if (!p || !(p["imageId"].isString()) || !(p["x"]) || !(p["y"]) || (p["covariance"].size()!=3))
      {
         xmsg<<"Tiepoint JSON field \"imagePoints\" entry is ill-formed or not complete:\n"
               <<p.toStyledString()<<endl;
         throw ossimException(xmsg.str());
      }

      string imageId = p["imageId"].asString();
      string filename = p["filename"].asString();
      shared_ptr<Image> image (new Image(imageId, filename));
      m_images.push_back(image);

      ossimDpt xy(p["x"].asDouble(), p["y"].asDouble());
      m_imagePoints.push_back(xy);

      const Json::Value& covariance = p["covariance"];
      NEWMAT::SymmetricMatrix c (2);
      c(1,1) = covariance[0].asDouble();
      c(2,2) = covariance[1].asDouble();
      c(1,2) = covariance[2].asDouble();
      m_covariances.push_back(c);
   }
}

void TiePoint::saveJSON(Json::Value& json_node) const
{
   // ID
   json_node["id"] = m_tiePointId;

   // Type
   string tpType = json_node["type"].asString();
   switch (m_type)
   {
   case TiePoint::MANUAL:
      json_node["type"] = "M";
      break;
   case TiePoint::AUTO:
      json_node["type"] = "A";
      break;
   case TiePoint::GCP:
      json_node["type"] = "G";
      break;
   default:
      json_node["type"] = "UNNASSIGNED";
   }

   // Image points
   Json::Value jsonList (Json::arrayValue);
   // Loop over points on each image:
   for (int i=0; i<m_imagePoints.size(); ++i)
   {
      jsonList[i]["filename"] = m_images[i]->getFilename();
      jsonList[i]["imageId"] = m_images[i]->getImageId();
      jsonList[i]["x"] = m_imagePoints[i].x;
      jsonList[i]["y"] = m_imagePoints[i].y;

      if (i<m_covariances.size())
      {
         Json::Value covJson (Json::arrayValue);
         covJson[0] = m_covariances[i](1,1);
         covJson[1] = m_covariances[i](2,2);
         covJson[2] = m_covariances[i](1,2);
         jsonList[i]["covariance"] = covJson;
      }
   }
   json_node["imagePoints"] = jsonList;
}

std::ostream& TiePoint::print(std::ostream& out) const
{
   Json::Value node;
   saveJSON(node);
   out << node.toStyledString();
   return out;
}

} // end namespace ISA
