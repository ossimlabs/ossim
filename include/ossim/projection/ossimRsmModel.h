//---
// File: ossimRsmModel.h
//---
#ifndef ossimRsmModel_H
#define ossimRsmModel_H 1

#include <ossim/projection/ossimSensorModel.h>
#include <ossim/support_data/ossimRsmida.h>
#include <ossim/support_data/ossimRsmpca.h>
#include <ossim/support_data/ossimRsmpia.h>
#include <iosfwd>
#include <vector>

/**
 * @class ossimRsmModel
 */
class OSSIM_DLL ossimRsmModel : public ossimSensorModel
{
public:

   /** @brief default constructor */
   ossimRsmModel();

   /** @brief copy constructor */
   ossimRsmModel( const ossimRsmModel& obj );

   /** @brief assignment operator */
   const ossimRsmModel& operator=( const ossimRsmModel& rhs );

   /**
    * @brief worldToLineSample()
    * Overrides base class implementation. Directly computes line-sample from
    * the polynomials.
    */
   virtual void  worldToLineSample(const ossimGpt& world_point,
                                   ossimDpt&       image_point) const;
 

   /**
    * @brief lineSampleToWorld()
    * Overrides base class pure virtual.  Intersects DEM.
    */
   virtual void  lineSampleToWorld(const ossimDpt& image_point,
                                   ossimGpt&       world_point) const;
   /**
    * @brief lineSampleHeightToWorld()
    * Overrides base class pure virtual. Height understood to be relative to
    * standard ellipsoid.
    */
   virtual void lineSampleHeightToWorld(const ossimDpt& image_point,
                                        const double&   heightEllipsoid,
                                        ossimGpt&       worldPoint) const;
   
   /**
    * @brief imagingRay()
    * Overrides base class pure virtual.
    */
   virtual void imagingRay(const ossimDpt& image_point,
                           ossimEcefRay&   image_ray) const;

   virtual void updateModel();
   virtual void initAdjustableParameters();

   /**
    * @brief dup()
    * Returns pointer to a new instance, copy of this.
    */
   virtual ossimObject* dup() const;

   inline virtual bool useForward()const {return false;}


   /**
    * @brief print()
    * Extends base-class implementation. Dumps contents of object to std::ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @brief saveState
    * Fulfills ossimObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0) const;
   
   /**
    * @brief loadState
    * Fulfills ossimObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   
protected:

   /**
    * @brief Gets index into RSM Polynomial Coefficients(rsmpca) container
    * array for a given ground point.
    *
    * @param x Longitude in radians.
    * @param y Latitude, in radians.
    * @param z Height in meters.
    * @return Index into m_pca vector.
    */
   ossim_uint32 getPcaIndex( const double& x, const double& y, const double& z ) const;

   /**
    * @brief Gets index into RSM Polynomial Coefficients(rsmpca) container
    * array for a given image point.
    *
    * @note (0,0) in ossim space is (0.5, 0.5) in rsm space.
    *
    * @param ipt Image line, sample.
    * 
    * @param shiftPoint If true ipt is shifted +0.5 to go from ossim
    * space(0 is center of pixel) to RSM space(0 is upper left of pixel).
    * If false point is coming from rsm space and does not need to be shifted.
    * 
    * @return Index into m_pca vector.
    */    
   ossim_uint32 getPcaIndex( const ossimDpt& ipt, bool shiftPoint ) const;

   /**
    * @brief Gets index into RSM Polynomial Coefficients(rsmpca) container
    * array for a given ground point.
    *
    * @param x Longitude in radians.
    * @param y Latitude, in radians.
    * @param z Height in meters.
    * @parma ipt Initialized by this.
    */    
   void lowOrderPolynomial( const double& x, const double& y, const double& z,
                            ossimDpt& ipt ) const;
   
   double polynomial( const double& x, const double& y, const double& z,
                      const ossim_uint32& maxx, const ossim_uint32& maxy,
                      const ossim_uint32& maxz,
                      std::vector<double> pcf ) const;
   
   double dPoly_dLat( const double& x, const double& y, const double& z,
                      const ossim_uint32& maxx, const ossim_uint32& maxy,
                      const ossim_uint32& maxz,
                      std::vector<double> pcf) const;
   
   double dPoly_dLon( const double& x, const double& y, const double& z,
                      const ossim_uint32& maxx, const ossim_uint32& maxy,
                      const ossim_uint32& maxz,
                      std::vector<double> pcf ) const;
   
   double dPoly_dHgt( const double& x, const double& y, const double& z,
                      const ossim_uint32& maxx, const ossim_uint32& maxy,
                      const ossim_uint32& maxz,
                      std::vector<double> pcf ) const;

   /**
    * @brief Performs sanity check on key/required rsm data.
    * @return true on success, false on error.
    */
   bool validate() const;
   
   /** @brief virtual destructor */
   virtual ~ossimRsmModel();

   ossimRsmida m_ida;
   ossimRsmpia m_pia;
   std::vector<ossimRsmpca> m_pca;

   TYPE_DATA
      
}; // End: class ossimRsmModel

#endif /* #ifndef ossimRsmModel_H */
