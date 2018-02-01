//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef TiePoint_HEADER
#define TiePoint_HEADER 1

#include <ossim/base/JsonInterface.h>
#include <ossim/reg/Image.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/matrix/newmat.h>
#include <vector>
#include <memory>

namespace ossim
{
class TiePoint;
typedef std::vector< std::shared_ptr<TiePoint> > TiePointList;

/**
 * Class for representing a single tiepoint on two or more images. It also represents image-points
 * associated with a ground control point.
 */
class TiePoint : public ossim::JsonInterface,
                 public std::enable_shared_from_this<TiePoint>
{
public:
   enum Type {
      UNASSIGNED,
      MANUAL,  //> The point was created or edited manually and contains at least a pair of image points
      AUTO,    //> Result of auto tie point generation and contains at least a pair of image points
      GCP      //> point is associated with a manually-entered ground control. Possibly multiple image points
   };

   TiePoint();

   TiePoint(const TiePoint& copy);

   /**
   * Creates new tiepoint from JSON object formatted as:
   * {
   *    "id": <unsigned int>, // may be ignored if not valid
   *    "type": "M"|"A"|"G"  For “manual”, “auto”, or “GCP-associated”
   *    "imagePoints: [
   *        {
   *           "imageId": <string>,
   *           "x": <double>,
   *           "y": <double>,
   *           "covariance": [ cxx, cyy, cxy ],
   *           "gcpId": <string> Only if associated with a GCP
   *        }
   *    ]
   * }
   */
   TiePoint(const Json::Value& tp_json_node);

   virtual ~TiePoint();

   unsigned int getImageCount() const { return m_images.size(); }

   const std::string& getTiePointId() const { return m_tiePointId; }

   void setTiePointId(const std::string& id);

   /**
    * Fetches the image point coordinates along with image ID and GCP ID if available.
    * @param index Inout index into the list of images containing this tiepoint
    * @param imageId Output image ID for the ccorresponding image point
    * @param imagePoint image coordinates
    * @param cov image point measurement covariance in (x, y) coordinates
    */
   void getImagePoint(unsigned int index,
                      std::string& imageId,
                      ossimDpt& imagePoint,
                      NEWMAT::SymmetricMatrix& cov) const;

   /**
    * Sets image point value and associated measurement covariance for specified image ID.
    * If the image ID is not found, it will be added as new observation.
    */
   void setImagePoint(std::shared_ptr<Image> image,
                      const ossimDpt& imagePoint,
                      const NEWMAT::SymmetricMatrix& cov);

   Type getType() const { return m_type; }
   void setType(Type t) { m_type = t; }

   const std::string& getGcpId() const { return m_gcpId; }
   void setGcpId(const std::string& id);

   /*
   * Refer to <a href="https://docs.google.com/document/d/1DXekmYm7wyo-uveM7mEu80Q7hQv40fYbtwZq-g0uKBs/edit?usp=sharing">3DISA API document</a>
   * for JSON format used.
   */
   virtual void loadJSON(const Json::Value& json);

   /*
   * Refer to <a href="https://docs.google.com/document/d/1DXekmYm7wyo-uveM7mEu80Q7hQv40fYbtwZq-g0uKBs/edit?usp=sharing">3DISA API document</a>
   * for JSON format used.
   */
   virtual void saveJSON(Json::Value& json) const;

   virtual std::ostream& print(std::ostream& out) const;
   friend std::ostream& operator << (std::ostream& out, const TiePoint& tp)
   { return tp.print(out); }

protected:

   Type m_type;
   std::string  m_tiePointId;
   std::vector< std::shared_ptr<Image> > m_images;    //> List of images containing common feature
   std::vector<ossimDpt> m_imagePoints; //> List of image point measurements for common feature
   std::vector<NEWMAT::SymmetricMatrix> m_covariances; //> List of measurement covariances corresponding to image points vector
   double m_gsd; //> image scale (meters/pixel) at which matching was performed
   std::string m_gcpId; //> Cross reference to GCP record associated with this TP.
   static int s_runningId;
};

typedef std::vector< std::shared_ptr<TiePoint> > TiePointList;

} // end namespace ATP
#endif /* #ifndef TiePoint_HEADER */
