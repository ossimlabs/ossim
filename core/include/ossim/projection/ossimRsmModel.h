//---
// File: ossimRsmModel.h
//---
#ifndef ossimRsmModel_H
#define ossimRsmModel_H 1

#include <ossim/projection/ossimSensorModel.h>
#include <iosfwd>

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

   double polynomial( const double& x, const double& y, const double& z,
                      const ossim_uint32& maxx, const ossim_uint32& maxy,
                      const ossim_uint32& maxz,
                      std::vector<ossim_float64> pcf ) const;
   
   double dPoly_dLat( const double& x, const double& y, const double& z,
                      const ossim_uint32& maxx, const ossim_uint32& maxy,
                      const ossim_uint32& maxz,
                      std::vector<ossim_float64> pcf) const;
   
   double dPoly_dLon( const double& x, const double& y, const double& z,
                      const ossim_uint32& maxx, const ossim_uint32& maxy,
                      const ossim_uint32& maxz,
                      std::vector<ossim_float64> pcf ) const;
   
   double dPoly_dHgt( const double& x, const double& y, const double& z,
                      const ossim_uint32& maxx, const ossim_uint32& maxy,
                      const ossim_uint32& maxz,
                      std::vector<ossim_float64> pcf ) const;

   /**
    * @brief print()
    * Extends base-class implementation. Dumps contents of object to ostream.
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
   
   /** @brief virtual destructor */
   virtual ~ossimRsmModel();
   
   ossimString m_iid;
   ossimString m_edition;

   ossim_uint32 m_rsn;
   ossim_uint32 m_csn;

   ossim_float64 m_rfep;
   ossim_float64 m_cfep;
   ossim_float64 m_rnrmo;
   ossim_float64 m_cnrmo;
   ossim_float64 m_xnrmo;
   ossim_float64 m_ynrmo;
   ossim_float64 m_znrmo;
   ossim_float64 m_rnrmsf;
   ossim_float64 m_cnrmsf;
   ossim_float64 m_xnrmsf;
   ossim_float64 m_ynrmsf;
   ossim_float64 m_znrmsf;
   
   ossim_uint32 m_rnpwrx;
   ossim_uint32 m_rnpwry;
   ossim_uint32 m_rnpwrz;
   ossim_uint32 m_rntrms;

   std::vector<ossim_float64> m_rnpcf;

   ossim_uint32 m_rdpwrx;
   ossim_uint32 m_rdpwry;
   ossim_uint32 m_rdpwrz;
   ossim_uint32 m_rdtrms;

   std::vector<ossim_float64> m_rdpcf;

   ossim_uint32 m_cnpwrx;
   ossim_uint32 m_cnpwry;
   ossim_uint32 m_cnpwrz;
   ossim_uint32 m_cntrms;

   std::vector<ossim_float64> m_cnpcf;

   ossim_uint32 m_cdpwrx;
   ossim_uint32 m_cdpwry;
   ossim_uint32 m_cdpwrz;
   ossim_uint32 m_cdtrms;

   std::vector<ossim_float64> m_cdpcf;

   TYPE_DATA
      
}; // End: class ossimRsmModel

#endif /* #ifndef ossimRsmModel_H */
