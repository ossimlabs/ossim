//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef GroundControlPoint_HEADER
#define GroundControlPoint_HEADER 1

#include <ossim/base/JsonInterface.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/matrix/newmat.h>
#include <memory>

namespace ossim
{
/**
 * Class for representing a ground control point.
 * TODO: mplement cross-correlation between GCPs
 */
class GroundControlPoint : public ossim::JsonInterface,
                           public std::enable_shared_from_this<GroundControlPoint>
{
public:

   /**
    * Creates new GCP from JSON object.
    * The ground point (GCP) coordinates are specified in either ECF or geographic.
    * The associated covariance must be in the same coordinate system. If correlations exist to
    * other GCPs, those GCPs must share the same coordinate system.  TBD: Perhaps the GCP
    * information, including coordinates, covariances and cross-covariances can be ingested prior
    * and accessed from the database with only GCP ID. Id so, the remainder of this message is not
    * needed.
    *
    * The JSON Format:
    * {
    *    "id": <string>,
    *    "ecf": [ <X>, <Y>, <Z> ], OR
    *    "geo": [ <lat>, <lon>, <hgt_msl> ],
    *    "covariance": [ c11, c22, c33, c12, c13, c23 ],
    *    "crossCovariances": [ (Can be excluded if no correlation information available)
    *       {
    *           "id": <string>, The other GCP’s ID
    *           "crossCovariance": [ c11, c22, c33, c12, c13, c23 ]
    *       }
    *    ]
    * }
    *
    */
   GroundControlPoint(const Json::Value& image_json_node);

   virtual ~GroundControlPoint();

   const std::string& getId() const { return m_id; }
   const ossimEcefPoint& getECF() const { return m_gcp; }

   const NEWMAT::SymmetricMatrix& getCovariance() const {return m_covariance;}

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

private:
   GroundControlPoint();

   std::string m_id;
   ossimEcefPoint m_gcp;
   NEWMAT::SymmetricMatrix m_covariance; //> X, Y, Z (ECF)
};

typedef std::vector< std::shared_ptr<GroundControlPoint> > GcpList;

} // end namespace ossimMsp
#endif /* #ifndef GroundControlPoint_HEADER */
